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
        bterr("BRDSUP",QRDBLK,NULL);
        goto fin;
    } 
    if (bgtinf(ZSUPER,ZBTYPE) != ZROOT) {
        bterr("BRDSUP",QSRNR,NULL);
        ioerr = QSRNR;
        goto fin;
    }
    if (bgtinf(ZSUPER,ZBTVER) != ZVERS) {
        bterr("BRDSUP",QBADVR,itostr(ZVERS));
        ioerr = QBADVR;
        goto fin;
    }
    
    /* retain free list pointers et al */
    btact->cntxt->super.snfree = bgtinf(ZSUPER,ZMISC);
    btact->cntxt->super.sfreep = bgtinf(ZSUPER,ZNXBLK);
    btact->cntxt->super.sblkmx = bgtinf(ZSUPER,ZNBLKS);
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
        bterr("BWRSUP",QRDSUP,itostr(ZSUPER));
        goto fin;
    }
    nkeys = bgtinf(ZSUPER,ZNKEYS);
    bsetbk(ZSUPER,ZROOT,btact->cntxt->super.snfree,
        btact->cntxt->super.sfreep,
        nkeys,
        btact->cntxt->super.sblkmx);
    ioerr = bwrblk(ZSUPER);
    if (ioerr != 0) {
        bterr("BWRSUP",QWRSUP,itostr(ZSUPER));
        goto fin;
    }
    return(0);
fin:
    return(ioerr);
}
