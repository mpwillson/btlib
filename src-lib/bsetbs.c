/*
  bsetbs: alter busy state of in-memory block

  void bsetbs(int blk,int status)

     blk    block which requires setting
     status new status (1 for busy, 0 for flushable)

*/

#include <stdio.h>
#include "bc.h"
#include "bt.h"
#include "btree_int.h"

void bsetbs(int blk,int status)
{
    int idx,ioerr;

    ioerr = brdblk(blk,&idx);
    if (idx < 0)
        bterr("BSETBS",QRDBLK,ioerr);
    else {
        if (status == 1)
            ((btact->cntrl)+idx)->busy = TRUE;
        else
            ((btact->cntrl)+idx)->busy = FALSE;
    }
    return;
}
