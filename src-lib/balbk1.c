/*
  balbk1: balances keys between blocks 

  void balbk1(int lblk,int rblk,int diff,char *key,int val)

        lblk   left block of pair 
        rblk   right block of pair 
        diff   if negative, move diff/2 keys left 
               else move diff/2 keys right 
        key    name of current parent key for both blocks 
        val    value of parent key
*/

#include <stdlib.h>
#include <stdio.h>

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

void balbk1(int lblk,int rblk,int diff,char *key,int val)
{

    int i,tblk,fblk,keypos,dir,tval,link1,link2,result;
    int limit;
    char tkey[ZKYLEN];

#if DEBUG >= 1
    printf("BALBK1: Balancing keys between lblk: %d, rblk: %d\n",lblk,rblk);
    printf("        with parent: %s\n",key);
#endif
    if (diff < 0) {
        dir = 1;
        tblk = lblk;
        fblk = rblk;
        keypos = 0; 
        limit = abs(diff/2);
    }
    else {
        dir = -1;
        tblk = rblk;
        fblk = lblk;
        keypos = bgtinf(fblk,ZNKEYS)-1;
        limit = keypos-abs(diff/2);
    }
    /* move parent key into to block */
    bputky(tblk,key,val,ZNULL,ZNULL);
    bremky(btact->cntxt->lf.lfblk,btact->cntxt->lf.lfpos);
    /* move keys from fblk to tblk */
    for (i=keypos;i!=limit;i+=dir) {
        bsrhbk(fblk,tkey,&keypos,&tval,&link1,&link2,&result);
        if (result != 0) {
            bterr("BALBK1",QBALSE,0);
            goto fin;
        }
        bremky(fblk,keypos);
        if (dir < 0) keypos--;
        bputky(tblk,tkey,tval,ZNULL,ZNULL);
    }
    /* move last key from fblk into parent */
    i = keypos;
    bsrhbk(fblk,tkey,&keypos,&tval,&link1,&link2,&result);
    if (result != 0) {
        bterr("BALBK1",QBALSE,0);
        goto fin;
    }
    bputky(btact->cntxt->lf.lfblk,tkey,tval,lblk,rblk);
    bremky(fblk,keypos);
    btact->cntxt->stat.xbal++;
fin:
    return;
}
