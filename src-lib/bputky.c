/*
    bputky: inserts key, value and links into block

    int bputky(int blk,char *key,int val,int link1,int link2)

        blk    block for which insertion is required
        key    key to insert
        val    value of key to insert
        link1  left link pointer
        link2  right link pointer

*/

#include <stdio.h>
#include <string.h>

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int bputky(int blk,char *key,int val,int link1,int link2)
{
    int i,idx,ioerr;
    char lkey[ZKYLEN];

#if DEBUG >= 1
    fprintf(stderr,
            "bputky: blk = %d, key = %s, val = %d, link1 = %d, link2 = %d\n",
            blk,key,val,link1,link2);
#endif
    ioerr = brdblk(blk,&idx);
    if (idx < 0)
        bterr("BPUTKY",QRDBLK,ioerr);
    else {
        /* get local copy of key, truncated if necessary */
        strncpy(lkey,key,ZKYLEN);
        lkey[ZKYLEN-1] = '\0';
        if (((btact->memrec)+idx)->infblk[ZNKEYS] == ZMXKEY) {
            bterr("BPUTKY",QBLKFL,blk);
            goto fin;
        }
        if (((btact->memrec)+idx)->infblk[ZNKEYS] == 0) {
            /* block empty */
            strcpy(((btact->memrec)+idx)->keyblk[0],lkey);
            ((btact->memrec)+idx)->valblk[0] = val;
            ((btact->memrec)+idx)->lnkblk[0] = link1;
            ((btact->memrec)+idx)->lnkblk[1] = link2;
        }
        else {  
            for (i=((btact->memrec)+idx)->infblk[ZNKEYS];i>0;i--) {
                if (strcmp(key,((btact->memrec)+idx)->keyblk[i-1]) < 0) {
                    /* move info to make room */
                    strcpy(((btact->memrec)+idx)->keyblk[i],
                        ((btact->memrec)+idx)->keyblk[i-1]);
                    ((btact->memrec)+idx)->valblk[i] = 
                        ((btact->memrec)+idx)->valblk[i-1];
                    ((btact->memrec)+idx)->lnkblk[i+1] = 
                        ((btact->memrec)+idx)->lnkblk[i];
                }
                else break;
            }
            /* move left link if inserting in first position */
            if (i == 0) ((btact->memrec)+idx)->lnkblk[1] = 
                ((btact->memrec)+idx)->lnkblk[0];
            strcpy(((btact->memrec)+idx)->keyblk[i],lkey);
            ((btact->memrec)+idx)->valblk[i] = val;
            if (link1 == ZNULL && link2 == ZNULL) {
                /* inserting a leaf key */
                ((btact->memrec)+idx)->lnkblk[i] = ZNULL;
                ((btact->memrec)+idx)->lnkblk[i+1] = ZNULL;
            }
            else {
                /* if inserting in first pos, then use llink */
                if (i == 0) ((btact->memrec)+idx)->lnkblk[0] = link1;
                /* rlink is inserted if ZERO or positive */
                if (link2 >= 0) ((btact->memrec)+idx)->lnkblk[i+1] = link2;
            }
        }
    }
    ((btact->memrec)+idx)->infblk[ZNKEYS]++;
    ((btact->cntrl)+idx)->writes++;
fin:
    return(0);
}
