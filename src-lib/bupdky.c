/*
  bupdky:  updates value of  key 

     int bupdky(BTA *b, char *key,int val,int *ok)

         b      pointer to BT context
         key    key to update
         val    new value of key 
         ok     returned TRUE if key updated
         
     bupdky returns non-ZERO if error occurred


*/

#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

int bupdky(BTA *b, char *key,int val,int *ok)
{
    int lval,ierr,found;

    bterr("",0,0);
    if ((ierr=bvalap("BUPDKY",b)) != 0) return(ierr);

    btact = b;
    if (btact->shared) {
        if (!block()) {
            bterr("BUPDKY",QBUSY,0);
            goto fin;
        }
    }

    if (b->cntxt->super.smode != 0) 
        /* read only, can't update */
        *ok = FALSE;
    else {
        ierr = bfndky(b,key,&lval,&found);
        if (found && ierr == 0) {
            bmodky(btact->cntxt->lf.lfblk,btact->cntxt->lf.lfpos,val);
            *ok = TRUE;
        }
        else 
            *ok = FALSE;
    }
fin:
    if (btact->shared) bulock();
    return(btgerr());
}
