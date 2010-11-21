/*
 * $Id: butil.c,v 1.6 2010-05-26 12:39:16 mark Exp $
 *
 *  butil:  utility routines for the B Tree library
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

#include "btree_int.h"

static char buf[80];

/* Return BTint as string */
char* itostr(BTint v)
{
    sprintf(buf,ZINTFMT,v);
    return buf;
}

/* Checks structure of index file, from current root.  Returns number
 * of keys found. */
BTint btkeys(BTA* b,int stats)
{
    BTint tblks = 0,
        tnkeys = 0,
        empty_blk = ZNULL,
        blkno = ZNULL;
    BTint val,link1,link2;
    int ioerr,idx,result,loc = 0;
    int blk_depth[STKMAX+1];
    int leaf_depth = -1,unbalanced = FALSE;
    char key[ZKYLEN];

    
    bterr("",0,NULL);
    if ((ioerr=bvalap("BTKEYS",b)) != 0) return(ioerr);

    if (btact->shared) {
        if (!block()) {
            bterr("BTKEYS",QBUSY,NULL);
            goto fin;
        }
    }
    
    btact = b;          /* set context pointer */
    
    if (btact->idxunt == NULL) {
        bterr("BTKEYS",QNOBTF,NULL);
        goto fin;
    }

    for (idx=0;idx<=STKMAX;idx++) blk_depth[idx] = 0;
    
    tblks = 0; tnkeys = 0; blkno = ZNULL;
    do {
        int depth,nkeys;
        bnxtbk(&blkno);
        depth =  btstk_depth()/2;
        tblks++;
        blk_depth[depth]++;
        ioerr = brdblk(blkno,&idx);
        nkeys = bgtinf(blkno,ZNKEYS);
        if (nkeys == 0 && empty_blk == ZNULL) {
            if (bgtinf(blkno,ZBTYPE) != ZROOT) empty_blk = blkno;
            continue;
        }
        bsrhbk(blkno,key,&loc,&val,&link1,&link2,&result);
        if (link1 == ZNULL) {
            if (leaf_depth < 0) {
                leaf_depth = depth;
            }
            else if (leaf_depth != depth) {
                unbalanced = TRUE;
                if (depth > leaf_depth) leaf_depth = depth;
            }
        }
        tnkeys += nkeys;
    } while (blkno != btact->cntxt->super.scroot);

    if (stats) {
        printf("Block stats:\n");
        if (empty_blk != ZNULL) {
            printf("At least one non-root block has no keys.  "
                   "First encountered was " ZINTFMT "\n",empty_blk);
        }
        printf("Index balanced: %s\n",(unbalanced)?"NO":"YES");
        printf("Max leaf depth: %d\n",leaf_depth);
        printf("Depth   Number\n");
        for (idx=0;idx<=leaf_depth;idx++) {
            printf("%5d %8d\n",idx,blk_depth[idx]);
        }       
    }
    
  fin:
    return tnkeys;
}
