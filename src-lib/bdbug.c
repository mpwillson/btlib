/*
  bdbug: write out internal info

  int bdbug(BTA * b,char *cmd,int blkno)

    b      pointer to BT context
    cmd    debug command
    blkno  block number

*/

#include <stdio.h>
#include <string.h>

#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

int bdbug(BTA * b,char *cmd,int blkno)
{
    int i,j,ioerr;
    int tblks,tnkeys;
    DATBLK *d;

    bterr("",0,0);
    if ((ioerr=bvalap("BDBUG",b)) != 0) return(ioerr);

    if (btact->shared) {
        if (!block()) {
            bterr("BDBUG",QBUSY,0);
            goto fin;
        }
    }
    
    btact = b;          /* set context pointer */
    
    if (btact->idxunt == NULL) {
        bterr("BDBUG",QNOBTF,0);
        goto fin;
    }
    if (strcmp(cmd,"super") == 0) {
        printf(
            "  Number of blocks: %10d\n"
            "  # free blocks:    %10d\n"
            "  First free:       %10d\n"
            "  Current root blk: %10d\n"
            "  Current root nm:  %s\n"
            "  Block overhead:   %10d\n"
            "  Block size:       %10d\n"
            "  Keys per block:   %10d\n",
            btact->cntxt->super.sblkmx,btact->cntxt->super.snfree,
            btact->cntxt->super.sfreep,btact->cntxt->super.scroot,
            btact->cntxt->super.scclas,ZPAD,ZBLKSZ,ZMXKEY);
    }
    else if (strcmp(cmd,"control") == 0) {
        fprintf(stdout,"  Index file: %s\n"
                "  Shared?:    %10d\n"
                "  Last key:   %s\n"
                "  Last blk:   %10d\n"
                "  Last pos:   %10d\n"
                "  LRU head:   %10d\n"
                "  LRU tail:   %10d\n",
                btact->idxfid,btact->shared,btact->cntxt->lf.lfkey,
                btact->cntxt->lf.lfblk,btact->cntxt->lf.lfpos,
                btact->cntxt->lru.lruh,btact->cntxt->lru.lrut);
        fprintf(stdout,"      Mblk       Blk      Busy    Writes     Lrunxt\n");
        for (i=0;i<ZMXBLK;i++)
            fprintf(stdout,"%10d%10d%10d%10d%10d\n",
                    i,((btact->cntrl)+i)->inmem,
                    ((btact->cntrl)+i)->busy,
                    ((btact->cntrl)+i)->writes,
                    ((btact->cntrl)+i)->lrunxt);
    }
    else if (strcmp(cmd,"stats") == 0) {
        fprintf(stdout,"  Logical reads:   %10d\n"
                "  Logical writes:  %10d\n"
                "  Physical reads:  %10d\n"
                "  Physical writes: %10d\n"
                "  Block splits:    %10d\n"
                "  Block joins:     %10d\n"
                "  Block balances:  %10d\n"
                "  Blocks obtained: %10d\n"
                "  Blocks freed:    %10d\n",
                btact->cntxt->stat.xlogrd,btact->cntxt->stat.xlogwr,
                btact->cntxt->stat.xphyrd,btact->cntxt->stat.xphywr,
                btact->cntxt->stat.xsplit,btact->cntxt->stat.xjoin,
                btact->cntxt->stat.xbal,btact->cntxt->stat.xgot,
                btact->cntxt->stat.xrel);
    }
    else if (strcmp(cmd,"space") == 0) {
        tblks = 0; tnkeys = 0; blkno = ZNULL;
        do {
            bnxtbk(&blkno);
            tblks++;
            ioerr = brdblk(blkno,&i);
            tnkeys += ((btact->memrec)+i)->infblk[ZNKEYS];
        } while (blkno != btact->cntxt->super.scroot);
        
        fprintf(stdout,
                " No. of blocks:    %10d\n"
                " Max. poss. keys:  %10d\n"
                " Actual keys:      %10d\n"
                " Occupancy (%c):    %10.2f\n",
                tblks, tblks*ZMXKEY,tnkeys,'%',
                (double) tnkeys/(tblks*ZMXKEY)*100.0);
    }
    else if (strcmp(cmd,"block") == 0) {
        ioerr = brdblk(blkno,&i);
        if (i >= 0) {
            fprintf(stdout,"  Block:%10d\n"
                    "  Misc: %10d\n"
                    "  Nxblk:%10d\n"
                    "  Nkeys:%10d\n"
                    "  Nblks:%10d\n",
                    ((btact->memrec)+i)->infblk[ZBTYPE],
                    ((btact->memrec)+i)->infblk[ZMISC],
                    ((btact->memrec)+i)->infblk[ZNXBLK],
                    ((btact->memrec)+i)->infblk[ZNKEYS],
                    ((btact->memrec)+i)->infblk[ZNBLKS]);
            if (((btact->memrec)+i)->infblk[ZBTYPE] == ZDATA) {
                d = (DATBLK *) (btact->memrec)+i;
                bxdump(d->data,ZBLKSZ-(ZINFSZ*ZBPW));
                goto fin;
            }
            fprintf(stdout,"  %32s %10s %10s %10s\n","Key","Val","Llink","Rlink");
            for (j=0;j<((btact->memrec)+i)->infblk[ZNKEYS];j++)
                fprintf(stdout,"  %32s %10d %10d %10d\n",
                        ((btact->memrec)+i)->keyblk[j],
                        ((btact->memrec)+i)->valblk[j],
                        ((btact->memrec)+i)->lnkblk[j],
                        ((btact->memrec)+i)->lnkblk[j+1]);
        }
        else {
            bterr("BDBUG",QRDBLK,blkno);
        }
    }
    else if (strcmp(cmd,"stack") == 0) {
        fprintf(stdout,"     Level  Contents\n");
        for (i=0;i<=btact->cntxt->stk.stkptr;i++)
            fprintf(stdout,"%10d%10d\n",i,btact->cntxt->stk.stk[i]);
    }
    else {
        bterr("BDBUG",QBADOP,0);
    }
  fin:
    if (btact->shared) bulock();
    return(btgerr());
}
