/*
 * $Id$
 *
 *
 * bnxtbk:  returns next block from index file
 *
 * Parameters:
 *   blk - current block (or ZNULL for start from root)
 *     
 *   current root (scroot) is returned as last block.  
 *   blk must be ZNULL on first call.
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

int bnxtbk(int *blk)
{
    int idx,nkeys,ioerr;

    /* if blk is ZNULL, first call; initialise stack and position
    at leftmost leaf node */
    
    if (*blk == ZNULL) {
        bstkin();
        bpush(-1);
        bpush(-1);
        btact->cntxt->lf.lfblk = btact->cntxt->super.scroot;
        btact->cntxt->lf.lfpos = 0;
        bleaf(0);
    }

    while (btact->cntxt->lf.lfblk >= 0) {
        ioerr = brdblk(btact->cntxt->lf.lfblk,&idx);
        if (idx < 0) {
            bterr("BNXTBK",QRDBLK,itostr(btact->cntxt->lf.lfblk));
            break;
        }
        nkeys = bgtinf(btact->cntxt->lf.lfblk,ZNKEYS);
        if (btact->cntxt->lf.lfpos > nkeys) {
            /* finished with this block, get parent from stack */
            *blk = btact->cntxt->lf.lfblk;
            btact->cntxt->lf.lfpos = bpull();
            btact->cntxt->lf.lfblk = bpull();
            btact->cntxt->lf.lfpos++;
            break;
        }
        /* if rlink to process, walk to leftmost leaf of this branch */
        bleaf(0);
        btact->cntxt->lf.lfpos = bgtinf(btact->cntxt->lf.lfblk,ZNKEYS);
        btact->cntxt->lf.lfpos += 2;
    }
    return(0);
}
