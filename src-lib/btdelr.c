/*
 * $id$
 *
 * btdelr: delete root name from super root, and all the blocks
 *          belonging to the root.
 *
 * Parameters:
 *    b       index context pointer
 *    root    name of root to delete
 *
 * Returns zero if no errors, error code otherwise
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
#include "btree.h"
#include "btree_int.h"

int btdelr(BTA *b,char *root)
{
    int svblk,blk,status;
    int thisblk;
    
    bterr("",0,NULL);
    if ((status=bvalap("BTDELR",b)) != 0) return(status);
    btact = b;

    if (btact->shared) {
        if (!block()) {
            bterr("BTDELR",QBUSY,NULL);
            goto fin;
        }
    }

    /* save current root */
    svblk = btact->cntxt->super.scroot;
    if (btact->cntxt->super.scroot >= 0) {
        bsetbs(btact->cntxt->super.scroot,FALSE);
    }
    /* make super current and find named root */
    btact->cntxt->super.scroot = ZSUPER;
    bsetbs(btact->cntxt->super.scroot,TRUE);
    status = bfndky(b,root,&blk);
    if (status != 0) goto fin;
    if (blk == svblk) {
        /* can't delete current root */
        bsetbs(btact->cntxt->super.scroot,FALSE);
        btact->cntxt->super.scroot = svblk;
        bsetbs(btact->cntxt->super.scroot,TRUE);
        bterr("BTDELR",QDELCR,NULL);
        goto fin;
    }
    /* delete key from superroot */
    if (status == 0) status = bdelky(b,root);
    btact->cntxt->super.smod++;     /* super root updated */
    if (status != 0) goto fin;
    bclrlf();
    if (status == 0) {
        /* unbusy super root */
        bsetbs(btact->cntxt->super.scroot,FALSE);
        /* make name root current and free all blocks */
        btact->cntxt->super.scroot = blk;
        bsetbs(btact->cntxt->super.scroot,TRUE);
        /* free any data blocks */
        blk = bgtinf(btact->cntxt->super.scroot,ZNXBLK);
        while (blk != ZNULL) {
            thisblk = blk;
            blk = bgtinf(blk,ZNXBLK);
            bmkfre(thisblk);
        }
        /* free index blocks */
        blk = ZNULL;
        bnxtbk(&blk);
        while (blk != btact->cntxt->super.scroot) {
            bmkfre(blk);
            bnxtbk(&blk);
        }
        bsetbs(btact->cntxt->super.scroot,FALSE);
        bmkfre(btact->cntxt->super.scroot);
    }
    /* return to saved root  */
    btact->cntxt->super.scroot = svblk;
    if (btact->cntxt->super.scroot >= 0)
        bsetbs(btact->cntxt->super.scroot,TRUE);
  fin:
    if (btact->shared) bulock();
    return(btgerr());
}
