/*
  binsky:  inserts key into index (duplicates not permitted)

  int binsky(BTA *b, char *key,int val,int *ok)

     b      pointer to BT context      
     key    key to insert
     val    value of key 
     ok     returned TRUE if key inserted

   binsky returns non-ZERO if error occurred


*/

#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"
int binsky(BTA *b, char *key,int val,int *ok)
{
    int lval,ierr,found;

    bterr("",0,0);
    if ((ierr=bvalap("BINSKY",b)) != 0) return(ierr);

    btact = b;
    if (btact->shared) {
        if (!block()) {
            bterr("BINSKY",QBUSY,0);
            goto fin;
        }
    }

    if (b->cntxt->super.smode != 0) 
        /* read only, can't insert */
        *ok = FALSE;
    else {
        ierr = bfndky(b,key,&lval,&found);
        if (!found && ierr == 0) {
            bputky(btact->cntxt->lf.lfblk,key,val,ZNULL,ZNULL);
            *ok = TRUE;
        }
        else 
            *ok = FALSE;
    }
fin:
    if (btact->shared) bulock();
    return(btgerr());
}
