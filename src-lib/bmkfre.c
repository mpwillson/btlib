/* 
   bmkfre: return block to free list

   void bmkfre(int blk)

        blk    number of block to free up
*/
    
#include <stdio.h>
#include "bc.h"
#include "bt.h"
#include "btree_int.h"

void bmkfre(int blk)
{
    bsetbk(blk,ZFREE,0,btact->cntxt->super.sfreep,0,0);
    btact->cntxt->super.sfreep = blk;
    btact->cntxt->super.snfree++;
    btact->cntxt->stat.xrel++;
    btact->cntxt->super.smod++;     /* super block updated */

    return;
}




