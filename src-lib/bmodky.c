/*
  bmodky: replaces value of key at location loc in block

  int bmodky(int blk,int loc,int val)

    blk    block for which replacement is required
    loc    location in block to store information
    val    new value of key

*/

#include <stdio.h>
#include <string.h>

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int bmodky(int blk,int loc,int val)
{
    int idx,ioerr;

    if (loc >= ZMXKEY || loc < 0) {
        bterr("BMODKY",QLOCTB,itostr(loc));
    }
    else {
        ioerr = brdblk(blk,&idx);
        if (idx < 0) {
            bterr("BMODKY",QRDBLK,itostr(blk));
        }
        else {
            ((btact->memrec)+idx)->valblk[loc] = val;
            ((btact->cntrl)+idx)->writes++;
#if DEBUG >= 1
            printf("BMODKY: Modifed target at blk: %d, pos: %d\n",blk,loc);
            printf(" ..using new val = %d\n",val);
#endif
        }
    }
    return(0);
}
