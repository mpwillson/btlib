/*
  btcrtr: creates a new root in index file

  int btcrtr(BTA *b, char *root)

      b      pointer to BT context
      root   name of root to create

  Returns zero if no errors, error code otherwise
*/

#include <stdio.h>
#include <string.h>
#include "bc.h"
#include "btree.h"
#include "btree_int.h"

int btcrtr(BTA *b, char *root)
{
    int svblk,blk,status;

    bterr("",0,NULL);

    if ((status=bvalap("BTCRTR",b)) != 0) return(status);

    btact = b;

    if (btact->shared) {
        if (!block()) {
            bterr("BTCRTR",QBUSY,NULL);
            goto fin;
        }
    }

    svblk = btact->cntxt->super.scroot;
    /* unbusy current root */
    bsetbs(btact->cntxt->super.scroot,FALSE);
    /* make current root the superroot (where root names are stored) */
    btact->cntxt->super.scroot = ZSUPER;
    bsetbs(btact->cntxt->super.scroot,TRUE);
    status = binsky(b,root,ZNULL);
    btact->cntxt->super.smod++;     /* super root updated */
    bsetbs(btact->cntxt->super.scroot,FALSE);
    bclrlf();
    /* if ok, set up new root, else restore old root */
    if (status == 0) {
        /* get free block for new root */
        blk = bgtfre();
        if (blk == 0) goto fin;
        /* update root block number */
        status = bupdky(b,root,blk);
        if (status != 0 ) goto fin;
        bsetbk(blk,ZROOT,0,ZNULL,0,0);
        btact->cntxt->super.scroot = blk;
        strcpy(btact->cntxt->super.scclas,root);
        bsetbs(btact->cntxt->super.scroot,TRUE);
    }
    else {
        btact->cntxt->super.scroot = svblk;
        bsetbs(btact->cntxt->super.scroot,TRUE);
    }
fin:
    if (btact->shared) bulock();
    return(btgerr());
}
