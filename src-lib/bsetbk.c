/*
  bsetbk:  sets info array in block

  int bsetbk(int blk,int type,int misc,int nxblk,int nkeys,int nblks)

    blk    block for which information should be set
    type   block type
    misc   misc info
    nxblk  next block
    nkeys  number of active keys
    nblks  total number of blocks in index file

*/

#include <stdio.h>
#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int bsetbk(int blk,int type,int misc,int nxblk,int nkeys,int nblks)
{
    int ioerr,idx;
    
    ioerr = brdblk(blk,&idx);
    if (idx < 0) {
        bterr("BSETBK",QRDBLK,itostr(blk));
    }
    else {
        ((btact->memrec)+idx)->infblk[ZBTYPE] = type;
        ((btact->memrec)+idx)->infblk[ZMISC] = misc;
        ((btact->memrec)+idx)->infblk[ZNXBLK] = nxblk;
        ((btact->memrec)+idx)->infblk[ZNKEYS] = nkeys;
        ((btact->memrec)+idx)->infblk[ZNBLKS] = nblks;
        ((btact->cntrl)+idx)->writes++;
    }
    return(0);
}
