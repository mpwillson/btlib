/*
  bjoin: bjoins keys in rblk to lblk (using tkey)

  void bjoin(int lblk,int rblk,char *tkey,int val)

        lblk   left block of joining pair
        rblk   right block of joining pair
        tkey   name of parent key
        val    value of parent key

   Parent block of left and right siblings is in last found context.
*/

#include <stdio.h>

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

void bjoin(int lblk,int rblk,char *tkey,int val)
{
    int lnkeys,rnkeys;

#if DEBUG >= 1
    printf("BJOIN: joining lblk: %d, rblk: %d, using parent: %s\n",
            lblk,rblk,tkey);
#endif
    bremky(btact->cntxt->lf.lfblk,btact->cntxt->lf.lfpos);
    bsetbs(lblk,1);
    bputky(lblk,tkey,val,ZNULL,ZNULL); 
    lnkeys = bgtinf(lblk,ZNKEYS);
    rnkeys = bgtinf(rblk,ZNKEYS);
    bcpybk(lblk,rblk,lnkeys,0,rnkeys); 
    bstinf(lblk,ZNKEYS,lnkeys+rnkeys);
    bmkfre(rblk);
    bsetbs(lblk,0);
    btact->cntxt->stat.xjoin++;
    return;
}
