/*
  bgtinf: get information about block

  int bgtinf(int blk,int type)

    blk    block number on which info is required
    type   type of information required

  bgtinf returns the info requested

*/


#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int bgtinf(int blk,int type)
{
    int val,ioerr,idx;

    val = 0;
    if (type >= ZINFSZ)
        bterr("BGTINF",QINFER,NULL);
    else {
        ioerr = brdblk(blk,&idx);
        if (idx < 0)
            bterr("BGTINF",QRDBLK,NULL);
        else
            val = ((btact->memrec)+idx)->infblk[type];

    }
    return(val);
}
