/*
 * $Id: btchgr.c,v 1.4 2004/09/26 11:49:18 mark Exp $
 *
 *
 * btchgr: change B tree root
 *
 * Parameters:
 *    b       index file context pointer
 *    root    name of root to switch to
 *            
 * Returns zero if no errors, error code otherwise
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
 * along with Foobar; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
