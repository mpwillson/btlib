/*
  btcls: close B tree index file
    
  int btcls(BTA* b)
 
      b  - pointer to BT context
      
   returns zero if no errors, error code otherwise
*/

#include <stdio.h>
#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

int btcls(BTA* b)
{
    int ioerr;
  
    bterr("",0,NULL);

    if ((ioerr=bvalap("BTCLS",b)) != 0) return(ioerr);

    btact = b;      /* set context pointer */

    if (!btact->shared) {
        /* Unlock exclusive access and flush buffers before closing file
         * Buffers will already be flushed to disk for shared access
         */
        bulock();
    }
    
    /* close index file and free context memory */
    ioerr = fclose(btact->idxunt);
    if (ioerr != 0) {
        bterr("BTCLS",QCLSIO,NULL);
    }
    else {
        btact->idxunt = NULL;
        bacfre(btact);
    }
    return(btgerr());
}

/*
  btsync: sync memory blocks with B tree index file
    
  returns zero if no errors, error code otherwise
*/
int btsync()
{
    int i,ioerr;

    if (btact->idxunt != NULL) {
        /* write out all in-memory blocks */
        for (i=0;i<ZMXBLK;i++) {
            ioerr = bwrblk(((btact->cntrl)+i)->inmem);
            if (ioerr != 0) {
                bterr("BTSYNC",QWRBLK,itostr(((btact->cntrl)+i)->inmem));
                goto fin;
            }
        } 
        /* write super block if necessary */
        if (btact->cntxt->super.smod != 0) {
            if ( bwrsup() != 0) goto fin;
            btact->cntxt->super.smod = 0;
        }
    }

    /* force writes to disk */
    fflush(btact->idxunt);

    /* re-initialise control blocks to ensure blocks will be re-read
       from disk */
    initcntrl(btact);
    
fin:
    return(btgerr());
}


