/*
  bleaf: position to left or right-most leaf block from current position

  int bleaf(int dir)

    dir    direction (0 for left, 1 for right)

*/


#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int bleaf(int dir)
{
    int idx,ioerr;

    ioerr = brdblk(btact->cntxt->lf.lfblk,&idx);
    if (idx < 0)
        bterr("BLEAF",QRDBLK,ioerr);
    else {
        if (bgtinf(btact->cntxt->lf.lfblk,ZNKEYS) == 0) return 0;
        while (((btact->memrec)+idx)->lnkblk[btact->cntxt->lf.lfpos] != ZNULL) {
            bpush(btact->cntxt->lf.lfblk);
            bpush(btact->cntxt->lf.lfpos);
            btact->cntxt->lf.lfblk = 
                ((btact->memrec)+idx)->lnkblk[btact->cntxt->lf.lfpos];
            ioerr = brdblk(btact->cntxt->lf.lfblk,&idx);
            if (idx < 0) {
                bterr("BLEAF",QRDBLK,ioerr);
                break;
            }
            if (dir == 0) 
                /* proceed to leftmost leaf  */
                btact->cntxt->lf.lfpos = 0;
            else {
                /* proceed to rightmost leaf */
                 btact->cntxt->lf.lfpos = bgtinf(btact->cntxt->lf.lfblk,ZNKEYS);
                 /*lfpos++; */
            }
        }
    }
    /* if proceeding right, ensure we point at last key */
    if (dir == 1) btact->cntxt->lf.lfpos--;
    return(0);
}
