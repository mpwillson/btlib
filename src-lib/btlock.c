/*
  btlock: locks index file for exclusive use

  int btlock(BTA *b)

     b      index file context pointer
     
  btlock returns 0 for no errors, error code otherwise

  NB  btlock may be called multiple times, but for each call, a
      corresponding call to btunlock must be made.
*/


#include <stdio.h>
#include <string.h>
#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

int btlock(BTA *b)
{
    int result;
    
    bterr("",0,NULL);

    if ((result=bvalap("BTLOCK",b)) != 0) return(result);

    btact = b;      /* set context pointer */
    if (btact->shared) {
        /* only lock if shared */
        if (!block()) {
            bterr("BTLOCK",QBUSY,NULL);
        }
    }
    return(btgerr());
}

/*

  btunlock: unlocks index file 

  int btunlock(BTA *b)

     b      index file context pointer
     
   btunlock returns 0 
*/

int btunlock(BTA *b)
{
    int result;
    
    bterr("",0,NULL);

    if ((result=bvalap("BTUNLOCK",b)) != 0) return(result);

    btact = b;      /* set context pointer */
    if (btact->shared) {
        bulock();
    }
    return(btgerr());
}

