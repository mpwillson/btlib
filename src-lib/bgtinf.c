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

#define MASK ((1<<((ZBPW/2)*ZBYTEW))-1)

int bgtinf(int blk,int type)
{
    int val,ioerr,idx;

    val = 0;
    if (type >= ZINFSZ)
        bterr("BGTINF",QINFER,NULL);
    else {
        ioerr = brdblk(blk,&idx);
        if (idx < 0) {
            bterr("BGTINF",QRDBLK,NULL);
        }
        else {
            switch (type) {
                case ZBTYPE:
                    val = ((btact->memrec)+idx)->infblk[ZBTYPE] & MASK;
                    break;
                case ZBTVER:
                    val = ((btact->memrec)+idx)->infblk[ZBTYPE] >>
                        ((ZBPW/2)*ZBYTEW);
                    break;
                default:
                    val = ((btact->memrec)+idx)->infblk[type];
            }
        }
    }
    return(val);
}
