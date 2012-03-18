/*
 * $Id: binsky.c,v 1.9 2010-11-21 15:04:28 mark Exp $
 *
 *
 * binsky:  inserts key into index
 *
 * Parameters:
 *    b      pointer to BT context      
 *    key    key to insert
 *    val    value of key 
 *
 * binsky returns non-ZERO if error occurred
 *
 * Copyright (C) 2003, 2004 Mark Willson.
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

int binsky(BTA *b, char *key,BTint val)
{
    BTint lval,dups_allowed;
    int status;
    char lkey[ZKYLEN+1];

    bterr("",0,NULL);
    if ((status=bvalap("BINSKY",b)) != 0) return(status);

    btact = b;
    if (btact->shared) {
        if (!block()) {
            bterr("BINSKY",QBUSY,NULL);
            goto fin;
        }
    }

    if (b->cntxt->super.smode != 0) {
        /* read only, can't insert */
        bterr("BINSKY",QNOWRT,NULL);
    }
    else {
        dups_allowed = bgtinf(btact->cntxt->super.scroot,ZMISC);
        strncpy(lkey,key,ZKYLEN);
        lkey[ZKYLEN-1] = '\0';
        status = bfndky(b,lkey,&lval);
        if (status == QNOKEY) {
            /* unique key */
            /* QNOKEY is not an error in this context; remove it */
            bterr("",0,NULL);
            bputky(btact->cntxt->lf.lfblk,key,val,ZNULL,ZNULL);
        }
        else if (status == 0 && dups_allowed) {
            /* inserting duplicate key */
            btdupkey(key,val);
        }
        else {
            bterr("BINSKY",QDUP,key);
        }
    }
fin:
    if (btact->shared) bulock();
    return(btgerr());
}
