/*
  bdelk1:  deletes key in index (does the real work)

  int bdelk1(char *key)

        key    name of key to delete

   bdelky returns 0 on succesful deletion, error-code otherwise
   
*/

#include <stdio.h>
#include <string.h>

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int bdelk1(char *key)
{
    int cblk,link1,link2,llink,rlink,val,result,blk,pos,type;
    char tkey[ZKYLEN];

    if (!btact->cntxt->lf.lfexct) {
        bterr("BDELK1",QDELEX,NULL);
        goto fin;
    }
    pos = btact->cntxt->lf.lfpos;
    blk = btact->cntxt->lf.lfblk;
    bsrhbk(blk,tkey,&pos,&val,&llink,&rlink,&result);
    if (result != 0 || strcmp(tkey,key) != 0) {
        bterr("BDELK1",QDELER,NULL);
        goto fin;
    }
    if (llink != ZNULL) {
        /* key not in leaf block, get rightmost leaf key to replace
         * deleted key */
        bleaf(1);
#if DEBUG >= 2
        printf("BDELK1: After bleaf() lfblk: %d, lfpos: %d\n",
               btact->cntxt->lf.lfblk,btact->cntxt->lf.lfpos);
#endif
        bsrhbk(btact->cntxt->lf.lfblk,tkey,&(btact->cntxt->lf.lfpos),
               &val,&link1,&link2,&result);
        if (result != 0) {
            bterr("BDELK1",QDELRP,NULL);
            goto fin;
        }
        brepky(blk,pos,tkey,val,llink,rlink);
        llink = ZNULL;
    }
    bremky(btact->cntxt->lf.lfblk,btact->cntxt->lf.lfpos);

    type = bgtinf(btact->cntxt->lf.lfblk,ZBTYPE);
    if (type == ZROOT) goto fin;
    /* deleted key is in leaf block (one way or the other); try to
     * join adjacent blocks, so one can be freed */
    bjnblk(&cblk);
    if (cblk == ZNULL) {
        /* couldn't join, see if balancing required */
        balblk();
    }
    /* if blocks joined, check parent.  If empty, must demote its
     * parent key */
    while (cblk != ZNULL) {
        bdemte(&cblk);
    }
fin:
    return (btgerr());
}
