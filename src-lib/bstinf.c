/*
  bstinf: set information about block

  int bstinf(int blk,int type,int val)

    blk    block for which information must be set
    type   type of information to set
    val    info value

*/
#define MASK 2**((ZBPW/2)*ZBYTEW)-1


#include "bc.h"
#include "bt.h"
#include "btree_int.h"
#include <math.h>

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
            switch (type) {
                case ZBTYPE:
                    ((btact->memrec)+idx)->infblk[type] =
                        (ZVERS << ((ZBPW/2)*ZBYTEW)) | val;
                case ZBTVER:
                    break; /* always set implicitly by ZBTYPE */
                default:
                    ((btact->memrec)+idx)->infblk[type] = val;
            }
            ((btact->cntrl)+idx)->writes++;
        }
    }
    return(0);
}


