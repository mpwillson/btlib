#include <stdio.h>
#include "bc.h"
#include "bt.h"
#include "btree_int.h"

/*
  bwrblk: write block from memory to disk

    int bwrblk(int blk)

      blk    number of block to write to disk

    ioerr  returned non-ZERO if an i/o error occurred

*/

int bwrblk(int blk)
{

    int i,ioerr;

    ioerr = 0;
    for (i=0;i<ZMXBLK;i++)
        if (((btact->cntrl)+i)->inmem == blk) break;

    if (i == ZMXBLK) 
        bterr("BWRBLK",QWRMEM,blk);
    else {
        if (((btact->cntrl)+i)->writes != 0) {
            ioerr = fseek(btact->idxunt,(long) blk*ZBLKSZ,0);
            if (ioerr == 0) {
                if ((ioerr = fwrite((btact->memrec)+i,sizeof(char),
                        ZBLKSZ, btact->idxunt)) == ZBLKSZ) {
                    btact->cntxt->stat.xphywr++;
                    ioerr = 0;
#if DEBUG > 0
                    fprintf(stderr,"..writing block %d, from idx %d\n",blk,i);
#endif                  
                }
                else {
                    ioerr = -1;
                }
            }
        }
        else {
             btact->cntxt->stat.xlogwr++;
        }
    }
    ((btact->cntrl)+i)->writes = 0;
    return(ioerr);
}
