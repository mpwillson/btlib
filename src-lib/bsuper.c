/*
 * brdsup  - reads super root
 * bwrsup  - writes super root
 *
 * Both return 0 for success, error code otherwise
 * 
 */

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

/* get super root from disk */
int brdsup()
{
    int ioerr,idx;

    ioerr = brdblk(ZSUPER,&idx);
    if (ioerr != 0) {
        bterr("BRDSUP",QRDBLK,0);
        goto fin;
    } 
    if (((btact->memrec)+idx)->infblk[ZBTYPE] != ZROOT) {
        bterr("BRDSUP",QSRNR,0);
        ioerr = QSRNR;
        goto fin;
    }
    /* retain free list pointers et al */
    btact->cntxt->super.snfree = ((btact->memrec)+idx)->infblk[ZMISC];
    btact->cntxt->super.sfreep = ((btact->memrec)+idx)->infblk[ZNXBLK];
    btact->cntxt->super.sblkmx = ((btact->memrec)+idx)->infblk[ZNBLKS];
    return(0);
fin:
    return(ioerr);
}

/* update super root on disk */
int bwrsup()
{
    int ioerr,idx,nkeys;

    ioerr = brdblk(ZSUPER,&idx);
    if (ioerr != 0) {
        bterr("BWRSUP",QRDSUP,ioerr);
        goto fin;
    }
    nkeys = bgtinf(ZSUPER,ZNKEYS);
    bsetbk(ZSUPER,ZROOT,btact->cntxt->super.snfree,
        btact->cntxt->super.sfreep,
        nkeys,
        btact->cntxt->super.sblkmx);
    ioerr = bwrblk(ZSUPER);
    if (ioerr != 0) {
        bterr("BWRSUP",QWRSUP,ioerr);
        goto fin;
    }
    return(0);
fin:
    return(ioerr);
}
