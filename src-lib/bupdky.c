/*
  bupdky:  updates value of  key 

     int bupdky(BTA *b, char *key,int val,int *ok)

         b      pointer to BT context
         key    key to update
         val    new value of key 
         
     bupdky returns non-ZERO if error occurred


*/

#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

int bupdky(BTA *b, char *key,int val)
{
    int lval,status;

    bterr("",0,NULL);
    if ((status=bvalap("BUPDKY",b)) != 0) return(status);

    btact = b;
    if (btact->shared) {
        if (!block()) {
            bterr("BUPDKY",QBUSY,NULL);
            goto fin;
        }
    }

    if (b->cntxt->super.smode != 0) {
        /* read only, can't update */
        bterr("BUPDKY",QNOWRT,NULL);
    }
    else {
        status = bfndky(b,key,&lval);
        if (status == 0) {
            bmodky(btact->cntxt->lf.lfblk,btact->cntxt->lf.lfpos,val);
        }
    }
fin:
    if (btact->shared) bulock();
    return(btgerr());
}
