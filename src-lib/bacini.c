/*  bacini: Initialise BTA structure, malloc'ing space as required
 *
 *      b - pointer to BT context
 */

#include <stdlib.h>
#include <stdio.h>

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int bacini(BTA *b)
{

    /* get file descriptor for bt file (used for file locking) */
    btact->fd = btact->idxunt->FILENO;
    btact->lckcnt = 0;
    
    if ((b->cntrl=(CNTRL *) malloc(sizeof(CNTRL)*ZMXBLK)) == NULL)
        goto err;
    if ((b->memrec=(MEMREC *) malloc(sizeof(MEMREC)*ZMXBLK)) == NULL)
        goto err;
    if ((b->cntxt=(CNTXT *) malloc(sizeof(CNTXT))) == NULL)
        goto err;

    initcntrl(b);
    
    bclrlf();
    bclrst();
    return(0);
err:
    bterr("BACINI",QNOMEM,NULL);
    return(QNOMEM);
}

void initcntrl(BTA *b)
{
    CNTRL *c;
    int i;

    /* initialise lru queue */
    b->cntxt->lru.lrut = -1;
    b->cntxt->lru.lruh = -1;

    /* initialise control data */
    c = b->cntrl;
    for (i=0;i<ZMXBLK;i++) {
        (c+i)->inmem = -1;
        (c+i)->busy = 0;
        (c+i)->writes = 0;
        bqadd(i);
    }
}
