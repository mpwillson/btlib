/*
  btchgr: change B tree root

  int btchgr(BTA *b,char *root)

      b       index file context pointer
      root    name of root to switch to
             
  Returns zero if no errors, error code otherwise

*/


#include <stdio.h>
#include <string.h>
#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

int btchgr(BTA *b,char *root)
{
    int svblk,blk,status;

    bterr("",0,NULL);

    if ((status=bvalap("BTCHGR",b)) != 0) return(status);

    btact = b;

    if (btact->shared) {
        if (!block()) {
            bterr("BTCHGR",QBUSY,NULL);
            goto fin;
        }
    }
    svblk = b->cntxt->super.scroot;
    /* unbusy current root */
    if (b->cntxt->super.scroot >= 0) bsetbs(b->cntxt->super.scroot,FALSE);

    /* make current root the superroot (where root names are stored) */
    b->cntxt->super.scroot = ZSUPER;
    bsetbs(b->cntxt->super.scroot,TRUE);
    status = bfndky(b,root,&blk);
    bsetbs(b->cntxt->super.scroot,FALSE);
    bclrlf();
    /* if ok, set up new root, else  restore old root */
    if (status == 0) {
        b->cntxt->super.scroot = blk;
        strcpy(b->cntxt->super.scclas,root);
        bsetbs(b->cntxt->super.scroot,TRUE);
    }
    else {
        b->cntxt->super.scroot = svblk;
        if (b->cntxt->super.scroot != ZNULL) {
            bsetbs(b->cntxt->super.scroot,TRUE);
        }
    }
fin:
    if (btact->shared) bulock();
    return(btgerr());
}
