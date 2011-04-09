/*
 *  $Id: btr.c,v 1.1 2011-03-17 19:06:23 mark Exp $
 *  
 *  NAME
 *      btr - attempts to recover corrupt btree index file
 *  
 *  SYNOPSIS
 *      btr {-k|-d} [-n cnt] [-v] [--] old_file new_file
 *  
 *  DESCRIPTION
  *
 *  NOTES
 *
 *  BUGS
 *     btr delves into the innards of a btree index file and should
 *     not be used as a typical example of use of the btree API.
 *  
 *  MODIFICATION HISTORY
 *  Mnemonic        Rel Date    Who
 *      
 * Copyright (C) 2011 Mark Willson.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "btree.h"
#include "btree_int.h"

/*
 * Open corrupt btree file using btr version of btopn, to bypass any
 * consistency checks.
 *
 * Try and read superroot.  If successful, store root names and root
 * blocks.
 *
 * For each block, starting from 1, read it in.  If marked as ZROOT or
 * ZINSUE, extract the keys and values directly from the in-memory
 * array.  If -k specified, write key and value to new btree index
 * file.  If -d specified, assume value is a valid draddr, and try and
 * read data record.  This might require special code to remember each
 * draddr so that we can detect circular references.  If data record
 * read OK, write key and record to new btree file.
 *
 * In later versions of the btree, each ZINUSE block will contain the
 * root block it belongs to.  This will allow us to partition keys by
 * their roots, although if we cannot read the superroot correctly,
 * the original root names will be lost.
 * 
 * Write errors to stderr.  Write stats on keys and/or data recovered.
 */

#define VERSION "$Id: btr.c,v 1.1 2011-03-17 19:06:23 mark Exp $"
#define KEYS    1
#define DATA    2

char *prog;
char *data_record;
int data_record_size = ZBLKSZ;

int file_exists(char *pathname)
{
    FILE* f;

    f = fopen(pathname,"r");
    if (f == NULL) {
        return FALSE;
    }
    else {
        fclose(f);
    }
    return TRUE;
}


void print_bterror(void)
{
    int errorcode, ioerr;
    char fname[ZRNAMESZ],msg[ZMSGSZ];

    btcerr(&errorcode,&ioerr,fname,msg);
    fprintf(stderr,"%s: btree error (%d) [%s] - %s\n",
            prog,errorcode,fname,msg);
}

void kalloc(char **buf,int bufsiz)
{
    *buf = (char *) malloc(bufsiz);
    /* fprintf(stderr,"..allocating %d bytes\n",bufsiz); */
    if (buf == NULL) {
        fprintf(stderr,"%s: cannot acquire enough memory (%d bytes)\n",
                prog,bufsiz);
        exit(EXIT_FAILURE);
    }
}

/* Open btree index file in recovery mode (i.e. limited checking) */
BTA *btropn(char *fid,int vlevel)
{
    int idx,ioerr,status;

    bterr("",0,NULL);

    btact = bnewap(fid);
    if (btact == NULL) {
        bterr("BTROPN",QNOACT,NULL);
        return NULL;
    }
    if ((btact->idxunt = fopen(fid,"r+b")) == NULL) {
        bterr("BTROPN",QOPNIO,fid);
        return NULL;
    }
    strcpy(btact->idxfid,fid);
    if (bacini(btact) != 0) {
        fclose(btact->idxunt);
        goto fin;
    }
    
    btact->shared = FALSE;
    btact->cntxt->super.smod = 0;
    btact->cntxt->super.scroot = 0;

    /* always lock file */
    if (!block()) {
        bterr("BTROPN",QBUSY,NULL);
        goto fin;
    }
    /* read in super root */
    ioerr = brdblk(ZSUPER,&idx);
    if (ioerr != 0) {
        bterr("BRDSUP",QRDSUP,(ioerr<0)?"(EOF?)":"");
        goto fin;
    } 
    if (bgtinf(ZSUPER,ZBTYPE) != ZROOT) {
        fprintf(stderr,"%s: superroot is not a root: possible file damage.\n",
                prog);
    }
    /* TBD:  Set absolute value for version below which limited
       recovery mode is invoked.  Also need to pass back a variable
       to indicate limited recovery mode is in effect. */
    if (bgtinf(ZSUPER,ZBTVER) != ZVERS) {
        fprintf(stderr,"%s: index file is version: %d. "
                "Running in limited recovery mode.\n",
                prog,bgtinf(ZSUPER,ZBTVER));    
    }
    
    /* retain free list pointers et al */
    btact->cntxt->super.snfree = bgtinf(ZSUPER,ZMISC);
    btact->cntxt->super.sfreep = bgtinf(ZSUPER,ZNXBLK);
    btact->cntxt->super.sblkmx = bgtinf(ZSUPER,ZNBLKS);
    /* change to default root */
    status = btchgr(btact,"$$default");
    if (status != 0) {
        fprintf(stderr,"%s: no $$default root found.\n",prog);
    }

    if (btgerr() != 0) 
        goto fin;
    return(btact);
fin:
    bacfre(btact);
    return(NULL);
}

int copy_data_record(BTA* in, BTA* out, BTA* da, char* key, BTint draddr,
                     int vlevel)
{
    int status,rsize;

    /* TDB validate draddr */

    btact = in;
    rsize = brecsz(draddr,da);
    status = btgerr();
    if (status != 0) print_bterror();
    if (rsize > data_record_size) {
        free(data_record);
        data_record_size = rsize;
        kalloc(&data_record,data_record_size);
    }
    if (vlevel >= 3) {
        fprintf(stderr,"Reading data record for key: %s; draddr: 0x" ZXFMT " ",
                key,draddr);
    }
    rsize = bseldt(draddr,data_record,rsize);
    status = btgerr();
    if (status == 0) {
        if (vlevel >= 3) {
            fprintf(stderr," (%d bytes read)\n",rsize);
        }
        status = btins(out,key,data_record,rsize);
    }
    else {
        if (vlevel >= 3)
            fprintf(stderr,"\n");
    }
    return status;
}

int copy_index(int mode, BTA *in, BTA *out, BTA *da, int vlevel, int ioerr_max)
{
    int nioerrs = 0,j,idx,status,block_type,nkeys;
    BTint blkno;
    char *keys[ZMXKEY];
    BTint vals[ZMXKEY];
    
    for (blkno=1;blkno<BTINT_MAX;blkno++) {
        btact = in;
        status = brdblk(blkno,&idx);
        if (status != 0) {
            if (feof(btact->idxunt)) {
                return nioerrs;
            }
            else {
                if (vlevel >= 3) {
                    fprintf(stderr,"%s: I/O error: %s\n",prog,strerror(errno));
                }
                nioerrs++;
                if (nioerrs >= ioerr_max) return nioerrs;
                continue;
            }
        }
        block_type = bgtinf(blkno,ZBTYPE);
        if (block_type == ZROOT || block_type == ZINUSE) {
            nkeys = bgtinf(blkno,ZNKEYS);
            /* TDB perform various checks on block info */
            if (nkeys < 0 || nkeys >= ZMXKEY) {
                if (vlevel >= 2) {
                    fprintf(stderr,"btr: block: %d, bad ZNKEYS value: %d\n",
                            blkno,nkeys);
                }
                continue;
            }
            if (vlevel >=3) {
                fprintf(stderr,"Processing block: %d, keys: %d\n",blkno,nkeys);
            }
            /* copy keys from block */
            for (j=0;j<nkeys;j++) {
                keys[j] = strdup(((btact->memrec)+idx)->keyblk[j]);
                vals[j] = ((btact->memrec)+idx)->valblk[j];
            }
            /* insert into new btree file */
            for (j=0;j<nkeys;j++) {
                if (mode == KEYS) {
                    status = binsky(out,keys[j],vals[j]);
                }
                else if (mode == DATA) {
                    status = copy_data_record(in,out,da,keys[j],vals[j],vlevel);
                }
                else {
                    fprintf(stderr,"%s: unknown copy mode: %d\n",prog,mode);
                    return 0;
                }
                free(keys[j]);
                if (status != 0) {
                    print_bterror();
                    return nioerrs;
                }
            }
        }   
    }
    return nioerrs;
}

int main(int argc, char *argv[])
{
    int exit_status;
    int more_args = TRUE;
    BTA *in, *out, *da = NULL;
    char current_root[ZKYLEN],*s;
    int copy_mode = KEYS;
    int nioerrs, ioerror_max = 0;
    int vlevel = 0;
    int preserve = TRUE;

    current_root[0] = '\0';
    s = strrchr(argv[0],'/');
    prog = (s==NULL)?argv[0]:(s+1);

    if (argc < 3) {
        fprintf(stderr,"%s: usage: %s {-k|-d} [-n cnt] [-v] [--] "
                "old_file new_file\n",
                prog,prog);
        return EXIT_FAILURE;
    }

    while (more_args && --argc > 0 && (*++argv)[0] == '-') {
        for (s=argv[0]+1; *s != '\0'; s++) {
            switch (*s) {
                case 'd':
                    copy_mode = DATA;
                    break;
                case 'f':
                    preserve = FALSE;
                    break;
                case 'k':
                    copy_mode = KEYS;
                    break;
                case 'n':
                    ioerror_max = atoi(*++argv);
                    break;
                case 'v':
                    vlevel++;
                    break;
                case '-' :
                    more_args = FALSE;
                    ++argv;
                    break;
                 default:
                     fprintf(stderr,"%s: unknown switch: '%c'\n",prog,*s);
                     exit(EXIT_FAILURE);     
            }
        }
    }

    if (vlevel > 0) {
        fprintf(stderr,"BTree Recovery: %s\n",VERSION);
    }
        
    if (btinit() != 0) {
        print_bterror();
        return EXIT_FAILURE;
    }

    if (copy_mode == DATA) {
        /* create index for remembering disk addresses */
        da = btcrt(".da.idx",0,FALSE);
        if (da == NULL) {
            print_bterror();
            return EXIT_FAILURE;
        }
        kalloc(&data_record,data_record_size);
    }
    
    exit_status = EXIT_SUCCESS;
    /* Open the original file */
    in = btopn(*argv,0,FALSE);
    if (in == NULL) {
        /* open file using fallback routine */
        in = btropn(*argv,vlevel);
        if (in == NULL) {
            print_bterror();
            return EXIT_FAILURE;
        }
    }
    /* Create the new file; need to check if existing index file roots
     * supports duplicates */
    argv++;
    if (preserve && file_exists(*argv)) {
        fprintf(stderr,"%s: target index file (%s) already exists.\n",
                prog,*argv);
        exit_status = EXIT_FAILURE;
    }
    else {
        out = btcrt(*argv,0,FALSE);
        if (out == NULL) {
            print_bterror();
            return EXIT_FAILURE;
        }
    
        nioerrs = copy_index(copy_mode,in,out,da,vlevel,ioerror_max);
        btcls(in);
        btcls(out);
        if (da != NULL) btcls(da);
        if (vlevel > 0) {
            fprintf(stderr,"%s: %d I/O errors encountered.\n",prog,nioerrs);
        }
    }
    
    return exit_status;
}
