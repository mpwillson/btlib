/*
  brepky: replaces key at location loc in block

  int brepky(int blk,int loc,char *key,int val,int link1,int link2)

    blk    block for which replacement is required
    loc    location in block to store information
    key    name of key
    val    value of key
    link1  left link pointer
    link2  right link pointer

*/

#include <stdio.h>
#include <string.h>

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int brepky(int blk,int loc,char *key,int val,int link1,int link2)
{
    int idx,ioerr;

    if (loc >= ZMXKEY || loc < 0) {
        bterr("BREPKY",QLOCTB,itostr(loc));
    }
    else {
        ioerr = brdblk(blk,&idx);
        if (idx < 0) {
            bterr("BREPKY",QRDBLK,itostr(blk));
        }
        else {
            strcpy(((btact->memrec)+idx)->keyblk[loc],key);
            ((btact->memrec)+idx)->valblk[loc] = val;
            ((btact->memrec)+idx)->lnkblk[loc] = link1;
            ((btact->memrec)+idx)->lnkblk[loc+1] = link2;
            ((btact->cntrl)+idx)->writes++;
#if DEBUG >= 1
            printf("BREPKY: Replaced target at blk: %d, pos: %d\n",blk,loc);
            printf(" ..using '%s', val = %d, llink = %d, rlink = %d\n",
                        key,val,link1,link2);
#endif
        }
    }
    return(0);
}
