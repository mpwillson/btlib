/*  balblk: attempts to balance blocks */

#include <stdlib.h>
#include "bc.h"
#include "bt.h"
#include "btree_int.h"

void balblk() 
{
    int nkeys,cnkeys,val,llink,rlink,result,diff,cblk;
    char tkey[ZKYLEN];

    cblk = btact->cntxt->lf.lfblk;
    /* get parent block */
    btact->cntxt->lf.lfpos = bpull();
    btact->cntxt->lf.lfblk = bpull();
    nkeys = bgtinf(btact->cntxt->lf.lfblk,ZNKEYS);
    cnkeys = bgtinf(cblk,ZNKEYS);
    if (btact->cntxt->lf.lfpos < nkeys) {
        /* check right sister block */
        bsrhbk(btact->cntxt->lf.lfblk,tkey,&btact->cntxt->lf.lfpos,&val,
               &llink,&rlink,&result);
        if (result != 0) {
            bterr("BALBLK",QBALSE,NULL);
            goto fin;
        }
        nkeys = bgtinf(rlink,ZNKEYS);
        diff = cnkeys-nkeys;
        if (abs(diff) > (int)(ZMXKEY/2)) {
            balbk1(cblk,rlink,diff,tkey,val);
            goto fin;
        }
    }
    if (btact->cntxt->lf.lfpos > 0) {
        /* check left sister block */
        btact->cntxt->lf.lfpos--;
        bsrhbk(btact->cntxt->lf.lfblk,tkey,&btact->cntxt->lf.lfpos,
               &val,&llink,&rlink,&result);
        if (result != 0) {
            bterr("BALBLK",QBALSE,NULL);
            goto fin;
        }
        nkeys = bgtinf(llink,ZNKEYS);
        diff = nkeys-cnkeys;
        if (abs(diff) > (int)(ZMXKEY/2)) {
            balbk1(llink,cblk,diff,tkey,val);
            goto fin;
        }
        btact->cntxt->lf.lfpos++;
    }
    /* no balancing possible; restore original environment */
    bpush(btact->cntxt->lf.lfblk);
    bpush(btact->cntxt->lf.lfpos);
    btact->cntxt->lf.lfblk = cblk;
fin:
    return;
}
