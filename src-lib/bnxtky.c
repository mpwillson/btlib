/*
 * $Id: bnxtky.c,v 1.13 2010-12-31 14:20:52 mark Exp $
 *
 * bnxtky:  returns next key from index
 *
 * Parameters:
 *   b       index file context pointer
 *   key     returned with next key
 *   val     returned with value of key
 *
 * bnxtky returns non-ZERO if an error occurred
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
#include "btree.h"

int bnxtky(BTA* b,char *key,BTint *val)
{
    int idx,nkeys,status;
    int found;

    bterr("",0,NULL);
    if ((status=bvalap("BNXTKY",b)) != 0) return(status);

    btact = b;          /* set global context pointer */

    if (btact->shared) {
        if (bgtinf(btact->cntxt->super.scroot,ZMISC)) {
            /* root supports duplicate keys; must be locked */
            if (btact->lckcnt == 0) {
                bterr("BNXTKY",QNOTOP,NULL);
                goto fin;
            }
            block(); /* balance bulock at routine exit */
        }
        else {
            /* can re-position when no dups */
            if (!block()) {
                bterr("BNXTKY",QBUSY,NULL);
                goto fin;
            }
            /* position to last found key via bfndky, since context could
             * have been invalidated by concurrent updates by other users.
             * Note we don't care if the key is found or not, so the error
             * status is always cleared. */
            status = bfndky(btact,btact->cntxt->lf.lfkey,val);
            bterr("",0,NULL);
        }
    }
    

    found = FALSE;
    while (btact->cntxt->lf.lfblk != ZNULL && !found) {
        status = brdblk(btact->cntxt->lf.lfblk,&idx);
        if (idx < 0) {
            bterr("BNXTKY",QRDBLK,itostr(btact->cntxt->lf.lfblk));
            break;
        }
        nkeys = bgtinf(btact->cntxt->lf.lfblk,ZNKEYS);
#if DEBUG >= 1
        printf("BNXTKY: lfblk: " ZINTFMT ", lfpos: %d, nkeys: %d\n",
               btact->cntxt->lf.lfblk,btact->cntxt->lf.lfpos,nkeys);
#endif
        if (btact->cntxt->lf.lfpos >= nkeys || nkeys == 0) {
            /* finished with this block (or no key was found at all),
             * get parent from stack */
            btact->cntxt->lf.lfpos = bpull();
            btact->cntxt->lf.lfblk = bpull();
            btact->cntxt->lf.lfexct = FALSE; 
            continue;
        }

        if (!btact->cntxt->lf.lfexct) {
            btact->cntxt->lf.lfexct = TRUE;
        }
        else {
            btact->cntxt->lf.lfpos++;
            idx = bleaf(0);
            if (idx < 0) continue;
        }
        
        if (btact->cntxt->lf.lfpos < nkeys) {
            found = TRUE;
            strcpy(key,
                   ((btact->memrec)+idx)->keyblk[btact->cntxt->lf.lfpos].key);
            /* remember found key (need for shared mode) */
            strcpy(btact->cntxt->lf.lfkey,key);
            *val = ((btact->memrec)+idx)->keyblk[btact->cntxt->lf.lfpos].val;
        }
    }
    if (btact->cntxt->lf.lfblk == ZNULL) {
        /* end of index reached */
        bterr("BNXTKY",QNOKEY,NULL);
    }
fin:
    if (btact->shared) bulock();
    return(btgerr());
}
