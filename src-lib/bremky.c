/* 
   bremky: deletes key (and rlink) at pos from blk

   void bremky(int blk,int pos)

        blk   number of block from which removal required
        pos   position of key and rlink 
*/

#include <string.h>

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

void bremky(int blk,int pos)
{
    int idx,i,ioerr;

    ioerr = brdblk(blk,&idx);
    if (idx < 0) {
        bterr("BREMKY",QRDBLK,ioerr);
        goto fin;
    }
    ((btact->memrec)+idx)->infblk[ZNKEYS]--;
    for (i=pos;i<((btact->memrec)+idx)->infblk[ZNKEYS];i++) { 
        strcpy(((btact->memrec)+idx)->keyblk[i],
               ((btact->memrec)+idx)->keyblk[i+1]);
        ((btact->memrec)+idx)->valblk[i] = ((btact->memrec)+idx)->valblk[i+1];
        ((btact->memrec)+idx)->lnkblk[i+1] = ((btact->memrec)+idx)->lnkblk[i+2];
    }

    ((btact->cntrl)+idx)->writes++;
fin:
    return;
}
