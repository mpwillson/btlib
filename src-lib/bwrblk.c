/*
 * $Id: bwrblk.c,v 1.5 2004/09/26 13:07:39 mark Exp $
 *
 * bwrblk: write block from memory to disk
 *
 * Parameters:
 *    blk    number of block to write to disk
 *
 * bwrblk returns non-ZERO if an i/o error occurred
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
#include <stdio.h>
#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int bwrblk(int blk)
{

    int i,ioerr;
    
    ioerr = 0;
    for (i=0;i<ZMXBLK;i++)
        if (((btact->cntrl)+i)->inmem == blk) break;

    if (i == ZMXBLK) {
        bterr("BWRBLK",QWRMEM,itostr(blk));
    }
    else {
        if (((btact->cntrl)+i)->writes != 0) {
            ioerr = fseek(btact->idxunt,(long) blk*ZBLKSZ,0);
            if (ioerr == 0) {
                if ((ioerr = fwrite((btact->memrec)+i,sizeof(char),
                        ZBLKSZ, btact->idxunt)) == ZBLKSZ) {
                    btact->cntxt->stat.xphywr++;
                    ioerr = 0;
#if DEBUG > 0
                    fprintf(stderr,"..writing block %d, from idx %d\n",blk,i);
#endif                  
                }
                else {
                    ioerr = -1;
                }
            }
        }
        else {
             btact->cntxt->stat.xlogwr++;
        }
    }
    ((btact->cntrl)+i)->writes = 0;
    return(ioerr);
}
