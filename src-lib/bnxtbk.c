/*
  bnxtbk:  returns next block from index file

  int bnxtbk(int *blk)

      blk - current block (or ZNULL for start from root)
      
    current root (scroot) is returned as last block.  
    blk must be ZNULL on first call.

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
