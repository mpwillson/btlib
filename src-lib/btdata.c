/*
 *  NAME
 *      btdata.c - handles data storage and retrieval from index files
 *
 *  DESCRIPTION
 *      These routines allow a btree to store and retrieve binary
 *      data, associated with a key.  This is a continuing
 *      development, and documentation will be enhanced in line with
 *      the code.
 *
 *  NOTES
 *      Data blocks will be returned to the free list only when they
 *      are completely empty.  No attempt is made to reclaim space
 *      when a data record is deleted, so a btree database is likely
 *      to grow over time.  To clean up a btree database, it must be
 *      copied to a new (and empty) btree database.
 *
 *      A data record address is held in a four byte int (as this is
 *      the maximum size of a data value stored with a key in the
 *      btree index), in the following format:
 *      
 *      31                                             0
 *      +----------------------------+-----------------+
 *      |   block number             | byte offset     |
 *      +----------------------------+-----------------+
 *
 *      With the default block size of 1024 bytes, the byte offset is 10 bits
 *      wide, while the block number field is 22 bits wide.
 *
 *      The field widths for the block number and offset are
 *      calculated from the block size when the btree library is
 *      initialised.
 *
 *      If a data record will not fit into a block, it is split into
 *      segments, sized to fit the data block.
 *      
 *      Each data segment is prefixed by ZDOVRH bytes of information
 *      (currently six).  These are used as follows:
 *
 *          Bytes 1 and 2: the size of the data segment in bytes
 *          (maximum size of a data segment is therefore 65536 bytes)
 *          
 *          Bytes 3-6: data record address of the next segment of this
 *          data record (0 if the last (or only) segment).
 *          
 *
 *  MODIFICATION HISTORY
 *  Mnemonic    Release Date    Who
 *  DT-ALPHA    1.0     010605  mpw
 *      Created.
 */
#include <string.h>

#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

/*
#undef DEBUG
#define DEBUG 1 
*/

/*------------------------------------------------------------------------
 * The btins routine will insert key and an associated data record
 * into a btree database.
 *
 *      BTA *b      btree context handle
 *      char *key   pointer to key string
 *      char *data  pointer to data
 *      int dsize   size of data record (in bytes)
 *      int *ok     return status of btins
 *                  (TRUE - btins ok, FALSE - btins failed)
 *
 * btins returns 0 if no errors encountered, error code otherwise.
 * Note that an btins can fail without an error (e.g. duplicate key)
 *------------------------------------------------------------------------
 */

int btins(BTA *b,char *key, char *data, int dsize,int *ok)
{
    int ierr, result;
    unsigned draddr = 0;

    bterr("",0,0);
    if ((result=bvalap("BTINS",b)) != 0) return(result);

    btact = b;  /* set context pointer */
    
    if (!dataok(btact)) {
        bterr("BTINS",QDAERR,0);
        goto fin;
    }
        
    if (btact->shared) {
        if (!block()) {
            bterr("BTINS",QBUSY,0);
            goto fin;
        }
    }

    /* insert data in btree if record has data */
    if (dsize > 0) {
        draddr = binsdt(data,dsize);
#if DEBUG > 0
        fprintf(stderr,"draddr = %x\n",draddr);
#endif
        if (draddr != ZNULL) {
            ierr = binsky(btact,key,draddr,ok);
            if (ierr != 0 || !*ok){
                /* can't insert new key, therefore must delete data */
                ierr = bdeldt(draddr);
            }
        }
    }
    else {
        bterr("BTINS",QEMPTY,0);
    }
    
fin:
    if (btact->shared) bulock();
    return(btgerr());
}

/*------------------------------------------------------------------------
 * The btupd routine will update the data record associated with a
 * key in the btree database.
 *
 *      BTA *b      btree context handle
 *      char *key   pointer to key string
 *      char *data  pointer to data
 *      int dsize   size of data record (in bytes)
 *      int *ok     return status of update
 *                  (TRUE - update ok, FALSE - update failed)
 *
 * btupd returns 0 if no errors encountered, error code otherwise.
 * Note that an btupd can fail without an error (e.g. non-existent key)
 *------------------------------------------------------------------------
 */

int btupd(BTA *b,char *key, char *data, int dsize,int *ok)
{
    int ierr, draddr, result;

    bterr("",0,0);
    if ((result=bvalap("BTUPD",b)) != 0) return(result);

    btact = b;      /* set context pointer */

    if (!dataok(btact)) {
        bterr("BTUPD",QDAERR,0);
        goto fin;
    }
    
    if (btact->shared) {
        if (!block()) {
            bterr("BTUPD",QBUSY,0);
            goto fin;
        }
    }

    /* find key in btree */
    ierr = bfndky(btact,key,&draddr,ok);
    if (ierr != 0 || !*ok) goto fin;

    /* update data in btree */
    ierr = bupddt(draddr,data,dsize);

fin:
    if (btact->shared) bulock();
    return(btgerr());
}

/*------------------------------------------------------------------------
 * The btsel routine will return the data record associated with a
 * key in the btree database.
 *
 *      BTA *b      btree context handle
 *      char *key   pointer to key string
 *      char *data  pointer to data recipient location
 *      int dsize   size of data record expected (in bytes)
 *      int *size   size of data record actually return (in bytes)
 *                  No more than dsize bytes will be returned.
 *      int *ok     return status of btsel
 *                  (TRUE - btsel ok, FALSE - btsel failed)
 *
 * btsel returns 0 if no errors encountered, error code otherwise.
 * Note that an btsel can fail without an error (e.g. non-existent key)
 *------------------------------------------------------------------------
 */

int btsel(BTA *b,char *key, char *data, int dsize,int *rsize,int *ok)
{
    int ierr, draddr, result;

    bterr("",0,0);
    if ((result=bvalap("BTSEL",b)) != 0) return(result);

    btact = b;      /* set context pointer */

    if (!dataok(btact)) {
        bterr("BTSEL",QDAERR,0);
        goto fin;
    }
    
    if (btact->shared) {
        if (!block()) {
            bterr("BTSEL",QBUSY,0);
            goto fin;
        }
    }

    /* find key in btree */
    ierr = bfndky(btact,key,&draddr,ok);
    if (ierr != 0 || !*ok) goto fin;

    /* TBD check for valid data pointer */

    /* retrieve data from btree */
    *rsize = bseldt(draddr,data,dsize);

fin:
    if (btact->shared) bulock();
    return(btgerr());
}

/*------------------------------------------------------------------------
 * The btdel routine will remove the data record associated with a
 * key in the btree database.
 *
 *      BTA *b      btree context handle
 *      char *key   pointer to key string
 *      int *ok     return status of delete
 *                  (TRUE - delete ok, FALSE - delete failed)
 *
 * btdel returns 0 if no errors encountered, error code otherwise.
 * Note that a delete can fail without an error (e.g. non-existent key)
 *------------------------------------------------------------------------
 */

int btdel(BTA *b,char *key,int *ok)
{
    int ierr, draddr, result;

    bterr("",0,0);
    if ((result=bvalap("BTDEL",b)) != 0) return(result);

    btact = b;      /* set context pointer */

    if (!dataok(btact)) {
        bterr("BTDEL",QDAERR,0);
        goto fin;
    }

    if (btact->shared) {
        if (!block()) {
            bterr("BTDEL",QBUSY,0);
            goto fin;
        }
    }

    /* find key in btree */
    ierr = bfndky(btact,key,&draddr,ok);
    if (ierr != 0 || !*ok) goto fin;

    /* delete data record first */
    ierr = bdeldt(draddr);
    if (ierr == 0) {
        ierr = bdelky(btact,key,ok);
    }

fin:
    if (btact->shared) bulock();
    return(btgerr());
}

/*------------------------------------------------------------------------
 * btseln will return the next key and data record following a
 * successful return from a previous call to the Btsel function.
 *------------------------------------------------------------------------
 */

int btseln(BTA *b,char *key, char *data, int dsize,int *rsize,int *ok)
{
    int ierr, draddr, result;

    bterr("",0,0);
    if ((result=bvalap("BTSELN",b)) != 0) return(result);

    btact = b;      /* set context pointer */

    if (!dataok(btact)) {
        bterr("BTSELN",QDAERR,0);
        goto fin;
    }

    if (b->shared) {
        if (!block()) {
            bterr("SELNEXT",QBUSY,0);
            goto fin;
        }
    }

    /* return next key in btree */
    ierr = bnxtky(btact,key,&draddr,ok);
    if (ierr != 0 || !*ok) goto fin;

    /* TBD check for valid data pointer */

    /* retrieve data from btree */
    *rsize = bseldt(draddr,data,dsize);

fin:
    if (btact->shared) bulock();
    return(btgerr());
}

/*------------------------------------------------------------------------
 * Btrecs will return the total record size of the data record
 * associated with a given key.  This value may be used by the client
 * application to acquire sufficient memory to retrieve the record.
 *
 *------------------------------------------------------------------------
 */

int btrecs(BTA *b, char *key, int *rsize, int *ok)
{
    int ierr, draddr, result;

    bterr("",0,0);
    if ((result=bvalap("BTRECS",b)) != 0) return(result);

    btact = b;      /* set context pointer */

    if (!dataok(btact)) {
        bterr("BTRECS",QDAERR,0);
        goto fin;
    }
    
    if (btact->shared) {
        if (!block()) {
            bterr("BTRECS",QBUSY,0);
            goto fin;
        }
    }
 
    ierr = bfndky(btact,key,&draddr,ok);
    if (ierr !=0 || !*ok) goto fin;

    *rsize = brecsz(draddr);

  fin:
    if (btact->shared) bulock();
    return(btgerr());
}


/*========================================================================
 * INTERNAL ROUTINES
 *========================================================================
 */

/*------------------------------------------------------------------------
 * bseldt will retrieve data record at address held in draddr.  A maximum
 * of dsize bytes will be retrieved.  If the actual number of bytes in
 * the data record is less than dsize bytes, the actual value is
 * returned, otherwise dsize is returned.
 *
 * Data is copied into the memory array pointed to by the data
 * character pointer.
 * 
 *------------------------------------------------------------------------
 */

int bseldt(int draddr, char *data, int dsize) 
{
    int dblk, ierr, idx,
        segsz = 0, cpsz = -1;
    int offset = 0;
    int totsz = 0;
    int sprem = dsize;
    
    DATBLK *d;

    while (sprem > 0 && draddr != 0) {
        /* unpick data pointer */
        cnvdraddr(draddr,&dblk,&offset);

        if (bgtinf(dblk,ZBTYPE) != ZDATA) {
            bterr("BSELDT",QNOTDA,0);
            totsz = -1;
            goto fin;
        }

        ierr = brdblk(dblk,&idx);
        d = (DATBLK *) (btact->memrec)+idx;
#if DEBUG > 0
        fprintf(stderr,"BSELDT: Using draddr x%x (%d,%d), found %x\n",
                draddr,dblk,offset,*(d->data+offset+ZDOVRH));
#endif

        segsz = rdsz(d->data+offset);
        draddr = rdint(d->data+offset+2);
#if DEBUG > 0
        fprintf(stderr,"BSELDT: Seg size: %d, next draddr: %x\n",segsz,draddr);
#endif
        cpsz = (segsz>sprem)?sprem:segsz;
#if DEBUG > 0
        fprintf(stderr,"BSELDT: copying %d bytes\n",cpsz);
#endif      
        memcpy(data,d->data+offset+ZDOVRH,cpsz);
        data += cpsz;
        sprem -= cpsz;
        totsz += cpsz;
    }
    
fin:
    return(totsz);
}

int bdeldt(unsigned draddr)
{
    int dblk, ierr;
    int size, offset;

    while (draddr != 0) {
        /* unpick data pointer */
        cnvdraddr(draddr,&dblk,&offset);
    
        if (bgtinf(dblk,ZBTYPE) != ZDATA) {
            bterr("BDELDT",QNOTDA,0);
            return(QNOTDA);
        }
        getseginfo(draddr,&size,&draddr);
    
        ierr = deldat(dblk,offset);
    }
    
    return(ierr);
}

/*------------------------------------------------------------------------
 * bupddt will update an existing data record with a new record.
 * Existing segments are reused.  If new segments are required, they
 * are attached to the existing segment chain. If the new data record
 * is smaller than the old, excess segments are deleted.
 * 
 * The function returns 0 for success, error code otherwise.
 *------------------------------------------------------------------------
 */

int bupddt(unsigned draddr, char *data, int dsize) 
{
    int dblk, ierr, idx, segsz, cpsz = ZNULL;
    int offset;
    int freesz;
    int remsz = dsize;
        
    DATBLK *d;

    while (draddr != 0 && remsz > 0) {
        /* unpick blk/offset pointer */
        cnvdraddr(draddr,&dblk,&offset);
#if DEBUG > 0
        fprintf(stderr,"Update processing blk: %d, offset: %d\n",dblk,offset);
#endif      
        if (bgtinf(dblk,ZBTYPE) != ZDATA) {
            bterr("BUPDDT",QNOTDA,0);
            goto fin;
        }
        getseginfo(draddr,&segsz,&draddr);

        ierr = brdblk(dblk,&idx);
        d = (DATBLK *) (btact->memrec)+idx;

        cpsz = ((segsz>remsz)?remsz:segsz);
#if DEBUG > 0
        fprintf(stderr,"Old seg: %d, new seg: %d\n",segsz,cpsz);
#endif
        memcpy(d->data+offset+ZDOVRH,data,cpsz);
        ((btact->cntrl)+idx)->writes++;
        if (cpsz == remsz) {
            /* last (or only) segment */
            wrsz(cpsz,d->data+offset);
            wrint((unsigned) 0,d->data+offset+2);
            /* update free space in block */
            freesz = bgtinf(dblk,ZMISC);
            freesz += (segsz-cpsz);
            bstinf(dblk,ZMISC,freesz);
        }
        remsz -= cpsz;
        data += cpsz;
    }
    if (draddr != 0) {
        /* new data record is smaller than original, need to free
         * remaining segments
         */
        bdeldt(draddr);
    }
    else if (remsz > 0) {
        /* new data record is larger, need new segments for rest of record
         */
        draddr = binsdt(data,remsz);
        /* insert returned data address into original last segment */
        ierr = brdblk(dblk,&idx);
        d = (DATBLK *) (btact->memrec)+idx;
        wrint(draddr,d->data+offset+2);
        ((btact->cntrl)+idx)->writes++;
    }
    
fin:
    return(btgerr());
}

/*------------------------------------------------------------------------
 * binsdt takes a data record, of length dsize, and copies it into 
 * one or more data blocks.  Data records are split into segments,
 * when a data record is too large to fit into one data block.
 *
 * Segments are stored in reverse order, to make it easier to
 * reconstitute the original record on retrieval.
 *
 * binsdt returns the data record address of the first segment, or
 * ZNULL if the record could not be stored.
 *------------------------------------------------------------------------
 */

int binsdt(char *data, int dsize)
{
    int offset, dblk, nblk;
    char *segptr = data+dsize;
    int remsize = dsize;
    int segaddr = 0;
    int freesz;

    /* ensure there is an active data block */
    dblk = bgtinf(btact->cntxt->super.scroot,ZNXBLK);
    if (dblk == ZNULL) {
        dblk = mkdblk();
        bstinf(btact->cntxt->super.scroot,ZNXBLK,dblk);
    }

    if (dblk == ZNULL) {
        bterr("BINSDT",QNOBLK,0);
        goto fin;
    }

    /* process the data record */
    while (remsize > 0) {
        /* free size is space left from first free byte onwards */
        freesz = (ZBLKSZ-(ZINFSZ*ZBPW))-bgtinf(dblk,ZNKEYS);
        if (freesz >= remsize+ZDOVRH) {
            /* segment fits in active block */
            segptr -= remsize;
            offset = insdat(dblk,segptr,remsize,segaddr);
            remsize = 0;
        }
        else if (freesz < (ZDOVRH+ZDSGMN)) {
            /* space below min seg size; need new block */
            nblk = mkdblk();
            if (nblk == ZNULL) {
                bterr("BINSDT",QNOBLK,0);
                goto fin;
            }
            /* maintain double-linked list of data blocks to allow
             * blocks to be returned to the free list, without
             * destroying the data block chain for this root.
             */
            bstinf(nblk,ZNXBLK,dblk);
            bstinf(dblk,ZNBLKS,nblk);
            
            dblk = nblk;
            /* new data list head */
            bstinf(btact->cntxt->super.scroot,ZNXBLK,dblk); 

        }
        else {
            /* determine beginning of segment and store */
            freesz -= ZDOVRH;
            segptr -= freesz;
            offset = insdat(dblk,segptr,freesz,segaddr);
            segaddr = mkdraddr(dblk,offset);
            remsize -= freesz;
        }
    }

    if (offset == ZNULL) goto fin;
    return(mkdraddr(dblk,offset));
    
  fin:
    return(ZNULL);
}

int deldat(int blk,int offset) 
{
    int ierr, freesz, size, idx;
    int pblk, nblk;
    DATBLK *d;

    ierr = brdblk(blk,&idx);
    if (ierr != 0) {
        return(ierr);
    }
    freesz = bgtinf(blk,ZMISC);
    d = (DATBLK *) (btact->memrec)+idx;
    size = rdsz(d->data+offset);
    freesz += (size+ZDOVRH);
#if DEBUG > 0
    fprintf(stderr,"Deleting segment: blk %d, offset: %d\n",blk,offset);
    fprintf(stderr,"seg size: %d, free space now = %d, free target = %d\n",
            size,freesz,ZBLKSZ-(ZINFSZ*ZBPW));
#endif  
    if (freesz == ZBLKSZ-(ZINFSZ*ZBPW)) {
        bstinf(blk,ZMISC,freesz);
        bstinf(blk,ZNKEYS,0);   /* reset offset pointer */          
        /* no data left in this block; can remove this block from the
         * active data block list */
        pblk = bgtinf(blk,ZNBLKS);
        nblk = bgtinf(blk,ZNXBLK);
        if (nblk != ZNULL) bstinf(nblk,ZNBLKS,pblk);
        if (pblk != ZNULL) bstinf(pblk,ZNXBLK,nblk);
        bmkfre(blk);
        if (blk == bgtinf(btact->cntxt->super.scroot,ZNXBLK)) {
            /* no active data block now */
            bstinf(btact->cntxt->super.scroot,ZNXBLK,ZNULL);
        }
    }
    else if (freesz < 0 || freesz > ZBLKSZ-(ZINFSZ*ZBPW)) {
        /* shouldn't have a negative count or greater than max free space*/
        bterr("BDELDT",QNEGSZ,freesz);
    }
    else {
        bstinf(blk,ZMISC,freesz);
    }
    return (0);
}

int insdat(int blk,char *data, int dsize, unsigned prevseg)
{
    int offset, ierr, idx, freesz;
    DATBLK *d;

    offset = bgtinf(blk,ZNKEYS);
    ierr = brdblk(blk,&idx);
    d = (DATBLK *) (btact->memrec)+idx;
    wrsz(dsize,d->data+offset);
    wrint(prevseg,d->data+offset+2);
#if DEBUG > 0
    fprintf(stderr,"writing segment at block %d, offset %d, of size %d\n",
            blk,offset,dsize);
#endif  
    memcpy(d->data+offset+ZDOVRH,data,dsize);
    ((btact->cntrl)+idx)->writes++;
    bstinf(blk,ZNKEYS,offset+ZDOVRH+dsize);
    freesz = bgtinf(blk,ZMISC);
    bstinf(blk,ZMISC,freesz-(ZDOVRH+dsize));
    return(offset);
}
/*-----------------------------------------------------------
 * return number of bytes occupied by data record pointed to by draddr
 * ----------------------------------------------------------
 */

int brecsz(unsigned draddr)
{
    int blk, offset, segsz, recsz;
    unsigned newdraddr;
    
    recsz = 0;
    while (draddr != 0) {
        cnvdraddr(draddr,&blk,&offset);
#if DEBUG > 0
        fprintf(stderr,"BRECSZ: Processing draddr: %0x, blk: %d, offset: %d\n",
                draddr,blk,offset);
#endif
        /* ensure we are pointing at a data block */
        if (bgtinf(blk,ZBTYPE) != ZDATA) {
            bterr("BRECSZ",QNOTDA,0);
            return(0);
        }
        getseginfo(draddr,&segsz,&newdraddr);
        
#if DEBUG > 0       
        fprintf(stderr,"\tSeg size: %d, next seg: %0x\n",segsz,draddr);
#endif      
        if (newdraddr == draddr) {
            /* next segment address should never refer to current
               segment */
            bterr("BRECSZ",QDLOOP,0);
            return(0);
        }
        draddr = newdraddr;
        recsz += segsz;
    }
    return(recsz);
}

/*-----------------------------------------------------------
 * create a new data block from free list (or thin air)
 * ----------------------------------------------------------
 */

int mkdblk(void)
{
    int blk;

    blk = bgtfre();
    if (blk != ZNULL ) {
        /* data block info set as follows:
           0 - block type
           1 - number of free bytes in block
           2 - pointer to next data block in data block chain
           3 - offset of first free byte within data area of block (0)
           4 - pointer to previous data block in block chain
        */
        bsetbk(blk,ZDATA,ZBLKSZ-(ZINFSZ*ZBPW),ZNULL,0,ZNULL);
        return(blk);
    }
    return(ZNULL);
}

int rdsz(char *a)
{
    short s;

    s = *a++ & 0xff;
    s |= (*a & 0xff) << 8;
    return ((int)s);
}

int rdint(char *a)
{
    int i;

    i = *a++ & 0xff;
    i |= (*a++ & 0xff) << 8;
    i |= (*a++ & 0xff) << 16;
    i |= (*a & 0xff) << 24;
    return(i);
}

void wrsz(int i, char *a)
{
    short s;

    s = i;
    *a++ = s & 0xff;
    *a = (s>>8) & 0xff;
}

void wrint(unsigned i, char *a)
{
    *a++ = i & 0xff;
    *a++ = (i>>8) & 0xff;
    *a++ = (i>>16) & 0xff;
    *a = (i>>24) & 0xff;
}

int offsetmask;                 /* used to mask out block number
                                 * from data address */
int offsetwidth;                /* number of bits needed to offer data
                                 * block segment offset */

/*-----------------------------------------------------------------------
 * Determine and save data block addressing, based on block size
 *---------------------------------------------------------------------*/

void setaddrsize(int blksz)
{
    int width = 0;
    offsetmask = blksz-1;
    while (blksz>>=1) width++;
    offsetwidth = width;
}

/*------------------------------------------------------------------------
 * Convert data block address into block number and offset
 *----------------------------------------------------------------------*/

void cnvdraddr(unsigned draddr, int *dblk, int *offset)
{
    *dblk = draddr >> offsetwidth;
    *offset = draddr & offsetmask;
}

/*------------------------------------------------------------------------
 * Convert data block number and data offset to draddr value
 *----------------------------------------------------------------------*/

unsigned mkdraddr(int dblk, int offset)
{
    return(dblk<<offsetwidth | offset);
}

/*------------------------------------------------------------------------
 * Return size and next segment address of data record segment
 * indicated by draddr
 *----------------------------------------------------------------------*/

int getseginfo(unsigned draddr, int *size, unsigned *nextseg) 
{
    int blk, offset;
    int ierr,  idx;
    DATBLK *d;

    cnvdraddr(draddr,&blk,&offset);
    
    ierr = brdblk(blk,&idx);
    if (ierr != 0) {
        return(ierr);
    }

    d = (DATBLK *) (btact->memrec)+idx;
    *size = rdsz(d->data+offset);
    *nextseg = rdint(d->data+offset+2);
    return(0);
}

/* -----------------------------------------------------------------------
 * Validate if the current root permits storage of data records
 * -----------------------------------------------------------------------
 */
int dataok(BTA* b)
{
    if (b->cntxt->super.scroot == ZSUPER)
        return FALSE;
    else
        return TRUE;
}




