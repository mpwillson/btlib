/*      bdelky:  deletes key in index

        int bdelky(BTA *b,char *key,int *ok)

             b       index context pointer
             key     name of key to delete 
             ok      returned TRUE if deletion occurred 
*/

#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

int bdelky(BTA *b,char *key,int *ok)
{

    int val,ierr;

    bterr("",0,0);
    if ((ierr=bvalap("BDELKY",b)) != 0) return(ierr);
    btact = b;

    if (btact->shared) {
        if (!block()) {
            bterr("BDELKY",QBUSY,0);
            goto fin;
        }
    }

    if (b->cntxt->super.smode != 0) {
        *ok = FALSE;
    }
    else {
        ierr = bfndky(b,key,&val,ok);
        if (*ok && (ierr == 0)) {
            bdelk1(key,ok);
        }
    }
fin:
    if (btact->shared) bulock();
    return(btgerr());
}
