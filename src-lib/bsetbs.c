/*
  bsetbs: alter busy state of in-memory block

  void bsetbs(int blk,int busy)

     blk    block which requires setting
     busy   new status (TRUE for busy, FALSE for flushable)

*/

#include <stdio.h>
#include "bc.h"
#include "bt.h"
#include "btree_int.h"

void bsetbs(int blk,int busy)
{
    int idx,status;

    status = brdblk(blk,&idx);
    if (idx < 0) {
        bterr("BSETBS",QRDBLK,itostr(blk));
    }
    else {
        ((btact->cntrl)+idx)->busy = busy;
    }
    return;
}
