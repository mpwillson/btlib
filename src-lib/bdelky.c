/*      bdelky:  deletes key in index

        int bdelky(BTA *b,char *key)

             b       index context pointer
             key     name of key to delete 
*/

#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

int bdelky(BTA *b,char *key)
{

    int val,status;

    bterr("",0,NULL);
    if ((status=bvalap("BDELKY",b)) != 0) return(status);
    btact = b;

    if (btact->shared) {
        if (!block()) {
            bterr("BDELKY",QBUSY,NULL);
            goto fin;
        }
    }

    if (b->cntxt->super.smode != 0) {
        bterr("BDELKY",QNOWRT,NULL);
    }
    else {
        status = bfndky(b,key,&val);
        if (status == 0) {
            status = bdelk1(key);
        }
    }
fin:
    if (btact->shared) bulock();
    return(btgerr());
}
