/*
 * $Id: bputky.c,v 1.14 2012-06-16 19:39:43 mark Exp $
 *
 * bputky: inserts key, value and links into block
 *
 * Parameters:
 *    blk    block for which insertion is required
 *    key    key to insert
 *    val    value of key to insert
 *    link1  left link pointer
 *    link2  right link pointer
 *
 * Copyright (C) 2003, 2004, 2010 Mark Willson.
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

#include <stdio.h>
#include <string.h>

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

/* #undef DEBUG */
/* #define DEBUG 2 */

int bputky(BTint blk,char *key,BTint val,BTint link1,BTint link2) 
{
    int i,idx,ioerr;
    int nkeys;
    char lkey[ZKYLEN];
    KEYENT keyent;
    
#if DEBUG >= 1
    fprintf(stderr,
            "bputky: blk = " ZINTFMT ", key = %s, val = " 
             ZINTFMT ", link1 = " ZINTFMT ", link2 = " ZINTFMT "\n",
            blk,key,val,link1,link2);
#endif
    ioerr = brdblk(blk,&idx);
    if (idx < 0) {
        bterr("BPUTKY",QRDBLK,itostr(blk));
    }
    else {
        /* get local copy of key, truncated if necessary */
        strncpy(lkey,key,ZKYLEN);
        lkey[ZKYLEN-1] = '\0';
        nkeys = ((btact->memrec)+idx)->infblk[ZNKEYS];
        if (nkeys == ZMXKEY) {
            bterr("BPUTKY",QBLKFL,itostr(blk));
            goto fin;
        }
        if (nkeys == 0) {
            /* block empty */
            strcpy(((btact->memrec)+idx)->keyblk[0].key,lkey);
            ((btact->memrec)+idx)->keyblk[0].val = val;
            ((btact->memrec)+idx)->keyblk[0].dup = ZNULL;
            ((btact->memrec)+idx)->lnkblk[0] = link1;
            ((btact->memrec)+idx)->lnkblk[1] = link2;
        }
        else {
            if (link1 == ZNULL && link2 != ZNULL) {
                /* inserting demoted key at end of block */
                i = nkeys;
            }
            else {
                for (i=((btact->memrec)+idx)->infblk[ZNKEYS];i>0;i--) {
                    /* ensure duplicate key is always inserted at front of
                       set so key promotion works */
                    if (strcmp(key,((btact->memrec)+idx)->keyblk[i-1].key) <= 0) {
                        /* move info to make room */
                        ((btact->memrec)+idx)->keyblk[i] =
                            ((btact->memrec)+idx)->keyblk[i-1];
                        ((btact->memrec)+idx)->lnkblk[i+1] = 
                            ((btact->memrec)+idx)->lnkblk[i];
                    }
                    else break;
                }
            }
#if DEBUG >= 1
            fprintf(stderr,"BPUTKY: inserting new key at pos %d\n",i);
#endif
            /* move left link if inserting in first position */
            if (i == 0) {
                ((btact->memrec)+idx)->lnkblk[1] = 
                    ((btact->memrec)+idx)->lnkblk[0];
            }
            strcpy(((btact->memrec)+idx)->keyblk[i].key,lkey);
            ((btact->memrec)+idx)->keyblk[i].val = val;
            ((btact->memrec)+idx)->keyblk[i].dup = ZNULL;
            if (link1 == ZNULL && link2 == ZNULL) {
                /* inserting a leaf key */
                ((btact->memrec)+idx)->lnkblk[i] = ZNULL;
                ((btact->memrec)+idx)->lnkblk[i+1] = ZNULL;
            }
            else {
                /* if inserting in first pos, then use llink */
                if (i == 0) ((btact->memrec)+idx)->lnkblk[0] = link1;
                /* rlink is inserted if ZERO or positive */
                if (link2 >= 0) ((btact->memrec)+idx)->lnkblk[i+1] = link2;
            }
        }
    }
    ((btact->memrec)+idx)->infblk[ZNKEYS]++;
    ((btact->cntrl)+idx)->writes++;
    if ( btact->wt_threshold > 0 &&
         ((btact->cntrl)+idx)->writes > btact->wt_threshold) {
        bwrblk(blk);
    }
  fin:
    return(0);
}
