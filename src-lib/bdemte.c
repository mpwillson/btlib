/*
    bdemte: if non-leaf block is empty, demote parent key

    void bdemte(int *cblk)

        cblk   returned potentially dangling block (i.e its parent has 
               no keys).  ZNULL if no dangling block.
*/

#include <stdio.h>

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

void bdemte(int *cblk)
{
    int nkeys,type,newblk,val,link1,link2,result,jblk,llink,rlink;
    int rblke;
    char tkey[ZKYLEN];

    nkeys = bgtinf(btact->cntxt->lf.lfblk,ZNKEYS);
    if (nkeys == 0) {
        type = bgtinf(btact->cntxt->lf.lfblk,ZBTYPE);
        if (type != ZROOT) {
            /* pull parent */
            btact->cntxt->lf.lfpos = bpull();
            btact->cntxt->lf.lfblk = bpull();
            nkeys = bgtinf(btact->cntxt->lf.lfblk,ZNKEYS);
            if (btact->cntxt->lf.lfpos != 0) { 
                /* ensure that empty block is on the right */
                rblke = TRUE;
                btact->cntxt->lf.lfpos--;
            }
            else {
                /* first block empty, so can't put on right */
                rblke = FALSE;
            }
            bsrhbk(btact->cntxt->lf.lfblk,tkey,&btact->cntxt->lf.lfpos,&val,
                   &link1,&link2,&result);
            if (result != 0) {
                bterr("BDEMTE",QDEMSE,NULL);
                goto fin;
            }
#if DEBUG >= 1
            printf("BDEMTE: Demoting key %s from blk: %d\n",tkey,
                   btact->cntxt->lf.lfblk);
#endif
            /* remove key from original block */
            bremky(btact->cntxt->lf.lfblk,btact->cntxt->lf.lfpos);
            /* prepare for split check */
            if (rblke) 
                jblk = link1;
            else
                jblk = link2;
        
            /* if target block for demotion is full, split it */
            nkeys = bgtinf(jblk,ZNKEYS);
            if (nkeys == ZMXKEY) {
                bsptbk(jblk,&newblk);
                if (newblk == ZNULL) {
                    bterr("BDEMTE",QDEMSP,NULL);
                    goto fin;
                }
                if (rblke) {
                    link1 = newblk;
                }
            }
            if (!rblke) {
                /* copy keys from right block to empty left block */
                nkeys = bgtinf(link2,ZNKEYS);
                bsetbs(link1,1);
                bcpybk(link1,link2,0,0,nkeys);
                bstinf(link1,ZNKEYS,nkeys);
                bsetbs(link1,0);
                llink = *cblk;
                rlink = ZNULL; /* 0 replaced by ZNULL for C */
            }
            else {
                llink = ZNULL;  /* 0 replaced by ZNULL for C */
                rlink = *cblk;
            }
            bputky(link1,tkey,val,llink,rlink);
            bmkfre(link2);
            /* set new potentially dangling block */
            *cblk = link1;
        }
        else { 
            /* empty block is root; copy keys from child and then free
             * the child block */
            nkeys = bgtinf(*cblk,ZNKEYS);
            bcpybk(btact->cntxt->lf.lfblk,*cblk,0,0,nkeys);
            bstinf(btact->cntxt->lf.lfblk,ZNKEYS,nkeys);
            bmkfre(*cblk);
            *cblk = ZNULL;
        }
    }
    else {
        *cblk = ZNULL;
    }
fin:
    return;
}
