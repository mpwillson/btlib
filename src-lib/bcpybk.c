/*
  bcpybk: copies contents of one block to another

  int bcpybk(int tblk,int fblk,int ts,int fs,int n)

      tblk   block number of to-block
      fblk   block number of from-block
      ts     first key number in to-block
      fs     first key number in from-block
      n      number of keys to copy
*/


#include <string.h>
#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int bcpybk(int tblk,int fblk,int ts,int fs,int n)
{
    int tidx,fidx,inc,i,j,ioerr;

    if (ts == 0)
        inc = 0;
    else
        inc = 1;

    /* get to-block in memory */
    ioerr = brdblk(tblk,&tidx);
    if (tidx < 0) {
        bterr("BCPYBK",QCPBLK,NULL);
        goto fin;
    }
    /* wire it down to ensure that it isn't replaced by from-block */
    bsetbs(tblk,1);
    /* get from-block in memory */
    ioerr = brdblk(fblk,&fidx);
    /* can un-busy to-block now */
    bsetbs(tblk,0);
    if (fidx < 0) {
        bterr("BCPYBK",QCPBLK,NULL); 
        goto fin;
    }
    /* copy keys and links */
    j = fs;
    for (i=ts;i<ts+n;i++) {
        strcpy(((btact->memrec)+tidx)->keyblk[i],
            ((btact->memrec)+fidx)->keyblk[j]);
        ((btact->memrec)+tidx)->valblk[i] = ((btact->memrec)+fidx)->valblk[j];
        ((btact->memrec)+tidx)->lnkblk[i+inc] = 
            ((btact->memrec)+fidx)->lnkblk[j];
        j = j+1;
    }
    ((btact->memrec)+tidx)->lnkblk[ts+n] = ((btact->memrec)+fidx)->lnkblk[j];
    ((btact->cntrl)+tidx)->writes += n;
fin:
    return(0);
}
