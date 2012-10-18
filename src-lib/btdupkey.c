/*
 * $Id: btdupkey.c,v 1.12 2012/10/14 19:31:24 mark Exp $
 *
 *
 * btdupkey:  inserts duplicate key into index
 *
 * Parameters:
 *    key    key to insert
 *    val    value of key 
 *
 * btdupkey returns non-ZERO if error occurred
 *
 * Copyright (C) 2012 Mark Willson.
 *
 * This file is part of the B Tree library.
 *
 * The B Tree library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The B Tree library  is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the B Tree library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "btree.h"
#include "btree_int.h"
#include <string.h>

KEYENT* getkeyent(BTint blk, int pos)
{
    int idx,ioerr;
    
    ioerr = brdblk(blk,&idx);
    if (ioerr == 0) {
        return &(((btact->memrec)+idx)->keyblk[pos]);
    }
    else {
        bterr("BTDUPPOS",-1,NULL); /*TDB set error code */
        return NULL;
    }
}

DKEY* getdkey(BTint draddr)
{
    static DKEY dkey;
    BTint blk;
    int offset,sz;

    cnvdraddr(draddr,&blk,&offset);
    if (bgtinf(blk,ZBTYPE) != ZDUP) {
        bterr("BTDUPPOS",0,itostr(draddr)); /*TBD: define error code */
    }
    else {
        sz = bseldt(draddr,(char *) &dkey,sizeof(DKEY));
        if (sz == sizeof(struct bt_dkey)) {
            return &dkey;
        }
        else {
            bterr("BTDUPPOS",-1,NULL); /*TDB set error code */
        }
    }
    return NULL;
}

int putdkey(BTint draddr, DKEY* dkey)
{
    bupddt(draddr,(char *) dkey,sizeof(struct bt_dkey));
    return btgerr();
}

/* Add duplicate key to index */

int btdupkey(char *key, BTint val)
{
    int n =  btact->cntxt->lf.lfpos;
    BTint cblk = btact->cntxt->lf.lfblk;
    KEYENT* keyent;
    struct bt_dkey dkey;
    BTint draddr;
    int sz;

    keyent = getkeyent(cblk,n);
    if (keyent == NULL) {
        bterr("BTDUPKEY",QRDBLK,itostr(cblk));
        return btgerr();
    }
    
    /* set dup entry invariants */
    dkey.deleted = FALSE;
    dkey.flink = ZNULL;
    
    /* if first duplicate, need to handle original key */
    if (keyent->dup == ZNULL) {
        fprintf(stderr,"creating new duplicate block.\n");
        /* construct duplicate key entry for original key */
        strcpy(dkey.key,keyent->key);
        dkey.val = keyent->val;
        dkey.blink = ZNULL;
        bsetbs(cblk,TRUE);
        draddr = binsdt(ZDUP,(char *) &dkey,sizeof(struct bt_dkey));
        bsetbs(cblk,FALSE);
        if (draddr == ZNULL) {
            return btgerr();
        }
        keyent->val = draddr;
        keyent->dup = draddr;
    }
    else {
        fprintf(stderr,"adding to existing duplicate block.\n");
    }
    /* add new duplicate key */
    strcpy(dkey.key,key);
    dkey.val = val;
    dkey.blink = keyent->dup;
    draddr = binsdt(ZDUP,(char *) &dkey,sizeof(struct bt_dkey));
    if (draddr == ZNULL) {
        return btgerr();
    }
    /* set flink of previous dup entry */
    sz = bseldt(keyent->dup,(char *) &dkey,sizeof(struct bt_dkey));
    if (sz != sizeof(struct bt_dkey)) {
        bterr("BTDUPKEY",-1,NULL); /*TDB set error code */
        return btgerr();
    }
    dkey.flink = draddr;
    if (putdkey(keyent->dup,&dkey) == 0) {
        /* set new dup list tail pointer */
        keyent->dup = draddr;
    }
    return btgerr();;
}

/* return val from next/prev undeleted dup key
   return value: 0 = value returned; >0 error code; ZNULL no more dup
   keys
*/
int btduppos(int direction, BTint *val)
{
    BTint newaddr;
    DKEY* dkey;
    KEYENT* keyent;

    if (direction != ZNEXT && direction != ZPREV) {
        bterr("BTDUPPOS",-1,NULL); /*TDB set error code */
        return btgerr();
    }

    /* if not exact position, and not in dup chain, nothing to do */
    if (btact->cntxt->lf.draddr == ZNULL && !btact->cntxt->lf.lfexct) {
        return ZNULL;
    }
    
    if (btact->cntxt->lf.draddr == ZNULL) {
        /* not in dup key chain, but are we at the start or end of
           one? */
        if ((keyent = getkeyent(btact->cntxt->lf.lfblk,btact->cntxt->lf.lfpos))
            == NULL) return ZNULL;
#if DEBUG >= 1
        fprintf(stderr,"BTDUPPOS: keyent->key: %s, val: " ZINTFMT
                ", dup: " ZINTFMT "\n",keyent->key,keyent->val,keyent->dup);
#endif  
        if (keyent->dup == ZNULL) return ZNULL;
        newaddr = (direction==ZNEXT?keyent->val:keyent->dup);
        dkey = getdkey(newaddr);
        if (dkey == NULL) {
            return btgerr();
        }
    }   
    else {
        /* find next/prev non-deleted duplicate key */
        dkey = getdkey(btact->cntxt->lf.draddr);
        if (dkey == NULL) {
            return btgerr();
        }
        do {
            newaddr = (direction==ZNEXT?dkey->flink:dkey->blink);
            if (newaddr == ZNULL) {
                /* at beginning/end of chain; must move to next key */
                btact->cntxt->lf.lfexct = TRUE;
                return ZNULL;
            }
            dkey = getdkey(newaddr);
            if (dkey == NULL) {
                return btgerr();
            }
#if DEBUG >= 1
            fprintf(stderr,"BTDUPPOS: draddr: " ZINTFMT ", dkey->key: %s, val: "
                    ZINTFMT "del: %d, blink: "
                    ZINTFMT ", flink: " ZINTFMT "\n",
                    newaddr, dkey->key, dkey->val, dkey->deleted,
                    dkey->blink, dkey->flink);
#endif
        } while (dkey->deleted);
    }
        
    btact->cntxt->lf.draddr = newaddr;
    btact->cntxt->lf.lfexct = TRUE;
    *val = dkey->val;
    return 0;
}

int btdeldup ()
{
    DKEY* dkey;
    KEYENT* keyent;
    BTint blk, flink = ZNULL, blink = ZNULL;
    int offset;
    
    /* either bfndky or btduppos will set context dup draddr if we
     * are at a duplicate key */
    
    if (btact->cntxt->lf.draddr != ZNULL) {
        dkey = getdkey(btact->cntxt->lf.draddr);
        if (dkey == NULL) return btgerr();
        keyent = getkeyent(btact->cntxt->lf.lfblk,btact->cntxt->lf.lfpos);
        if (keyent == NULL) return btgerr();
        if (dkey->blink == ZNULL && dkey->flink == ZNULL) {
            /* only duplicate in chain */
            keyent->val = dkey->val;
            keyent->dup = ZNULL;
            btact->cntxt->lf.draddr = ZNULL;
        }
        else if (dkey->blink == ZNULL) {
            /* deleting first in chain */
            keyent->val = dkey->flink;
            flink = dkey->flink;
        }
        else if (dkey->flink == ZNULL) {
            /* deleting last in chain */
            keyent->dup = dkey->blink;
            blink = dkey->blink;
        }
        else {
            /* in middle of list */
            flink = dkey->flink;
            blink = dkey->blink;
        }

        /* update deleted dup key */
        dkey->deleted = TRUE;
        if (putdkey(btact->cntxt->lf.draddr,dkey) != 0) return btgerr();
        cnvdraddr(btact->cntxt->lf.draddr,&blk,&offset);
        /* update used space in dup block; we expect deldat to leave
         * the data record intact */
        deldat(blk,offset);

        /* update previous/next entries as required */
        if (blink != ZNULL) {
            dkey = getdkey(blink);
            if (dkey == NULL) return btgerr();
            dkey->flink = flink;
            if (putdkey(blink,dkey) != 0) return btgerr();
        }
        if (flink != ZNULL) {
            dkey = getdkey(flink);
            if (dkey == NULL) return btgerr();
            dkey->blink = blink;
            if (putdkey(flink,dkey) != 0) return btgerr();
        }
        /* Mark inexact match, as we are pointing at a deleted key */
        btact->cntxt->lf.lfexct = FALSE;
    }
    return btgerr();
}

/* Display duplicate key entries in blk */
int btdispdups(BTint blk)
{
    BTint draddr, mx;
    DKEY* dkey;
    
    if (bgtinf(blk,ZBTYPE) != ZDUP) {
        return btgerr();
    }

    draddr = mkdraddr(blk,0);
    mx = mkdraddr(blk,bgtinf(blk,ZNKEYS));

    fprintf(stdout,"%20s%32s%20s%20s%20s%20s\n","Draddr","Key","Val","Del?",
            "Blink", "Flink");
    while (draddr < mx) {
        dkey = getdkey(draddr);
        if (dkey == NULL) {
            bterr("DISPDUPS",-1,NULL); /* TBD: internal error, most likely */
            return 0;
        }
        
        fprintf(stdout,"" Z20DFMT "%32s" Z20DFMT Z20DFMT  Z20DFMT Z20DFMT "\n",
                draddr, dkey->key, dkey->val, dkey->deleted, dkey->blink,
                dkey->flink);
        draddr += sizeof(DKEY)+ZDOVRH;
    }
    return 0;
}
