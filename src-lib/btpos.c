/*
 * $Id$
 *
 * btpos: Positions index to beginning or end of whole file or
 *        duplicate key section.
 *
 * Parameters:
 *    b      index file context pointer
 *    pos    ZSTART or ZEND
 *    indups if TRUE, position based on current key within duplicate
 *           set.  If dups are not permitted, acts as if set to FALSE.
 *    
 * When btpos returns (with no error), with an argument of ZSTART,
 * bnxtky will return the first key in the index (or the first key in
 * the duplicate section).  With ZEND, bprvky will return the last key in
 * the index (or the last key of the duplicate section).
 *  
 * Returns zero if no errors, error code otherwise
 *
 * Copyright (C) 2010, Mark Willson.
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
#include "btree.h"
#include "btree_int.h"

int btpos(BTA *b,int pos,int in_dups)
{
    int status;
    BTint val;
    char dkey[ZKYLEN],tkey[ZKYLEN];
    
    
    bterr("",0,NULL);
    if ((status=bvalap("BTPOS",b)) != 0) return(status);

    btact = b;      /* set context pointer */

    if (btact->shared) {
        if (!block()) {
            bterr("BTPOS",QBUSY,NULL);
            goto fin;
        }
    }
    if (!in_dups || bgtinf(btact->cntxt->super.scroot,ZMISC) == 0) {
        if (pos == ZSTART) {
            btact->cntxt->lf.lfexct = FALSE;
            bstkin();
            bpush(ZNULL);
            bpush(ZNULL);
            btact->cntxt->lf.lfblk = btact->cntxt->super.scroot;
            btact->cntxt->lf.lfpos = 0;
            bleaf(0);
        }
        else if (pos == ZEND) {
            btact->cntxt->lf.lfexct = FALSE;
            bstkin();
            bpush(ZNULL);
            bpush(ZNULL);
            btact->cntxt->lf.lfblk = btact->cntxt->super.scroot;
            btact->cntxt->lf.lfpos = bgtinf(btact->cntxt->super.scroot,ZNKEYS);
            bleaf(1);
        }
    }
    else if (pos == ZSTART) {
        strcpy(tkey,btact->cntxt->lf.lfkey);
        strcpy(dkey,tkey);    
        while (strcmp(dkey,tkey) == 0) {
            status = bprvky(b,dkey,&val);
            if (status == QNOKEY) {
                /* run out of keys; reposition at first
                   key */
                btpos(b,ZSTART,FALSE);
                goto fin;
            }
        }
    }
    else if (pos == ZEND) {
        strcpy(dkey,btact->cntxt->lf.lfkey);
        strcpy(dkey,tkey);  
        while (strcmp(tkey,dkey) == 0) {
            status = bnxtky(b,dkey,&val);
            if (status == QNOKEY) {
                /* run out of keys; reposition at last
                   key */
                btpos(b,ZEND,FALSE);
                goto fin;
            }
        }
    }
    if (btact->shared) bulock();
    
  fin:
    return btgerr();
}

