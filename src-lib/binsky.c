/*
  binsky:  inserts key into index (duplicates not permitted)

  int binsky(BTA *b, char *key,int val)

     b      pointer to BT context      
     key    key to insert
     val    value of key 

   binsky returns non-ZERO if error occurred


*/

#include "btree.h"
#include "btree_int.h"

int binsky(BTA *b, char *key,int val)
{
    int lval,status;

    bterr("",0,NULL);
    if ((status=bvalap("BINSKY",b)) != 0) return(status);

    btact = b;
    if (btact->shared) {
        if (!block()) {
            bterr("BINSKY",QBUSY,NULL);
            goto fin;
        }
    }

    if (b->cntxt->super.smode != 0) {
        /* read only, can't insert */
        bterr("BINSKY",QNOWRT,NULL);
    }
    else {
        status = bfndky(b,key,&lval);
        if (status == QNOKEY) {
            /* QNOKEY is not an error in this context; remove it */
            bterr("",0,NULL);
            bputky(btact->cntxt->lf.lfblk,key,val,ZNULL,ZNULL);
        }
        else {
            bterr("BINSKY",QDUP,key);
        }
    }
fin:
    if (btact->shared) bulock();
    return(btgerr());
}
