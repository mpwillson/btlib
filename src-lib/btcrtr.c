/*
  btcrtr: creates a new root in index file

  int btcrtr(BTA *b, char *root,int *ok)

      b      pointer to BT context
      root   name of root to create
      ok     returned TRUE if root created ok

  Returns zero if no errors, error code otherwise
*/

#include <stdio.h>
#include <string.h>
#include "bc.h"
#include "btree.h"
#include "btree_int.h"

int btcrtr(BTA *b, char *root,int *ok)
{
    int svblk,blk,ioerr;

    bterr("",0,0);

    if ((ioerr=bvalap("BTCRTR",b)) != 0) return(ioerr);

    btact = b;

    if (btact->shared) {
        if (!block()) {
            bterr("BTCRTR",QBUSY,0);
            goto fin;
        }
    }

    svblk = btact->cntxt->super.scroot;
    /* unbusy current root */
    bsetbs(btact->cntxt->super.scroot,0);
    /* make current root the superroot (where root names are stored) */
    btact->cntxt->super.scroot = ZSUPER;
    bsetbs(btact->cntxt->super.scroot,1);
    ioerr = binsky(b,root,ZNULL,ok);
    btact->cntxt->super.smod++;     /* super root updated */
    bsetbs(btact->cntxt->super.scroot,0);
    bclrlf();
    /* if ok, set up new root, else restore old root */
    if (*ok) {
        /* get free block for new root */
        blk = bgtfre();
        if (blk == 0) goto fin;
        /* update root block number */
        ioerr = bupdky(b,root,blk,ok);
        if (ioerr != 0 ) goto fin;
        bsetbk(blk,ZROOT,0,ZNULL,0,0);
        btact->cntxt->super.scroot = blk;
        strcpy(btact->cntxt->super.scclas,root);
        bsetbs(btact->cntxt->super.scroot,1);
    }
    else {
        btact->cntxt->super.scroot = svblk;
        bsetbs(btact->cntxt->super.scroot,1);
    }
fin:
    if (btact->shared) bulock();
    return(btgerr());
}
