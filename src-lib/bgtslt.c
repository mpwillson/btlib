/*
  bgtslt: frees in-memory slot and returns index

  bgtslt returns index of free memory block

*/

#include <stdio.h>
#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int bgtslt()
{
    int idx, i;

    idx = -1;
    /* any slots free? */
    for (i=0;i<ZMXBLK;i++) {
#if DEBUG >= 2
        printf("bgtslt: cntrl[%d].inmem = %d\n",i,((btact->cntrl)+i)->inmem);
#endif
        if (((btact->cntrl)+i)->inmem < 0) {
            idx = i;
            break;
        }
    }
    if (idx < 0 ) {
        /* nope, need to free a memory block (least recently used)  */
        idx = bqhead();
    }
    if (idx < 0)
        bterr("BGTSLT",QNOMEM,NULL);
    else {
        /* flush block to disk if slot not empty */
        if (((btact->cntrl)+idx)->inmem >= 0)
            if (bwrblk(((btact->cntrl)+idx)->inmem) != 0)
                bterr("BGTSLT",QWRBLK,NULL);
    }
#if DEBUG >= 2
    printf("bgtslt: found idx of %d\n",idx);
#endif
    return(idx);
}
