/*
 * $Id: bgtinf.c,v 1.7 2004/10/02 16:10:08 mark Exp $
 *
 * bgtinf: get information about block
 *
 * Parameters:
 *   blk    block number on which info is required
 *   type   type of information required
 *   
 * bgtinf returns the info requested
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


#include "bc.h"
#include "bt.h"
#include "btree_int.h"

#define MASK ((((BTint) 1)<<((ZBPW/2)*ZBYTEW))-1)

BTint bgtinf(BTint blk,int type)
{
    BTint val;
    int ioerr,idx;

    val = 0;
    if (type >= ZINFSZ)
        bterr("BGTINF",QINFER,NULL);
    else {
        ioerr = brdblk(blk,&idx);
        if (idx < 0) {
            bterr("BGTINF",QRDBLK,itostr((long) blk));
        }
        else {
            switch (type) {
                case ZBTYPE:
                    val = ((btact->memrec)+idx)->infblk[ZBTYPE] & MASK;
                    break;
                case ZBTVER:
                    val = ((btact->memrec)+idx)->infblk[ZBTYPE] >>
                        ((ZBPW/2)*ZBYTEW);
                    break;
                default:
                    val = ((btact->memrec)+idx)->infblk[type];
            }
        }
    }
    return(val);
}
