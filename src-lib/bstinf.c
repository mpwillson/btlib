/*
  bstinf: set information about block

  int bstinf(int blk,int type,int val)

    blk    block for which information must be set
    type   type of information to set
    val    info value

*/


#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int bstinf(int blk,int type,int val)
{
    int ioerr,idx;

    if (type >= ZINFSZ)
        bterr("BSTINF",QINFER,NULL);
    else {
        ioerr = brdblk(blk,&idx);
        if (idx < 0) {
            bterr("BSTINF",QRDBLK,itostr(blk));
        }
        else {
            ((btact->memrec)+idx)->infblk[type] = val;
            ((btact->cntrl)+idx)->writes++;
        }
    }
    return(0);
}


