/*
 * $id$
 *
 *
  bsrhbk: searches block for key

  int bsrhbk(int blk,char *key,int *loc,int *val,int *link1,int *link2,
           int *result)

       blk    - block to search
       key    - key (possibly returned)
       loc    - location within block
       val    - value of key (returned)
       link1  - left link (returned)
       link2  - right link (returned)
       
    if loc non-ZERO, returns key, val and links at loc
    result is set to 0
    if key non-blank, searches block for key, returns loc
    val and links
    if key is found, result is 0
    if key is larger than any in block, result is 1
    if key smaller than any in the block, result is -1
            
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

#include <string.h>

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int bsrhbk(int blk,char *key,int *loc,int *val,int *link1,int *link2,
           int *result)
{

    int idx,lo,hi,md,nkeys,quit,ioerr;

    ioerr = brdblk(blk,&idx);
    if (idx < 0) {
        bterr("BSRHBK",QRDBLK,itostr(blk));
        goto fin;
    }
    nkeys = ((btact->memrec)+idx)->infblk[ZNKEYS];
    if (*loc >= 0) {
        if (*loc >= nkeys) {
            /* requested loc not in block range */
            *result = -1;
            goto fin;
        }
        else {
            *result = 0;
            strcpy(key,((btact->memrec)+idx)->keyblk[*loc]);
        }
    }
    else {
        /* binary search */
        *loc = -1; /* init location at impossible position */
        *result = -1;
        quit = FALSE;
        lo = 0;
        hi = nkeys-1;
        md = -1;
        while (lo <= hi && !quit) {
            quit = (lo == hi);
            md = (lo+hi)/2;
            *result = strcmp(key,((btact->memrec)+idx)->keyblk[md]);
#if DEBUG >= 2
            printf("key: \"%s\" vs \"%s\"\n",key,
                   ((btact->memrec)+idx)->keyblk[md]);
            printf("lo: %d, hi: %d, md: %d\n",lo,hi,md);
            printf("result: %d, quit: %d\n",*result,quit);
#endif
            if (*result > 0) {
                lo = md+1;
            }
            else  if (*result < 0) {
                hi = md-1;
            }
            else {
                break;
            }
        }
        *loc = md;
    }

    if (*loc < 0) {
        *val = 0;
        *link1 = ZNULL;
        *link2 = ZNULL;
    }
    else {
        *val = ((btact->memrec)+idx)->valblk[*loc];
        *link1 = ((btact->memrec)+idx)->lnkblk[*loc];
        *link2 = ((btact->memrec)+idx)->lnkblk[*loc+1];
#if DEBUG >=2
        fprintf(stderr,"BSRHBK: val: %d, link1: %d, link2: %d\n",*val,
                *link1,*link2);
#endif      
    }
fin:
    return(0);
}
