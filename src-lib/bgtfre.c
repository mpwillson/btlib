/*
 * $Id: bgtfre.c,v 1.6 2004/09/26 11:49:18 mark Exp $
 *
 * bgtfre: gets free block
 *
 * Parameters:
 *   None
 *
 * bgtfre returns block number of free block
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

#include "bc.h"
#include "bt.h"
#include "btree_int.h"
#include <limits.h>

int bgtfre()
{
    int blk,idx,ioerr,faddr;
    
    if (btact->cntxt->super.sfreep == ZNULL) {
        /* unix can create a new block at file end other systems may
         * have to error return this request */
        
        blk = btact->cntxt->super.sblkmx;
        faddr = blk*ZBLKSZ+ZBLKSZ;
        if (faddr < 0 || faddr >= INT_MAX) {
            /* assume (possibly wrongly) that a file cannot be bigger
               (in terms of bytes) than the size of INT_MAX */
            bterr("BGTFRE",QF2BIG,NULL);
            return ZNULL;
        }
        
        if ((idx = bgtslt()) >= 0) {
            bqmove(idx);
            ((btact->cntrl)+idx)->inmem = blk;
            btact->cntxt->super.sblkmx++;
        }
        else 
            blk = ZNULL;
    }
    else {
        /* get one off the free list */
        blk = btact->cntxt->super.sfreep;
        ioerr = brdblk(blk,&idx);
        if (idx < 0) {
            bterr("BGTFRE",QRDBLK,itostr(blk));
        }
        else {
            if (bgtinf(blk,ZBTYPE) != ZFREE) {
                bterr("BGTFRE",QNOTFR,itostr(blk));
                return(ZNULL);
            }
            else {
                btact->cntxt->super.sfreep = bgtinf(blk,ZNXBLK);
            }
        }
        if (blk >= 0) {
            btact->cntxt->stat.xgot++;
            btact->cntxt->super.snfree--;
        }
    }
    btact->cntxt->super.smod++;     /* super block updated */

    return(blk);
}
