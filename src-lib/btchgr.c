/*
  btchgr: change B tree root

  int btchgr(BTA *b,char *root,int *ok)

      b       index file context pointer
      root    name of root to switch to
      ok      returned TRUE if switch ok
             
  Returns zero if no errors, error code otherwise

*/


#include <stdio.h>
#include <string.h>
#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

int btchgr(BTA *b,char *root,int *ok)
{
    int svblk,blk,ioerr;

    bterr("",0,0);

    if ((ioerr=bvalap("BTCHGR",b)) != 0) return(ioerr);

    btact = b;

    if (btact->shared) {
        if (!block()) {
            bterr("BTCHGR",QBUSY,0);
            goto fin;
        }
    }
    svblk = b->cntxt->super.scroot;
    /* unbusy current root */
    if (b->cntxt->super.scroot >= 0) bsetbs(b->cntxt->super.scroot,0);

    /* make current root the superroot (where root names are stored) */
    b->cntxt->super.scroot = ZSUPER;
    bsetbs(b->cntxt->super.scroot,1);
    ioerr = bfndky(b,root,&blk,ok);
    bsetbs(b->cntxt->super.scroot,0);
    if (ioerr != 0) goto fin;
    bclrlf();
    /* if ok, set up new root, else  restore old root */
    if (*ok) {
        b->cntxt->super.scroot = blk;
        strcpy(b->cntxt->super.scclas,root);
        bsetbs(b->cntxt->super.scroot,1);
    }
    else {
        b->cntxt->super.scroot = svblk;
        if (b->cntxt->super.scroot != ZNULL) bsetbs(b->cntxt->super.scroot,1);
    }
fin:
    if (btact->shared) bulock();
    return(btgerr());
}
