/*  
 *  btdelr: delete root name from super root, and all the blocks
 *          belonging to the root.
 *
 *      int btdelr(BTA *b,char *root,int *ok) 
 *
 *          b       index context pointer
 *          root    name of root to delete
 *          ok      returned TRUE if root deleted ok, FALSE otherwise
 *
 *  Returns zero if no errors, error code otherwise
 *  
 */

#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

int btdelr(BTA *b,char *root,int *ok) 
{
    int svblk,blk,ierr;
    int thisblk;
    
    bterr("",0,0);
    if ((ierr=bvalap("BTDELR",b)) != 0) return(ierr);
    btact = b;

    if (btact->shared) {
        if (!block()) {
            bterr("BTDELR",QBUSY,0);
            goto fin;
        }
    }

    *ok = FALSE;
    /* save current root */
    svblk = btact->cntxt->super.scroot;
    if (btact->cntxt->super.scroot >= 0) {
        bsetbs(btact->cntxt->super.scroot,0);
    }
    /* make super current and find named root */
    btact->cntxt->super.scroot = ZSUPER;
    bsetbs(btact->cntxt->super.scroot,1);
    ierr = bfndky(b,root,&blk,ok);
    if (ierr != 0 || *ok == FALSE) goto fin;
    if (blk == svblk) {
        /* can't delete current root */
        bsetbs(btact->cntxt->super.scroot,0);
        btact->cntxt->super.scroot = svblk;
        bsetbs(btact->cntxt->super.scroot,1);
        bterr("BTDELR",QDELCR,0);
        *ok = FALSE;
        goto fin;
    }
    /* delete key from superroot */
    if (*ok) ierr = bdelky(b,root,ok);
    btact->cntxt->super.smod++;     /* super root updated */
    if (ierr != 0) goto fin;
    bclrlf();
    if (*ok) {
        /* unbusy super root */
        bsetbs(btact->cntxt->super.scroot,0);
        /* make name root current and free all blocks */
        btact->cntxt->super.scroot = blk;
        bsetbs(btact->cntxt->super.scroot,1);
        /* free any data blocks */
        blk = bgtinf(btact->cntxt->super.scroot,ZNXBLK);
        while (blk != ZNULL) {
            thisblk = blk;
            blk = bgtinf(blk,ZNXBLK);
            bmkfre(thisblk);
        }
        /* free index blocks */
        blk = ZNULL;
        bnxtbk(&blk);
        while (blk != btact->cntxt->super.scroot) {
            bmkfre(blk);
            bnxtbk(&blk);
        }
        bsetbs(btact->cntxt->super.scroot,0);
        bmkfre(btact->cntxt->super.scroot);
    }
    /* return to saved root  */
    btact->cntxt->super.scroot = svblk;
    if (btact->cntxt->super.scroot >= 0) bsetbs(btact->cntxt->super.scroot,1);
    *ok = TRUE;
  fin:
    if (btact->shared) bulock();
    return(btgerr());
}
