/*
 * $Id: btcrt.c,v 1.5 2004/09/26 13:07:39 mark Exp $
 *
 *
 * btcrt:  create B tree index file
 *
 * Parameters:
 *   fid    holds filename of index file to create
 *   nkeys  maximum number of keys required (0
 *          if random files can be grown dynamically)
 *   shared FALSE for exclusive index file access
 *          TRUE for shared access
 *               
 * Returns null if no errors, index context handle otherwise
 *
 * Copyright (C) 2003, 2004 Mark Willson.
 *
 * This file is part of the B Tree library.
 *
 * The B Tree library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The B Tree library  is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the B Tree library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <stdio.h>
#include <string.h>
#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

BTA *btcrt(char *fid, int nkeys,int shared)
{
    int idx,ioerr,nblks,i;
    
    bterr("",0,NULL);

    btact = bnewap(fid);
    if (btact == NULL) {
        bterr("BTCRT",QNOACT,NULL);
        goto fin;
    }
    if ((btact->idxunt = fopen(fid,"w+b")) == NULL) {
        bterr("BTCRT",QCRTIO,NULL);
        goto fin;
    }
    btact->shared = shared;
    strcpy(btact->idxfid,fid);

    /* initialise bt context areas */
    if (bacini(btact) != 0) 
        goto fin;
    /* set current root to ZNULL to indicate creation in progress */
    btact->cntxt->super.scroot = ZNULL;

    /* unconditional lock newly created file */
    if (!block()) {
        bterr("BTCRT",QBUSY,NULL);
        return(NULL);
    }


    btact->cntxt->super.sblkmx = 2;     /* include super and default roots */
    btact->cntxt->super.snfree = 0;     /* count of blocks in free list */
    btact->cntxt->super.sfreep = ZNULL; /* head of free list */
    btact->cntxt->super.smod = 1;

    /* set up super root */
    idx = bgtslt();
    ((btact->cntrl)+idx)->inmem = ZSUPER;
    bsetbk(ZSUPER,ZROOT,btact->cntxt->super.snfree,
        btact->cntxt->super.sfreep,2,
        btact->cntxt->super.sblkmx);
    
    strcpy(((btact->memrec)+idx)->keyblk[1],"$$super");
    ((btact->memrec)+idx)->valblk[1] = ZSUPER;
    strcpy(((btact->memrec)+idx)->keyblk[0],"$$default");
    ((btact->memrec)+idx)->valblk[0] = 1; /* record address of default root */
    for (i=0;i<=2;i++) ((btact->memrec)+idx)->lnkblk[i] = ZNULL;

    /* write it out */
    ioerr = bwrblk(ZSUPER);
    if (ioerr != 0) {
        bterr("BTCRT",QWRSUP,itostr(ZSUPER));
        goto fin;
    }

    /* set up default root */
    idx = bgtslt();
    ((btact->cntrl)+idx)->inmem = 1;
    btact->cntxt->super.scroot = 1;
    strcpy(btact->cntxt->super.scclas,"$$default");
    bsetbk(1,ZROOT,0,ZNULL,0,0);
    ioerr = bwrblk(1);
    if (ioerr != 0) {
        bterr("BTCRT",QWRBLK,itostr(1));
        goto fin;
    }
    ioerr = brdblk(1,&idx);
    if (ioerr != 0) {
        bterr("BTCRT",QRDBLK,itostr(1));
        goto fin;
    }
    bsetbs(1,1);
    /* initialise free list if required */
    if (nkeys != 0) {
        idx = bgtslt();
        nblks = nkeys/ZMXKEY;
        for (i=2;i<nblks;i++) {
            ((btact->cntrl)+idx)->inmem = i;
            bmkfre(i); 
            btact->cntxt->super.sblkmx++;
        }
    }
    if (shared) bulock();   /* allow access in shared mode */
    if (btgerr() != 0 )
        return(NULL);
    else 
        return(btact);
fin:
    /* error - free memory */
    if (shared) bulock();
    bacfre(btact);
    return(NULL);
}
