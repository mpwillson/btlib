/*
 * $Id: brepky.c,v 1.4 2004/09/26 11:49:18 mark Exp $
 *
 * brepky: replaces key at location loc in block
 *
 * Parameters:
 *   blk    block for which replacement is required
 *   loc    location in block to store information
 *   key    name of key
 *   val    value of key
 *   link1  left link pointer
 *   link2  right link pointer
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
 * along with Foobar; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <string.h>

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int brepky(int blk,int loc,char *key,int val,int link1,int link2)
{
    int idx,ioerr;

    if (loc >= ZMXKEY || loc < 0) {
        bterr("BREPKY",QLOCTB,itostr(loc));
    }
    else {
        ioerr = brdblk(blk,&idx);
        if (idx < 0) {
            bterr("BREPKY",QRDBLK,itostr(blk));
        }
        else {
            strcpy(((btact->memrec)+idx)->keyblk[loc],key);
            ((btact->memrec)+idx)->valblk[loc] = val;
            ((btact->memrec)+idx)->lnkblk[loc] = link1;
            ((btact->memrec)+idx)->lnkblk[loc+1] = link2;
            ((btact->cntrl)+idx)->writes++;
#if DEBUG >= 1
            printf("BREPKY: Replaced target at blk: %d, pos: %d\n",blk,loc);
            printf(" ..using '%s', val = %d, llink = %d, rlink = %d\n",
                        key,val,link1,link2);
#endif
        }
    }
    return(0);
}
