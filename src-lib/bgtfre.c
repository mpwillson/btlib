/*
  bgtfre: gets free block

  bgtfre returns block number of free block

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
               than the size of INT_MAX */
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
