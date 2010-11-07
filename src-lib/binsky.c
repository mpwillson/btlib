/*
 * $Id: binsky.c,v 1.7 2010-05-26 12:39:16 mark Exp $
 *
 *
 * binsky:  inserts key into index (duplicates not permitted)
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
    int status,l;
    char lkey[ZKYLEN+1];
    char dup_suffix [] = { '\001', '\000'};

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
        if (dups_allowed) {
            /* make search key next in sorting sequence to ensure
               index is positioned correctly for insertion of dup
               key. */
            lkey[ZKYLEN-1] = '\0';
            strcat(lkey,dup_suffix);
        }
        status = bfndky(b,lkey,&lval);
        if (status == QNOKEY || (status == 0 && dups_allowed)) {
            /* QNOKEY is not an error in this context; remove it */
            bterr("",0,NULL);
            bputky(btact->cntxt->lf.lfblk,key,val,ZNULL,ZNULL);
        }
        else {
            bterr("BINSKY",QDUP,key);
        }
    }
fin:
    if (btact->shared) bulock();
    return(btgerr());
}
