/*
  btopn: opens existing B tree index

  BTA *btopn(char *fid, int mode, int shared)

    fid     name of file to open
    mode    if ZERO, index file can be updated
    shared  set TRUE is index file is to be shared

  Returns zero if no errors, error code otherwise
*/

#include <stdio.h>
#include <string.h>
#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

BTA *btopn(char *fid, int mode, int shared)
{
    int ioerr;
    int ok;

    bterr("",0,0);

    btact = bnewap(fid);
    if (btact == NULL) {
        bterr("BTOPN",QNOACT,0);
        return(NULL);
    }
    if ((btact->idxunt = fopen(fid,"r+b")) == NULL) {
        bterr("BTOPN",QNOOPN,0);
        return (NULL);
    }
    strcpy(btact->idxfid,fid);
    if (bacini(btact) != 0) {
        fclose(btact->idxunt);
        return(NULL);
    }
    
    btact->shared = shared;
    btact->cntxt->super.smod = 0;
    btact->cntxt->super.scroot = 0;

    /* always lock file; shared will unlock at routine exit */
    if (!block()) {
        bterr("BTOPN",QBUSY,0);
        goto fin;
    }
    /* read in super root */
    if (brdsup() != 0) goto fin;

    /* change to default root */
    ioerr = btchgr(btact,"$$default",&ok);
    if (ioerr != 0) goto fin;
    if (ok)
        btact->cntxt->super.smode = mode;
    else {
        bterr("BTOPN",QNODEF,0);
        goto fin;
    }
    if (btgerr() != 0) 
        goto fin;
    if (shared) bulock();
    return(btact);
fin:
    if (shared) bulock();
    bacfre(btact);
    return(NULL);
}



