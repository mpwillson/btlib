/*
  bpull: pull integer value off stack

      int bpull()

  bpush: push integer value on stack

      int bpush(int val)

          val    value to push on stack

  bstkin: initialise stack pointer

      void bstkin()

*/

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int bpull()
{
    int val;

    if (btact->cntxt->stk.stkptr < 0) 
        bterr("BPULL",QSTKUF,0);
    else {
        val = btact->cntxt->stk.stk[btact->cntxt->stk.stkptr];
        btact->cntxt->stk.stkptr--;
    }
    return(val);
}

int bpush(int val)
{
    if (btact->cntxt->stk.stkptr >= STKMAX) {
        bterr("BPUSH",QSTKOF,0);
    }
    else {
        btact->cntxt->stk.stkptr++;
        btact->cntxt->stk.stk[btact->cntxt->stk.stkptr] = val;
    }
    return(0);
}

void bstkin()
{
    btact->cntxt->stk.stkptr = -1;
}
