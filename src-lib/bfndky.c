/*
    bfndky: finds key in index

    int bfndky(BTA *b,char *key,int *val)

        b      index file context pointer
        key    key to search for
        val    returned with key value, if key found
     
    index is left positioned at next key (for use by bnxtky)
     
    bfndky returns 0 for no errors, error code otherwise
*/


#include <stdio.h>
#include <string.h>
#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

int bfndky(BTA *b,char *key,int *val)
{
    int cblk, index, link1, link2, result, newblk, nokeys, status;
    char lkey[ZKYLEN];
    
    bterr("",0,NULL);
    status = QNOKEY;
    if ((result=bvalap("BFNDKY",b)) != 0) return(result);

    btact = b;      /* set context pointer */

    if (btact->shared) {
        if (!block()) {
            bterr("BFNDKY",QBUSY,NULL);
            goto fin;
        }
    }

    /* take local copy of key, truncating if necessary */
    strncpy(lkey,key,ZKYLEN);
    lkey[ZKYLEN-1] = '\0';
    
    /* initialise stack etc */
    btact->cntxt->lf.lfexct = FALSE;
    cblk = btact->cntxt->super.scroot;
    bstkin();
    btact->cntxt->lf.lfblk = -1;
    btact->cntxt->lf.lfpos = -1;
    strcpy(btact->cntxt->lf.lfkey,lkey);

    while (cblk >= 0) {
#if DEBUG >= 2
        fprintf(stderr,"BFNDKY: searching block %d\n",cblk);
#endif      
        nokeys = bgtinf(cblk,ZNKEYS);
        if (nokeys == ZMXKEY && btact->cntxt->super.smode == 0) {
            /* split if block full and updating permitted */
            bsptbk(cblk,&newblk);
            if (newblk < 0) {
                bterr("BFNDKY",QSPLIT,NULL);
                break;
            }
            /* if split occured, then must re-examine parent */
            if (cblk != btact->cntxt->super.scroot) {
                index  = btact->cntxt->lf.lfpos;
                cblk = btact->cntxt->lf.lfblk;
                btact->cntxt->lf.lfpos = bpull();
                btact->cntxt->lf.lfblk = bpull();
            }
        }
        else {
            index = -1;
            bpush(btact->cntxt->lf.lfblk);
            bpush(btact->cntxt->lf.lfpos);
            bsrhbk(cblk,lkey,&index,val,&link1,&link2,&result);
            btact->cntxt->lf.lfblk = cblk;
            btact->cntxt->lf.lfpos = index; /* if block is empty, leave lfpos at -1 */
            if (result < 0) {
                /* must examine left block */
                cblk = link1;
                continue;
            }
            else if (result > 0) {
                /* must examine right block */
                cblk = link2;
                /* increment index to indicate this key "visited" */
                btact->cntxt->lf.lfpos++;
                continue;
            }
            else {
                status = 0;
                btact->cntxt->lf.lfexct = TRUE;
                break;
            }
        }
    }  
fin:
    if (btact->shared) bulock();
    /* non-zero status indicates no such key found */
    if (status) bterr("BFNDKY",QNOKEY,lkey);
    return(btgerr());
}
