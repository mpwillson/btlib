/*
  bjnblk: joins leaf blocks if possible 

  void bjnblk(int *cblk)

     cblk   returned as joined block (ZNULL if nothing joined)

   environment is left at parent block

*/
#include "bc.h"
#include "bt.h"
#include "btree_int.h"

void bjnblk(int *cblk)
{
    int nkeys,cnkeys,val,llink,rlink,result;
    char tkey[ZKYLEN];

    *cblk = btact->cntxt->lf.lfblk;
    btact->cntxt->lf.lfpos = bpull();
    btact->cntxt->lf.lfblk = bpull();
    nkeys = bgtinf(btact->cntxt->lf.lfblk,ZNKEYS);
    cnkeys = bgtinf(*cblk,ZNKEYS);
    /* is join with right sister possible? */
    if (btact->cntxt->lf.lfpos < nkeys) {
        bsrhbk(btact->cntxt->lf.lfblk,tkey,&btact->cntxt->lf.lfpos,&val,
               &llink,&rlink,&result);
        if (result != 0) {
            bterr("BJNBLK",QJNSE,0);
            goto fin;
        }
        nkeys = bgtinf(rlink,ZNKEYS);
        if (cnkeys+nkeys < ZMXKEY-ZTHRES) {
            bjoin(*cblk,rlink,tkey,val);
            goto fin;
        }
    }
    /* is join with left sister possible? */
    if (btact->cntxt->lf.lfpos > 0) {
        btact->cntxt->lf.lfpos--;
        bsrhbk(btact->cntxt->lf.lfblk,tkey,&btact->cntxt->lf.lfpos,&val,
               &llink,&rlink,&result);
        if (result != 0) {
            bterr("BJNBLK",QJNSE,0);
            goto fin;
        }
        nkeys = bgtinf(llink,ZNKEYS);
        if (cnkeys+nkeys < ZMXKEY-ZTHRES) {
            bjoin(llink,*cblk,tkey,val);
            *cblk = llink;
            goto fin;
        }
        btact->cntxt->lf.lfpos++;
    }
    /* no join possible; restore environment */
    bpush(btact->cntxt->lf.lfblk);
    bpush(btact->cntxt->lf.lfpos);
    btact->cntxt->lf.lfblk = *cblk;
    *cblk = ZNULL;
fin:
    return;
}
