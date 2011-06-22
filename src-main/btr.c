/*
 *  $Id: btr.c,v 1.9 2011-06-21 15:15:12 mark Exp $
 *  
 *  NAME
 *      btr - attempts to recover corrupt btree index file
 *  
 *  SYNOPSIS
 *      btr {-k|-d} [-n cnt] [-v] [-a] [-f] [--] old_file new_file
 *  
 *  DESCRIPTION
 *      btr will attempt the copy the contents of the btree inex file
 *      old_file into new_file, mediated by the following command
 *      arguments:
 *
 *      -k        copy keys only
 *      -d        copy keys and data
 *      -n cnt    accept up to cnt io errors before failing
 *      -a        allow duplicates in the new btree index file
 *      -v        be verbose (up to three levels of verbosity by -vv
 *                and -vvv)
 *      -f        overwrite new_file if it exists
 *
 *       btr will also attempt to copy index files created with
 *       previous versions of btree, but recovery is limited as
 *       necessary information for btree index reconstruction is not
 *       available in earlier versions of the index structure.  This
 *       mostly affects multi-rooted trees, where the roots will be
 *       lost and all keys copied into the $$default root.
 *
 *  NOTES
 *      Open corrupt btree file using btr version of btopn (if
 *      necessary), to bypass some consistency checks.
 *
 *      Try and read superroot.  If successful, store root names and root
 *      blocks.  Only the roots in the superroot are retained.
 *
 *      For each block, starting from 1, read it in.  If marked as
 *      ZROOT or ZINUSE, extract the keys and values directly from the
 *      in-memory array.  If -k specified, write key and value to new
 *      btree index file.  If -d specified, if value is a valid
 *      draddr, try and read data record.  Data record addresses are
 *      stored in a supporting bt index file, so that we can detect
 *      circular references.  If data record read OK, write key and
 *      record to new btree file.
 *
 *      In version 4 (and later) of the btree index, each ZINUSE block
 *      will contain the root block it belongs to.  This will allow us to
 *      partition keys by their roots, although if we cannot read the
 *      superroot correctly, the original root names will be lost.
 * 
 *  BUGS
 *      btr delves into the innards of a btree index file and should
 *      not be used as a typical example of use of the btree API.
 *  
 *  MODIFICATION HISTORY
 *  Mnemonic        Rel Date     Who
 *  BTR             1.0 20110401 mpw
 *    Created.
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

#define VERSION "$Id: btr.c,v 1.9 2011-06-21 15:15:12 mark Exp $"
#define KEYS    1
#define DATA    2

#define IOERROR -2
#define BAD_DRADDR -3
#define DR_READ_ERROR -4

/* version of btlib for which full recovery is possible, that is each
 * block records the root of the tree */
#define FULL_RECOVERY_VERSION 4

/* GLOBALS --------------------------------------------------------------*/

char *prog;
char *data_record;
int data_record_size = ZBLKSZ;
int limited_recovery = FALSE;

/* Recovery statistics */
struct {
    int   nioerrs;
    BTint keys;
    BTint records;
    BTint seg_addr_loops;
    BTint key_blocks_processed;
    BTint bad_draddrs;
    BTint dr_read_errors;
    BTint bad_blocks;
} stats;

/* Hold keys and values read from block */
struct bt_block_keys {
    int nkeys;
    char *keys[ZMXKEY];
    BTint vals[ZMXKEY];
};

typedef struct bt_block_keys BTKEYS;
BTKEYS* superroot_keys = NULL;

/* END GLOBALS ---------------------------------------------------------*/

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
    *buf = malloc(bufsiz);
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
        fprintf(stderr,"%s: superroot is not a root: possible file damage?\n",
                prog);
        /* superroot data cannot be trusted */
        btact->cntxt->super.snfree = 0;
        btact->cntxt->super.sfreep = 0;
        btact->cntxt->super.sblkmx = BTINT_MAX;
    }
    else {
        /* retain free list pointers et al */
        btact->cntxt->super.snfree = bgtinf(ZSUPER,ZMISC);
        btact->cntxt->super.sfreep = bgtinf(ZSUPER,ZNXBLK);
        btact->cntxt->super.sblkmx = bgtinf(ZSUPER,ZNBLKS);
    }
    
    if (bgtinf(ZSUPER,ZBTVER) < FULL_RECOVERY_VERSION) {
        limited_recovery = TRUE;
        fprintf(stderr,"%s: index file is version: " ZINTFMT ". "
                "Running in limited recovery mode.\n",
                prog,bgtinf(ZSUPER,ZBTVER));    
    }
    /* change to default root */
    status = btchgr(btact,"$$default");
    if (status != 0) {
        fprintf(stderr,"%s: no $$default root found.\n",prog);
    }
    return(btact);
fin:
    bacfre(btact);
    return(NULL);
}

int load_block(BTA* in, BTint blkno, int vlevel)
{
    int status, idx;
    
    btact = in;
    status = brdblk(blkno,&idx);
    if (status != 0) {
        if (feof(btact->idxunt)) {
            idx = EOF;
        }
        else {
            if (vlevel >= 3) {
                fprintf(stderr,"%s: I/O error: %s\n",prog,strerror(errno));
            }
            stats.nioerrs++;
            idx = IOERROR;
        }
    }
    return idx;
}

/* copy keys from block */
BTKEYS* get_keys(int idx, int nkeys,BTKEYS* k)
{
    int j;
    BTKEYS* keyblk;

    if (k == NULL) {
        keyblk = (BTKEYS *) malloc(sizeof(BTKEYS));
        if (keyblk == NULL) {
            fprintf(stderr,"%s: get_keys: unable to allocate memory.\n",
                    prog);
            return NULL;
        }
    }
    else {
        keyblk = k;
    }
    for (j=0;j<nkeys;j++) {
        keyblk->keys[j] = strdup(((btact->memrec)+idx)->keyblk[j]);
        keyblk->vals[j] = ((btact->memrec)+idx)->valblk[j];
    }
    keyblk->nkeys = nkeys;
    return keyblk;
}

/* load_root_names reads in the keys from the superroot.  Since we
 * can't trust the index, only the superroot block is processed.
 * We assume that there will be fewer roots than ZMXKEY.  If this
 * assumption is incorrect, then roots are named using the root block
 * number (for btree index files >= FULL_RECOVERY_VERSION). 
 */

int load_superroot_names(BTA* in,int vlevel)
{
    int i, idx, nkeys;
    
    idx = load_block(in,ZSUPER,vlevel);
    if (idx < 0) return idx;
    nkeys = bgtinf(ZSUPER,ZNKEYS);
    superroot_keys = get_keys(idx,nkeys,NULL);
    /* TBD: for some vlevel, print root names and blocks */
    if (vlevel >= 1) {
        printf("\nAttempting to recover the following root names:\n");
        printf("%-32s %s\n","RootName","BlockNum");
        for (i=0; i<nkeys; i++) {
            printf("%-32s " ZINTFMT "\n",superroot_keys->keys[i],
                   superroot_keys->vals[i]);
        }
        puts("");
    }
    return 0;
}

char* name_of_root(BTint blkno)
{
    int j;
    static char root_name[ZMXKEY+1];
    
    for (j=0;j<superroot_keys->nkeys;j++) {
        if (superroot_keys->vals[j] == blkno) {
            return superroot_keys->keys[j];
        }
    }
    return root_name;
}

int valid_draddr(BTint draddr)
{
    BTint dblk;
    int offset;

    cnvdraddr(draddr,&dblk,&offset);
    return (dblk > ZSUPER &&
            dblk < btact->cntxt->super.sblkmx &&
            bgtinf(dblk,ZBTYPE) == ZDATA &&
            offset >= 0 &&
            offset < ZBLKSZ);
}
 
int copy_data_record(BTA* in, BTA* out, BTA* da, char* key, BTint draddr,
                     int vlevel)
{
    int status,rsize;

    btact = in;
    if (!valid_draddr(draddr)) {
        if (vlevel >=3) {
            fprintf(stderr,"Invalid data address for key %s: 0x" ZXFMT "\n",
                    key,draddr);
        }
        return BAD_DRADDR;
    }
    
    /* Call brecsz with BTA*, which will cause it to record all
     * draddrs and error on a duplicate */
    rsize = brecsz(draddr,da);
    status = btgerr();
    if (status != 0) {
        print_bterror();
        return DR_READ_ERROR;
    }
    
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
        stats.records++;
        status = btins(out,key,data_record,rsize);
    }
    else {
        print_bterror();
        status = DR_READ_ERROR;
        if (vlevel >= 3)
            fprintf(stderr,"\n");
    }
    return status;
}

int copy_index(int mode, BTA *in, BTA *out, BTA *da, int vlevel, int ioerr_max,
               int allow_dups)
{
    int j,idx,status,block_type,nkeys;
    BTint blkno,root;
    BTKEYS* keyblk = NULL;
    char* current_root = "$$default";
    char* root_name = "*UNKNOWN*";

    status = load_superroot_names(in,vlevel);
    if (!limited_recovery && status < 0) return status;
    
    for (blkno=1;blkno<BTINT_MAX;blkno++) {
       idx = load_block(in,blkno,vlevel);
        if (idx < 0 || stats.nioerrs > ioerr_max) {
            return idx;
        }

        block_type = bgtinf(blkno,ZBTYPE);
        if (block_type == ZROOT || block_type == ZINUSE) {
            nkeys = bgtinf(blkno,ZNKEYS);
            /* TDB: perform various checks on block info */
            if (nkeys < 0 || nkeys > ZMXKEY) {
                if (vlevel >= 2) {
                    fprintf(stderr,"btr: block: " ZINTFMT
                            ", bad ZNKEYS value: %d\n",
                            blkno,nkeys);
                }
                continue;
            }
            stats.key_blocks_processed++;
             /* copy keys from block; re-use keyblk */
            keyblk = get_keys(idx,nkeys,keyblk);
            if (!limited_recovery) {
                /* check for multi-root index */
                root = bgtinf(blkno,ZNBLKS);
                root_name = name_of_root(root);
                if (strcmp(root_name,current_root) != 0) {
                    /* attempt to switch to root; create if it
                     * doesn't exist */
                    status = btchgr(out,root_name);
                    if (status == QNOKEY) {
                        status = btcrtr(out,root_name);
                    }
                    if (status != 0) {
                        print_bterror();
                        return status;
                    }
                    if (allow_dups) {
                        status = btdups(out,TRUE);
                        if (status != 0) {
                            print_bterror();
                            return status;
                        }
                    }
                    current_root = root_name;
                }   
            }
            if (vlevel >= 2) {
                fprintf(stderr,"Processing block: " ZINTFMT
                        ", keys: %8d [%s," ZINTFMT "]\n",blkno,nkeys,
                        current_root,root);
            }
           /* insert into new btree file */
            for (j=0;j<nkeys;j++) {
                if (mode == KEYS) {
                    stats.keys++;
                    status = binsky(out,keyblk->keys[j],keyblk->vals[j]);
                }
                else if (mode == DATA) {
                    status = copy_data_record(in,out,da,keyblk->keys[j],
                                              keyblk->vals[j],vlevel);

                    /* attempt to ignore (but count) errors on input
                       side */
                    if (status == QDLOOP) {
                        /* mostly likely problem on input side; let's
                           just copy the key */                        
                        stats.seg_addr_loops++;
                        status = -1;
                    }
                    if (status == BAD_DRADDR) stats.bad_draddrs++;
                    if (status == DR_READ_ERROR) stats.dr_read_errors++;
                    if (status < 0) {
                        /* copy_data_record can't access the data
                         * record; insert key only, if so.
                         */
                        status = binsky(out,keyblk->keys[j],keyblk->vals[j]);
                    }
                    else if (status == 0) {
                        stats.keys++;
                    }
                }
                else {
                    fprintf(stderr,"%s: unknown copy mode: %d\n",prog,mode);
                    return 0;
                }
                free(keyblk->keys[j]);
                if (status != 0) {
                    print_bterror();
                    return status;
                }
            }
        }
        else {
            if (vlevel >= 3 && block_type != ZFREE && block_type != ZDATA) {
                fprintf(stderr,"%s: ignoring block " ZINTFMT
                        " of unknown type 0x%x\n",prog,blkno,block_type);
                stats.bad_blocks++;
            }
        }
    }
    return status;
}

int main(int argc, char *argv[])
{
    int exit_status;
    int more_args = TRUE;
    BTA *in, *out, *da = NULL;
    char current_root[ZKYLEN],*s;
    int copy_mode = KEYS;
    int ioerror_max = 0;
    int vlevel = 0;
    int preserve = TRUE;
    int allow_dups = FALSE;
    int status;

    current_root[0] = '\0';
    s = strrchr(argv[0],'/');
    prog = (s==NULL)?argv[0]:(s+1);

    if (argc < 3) {
        fprintf(stderr,"%s: usage: %s [-k|-d] [-n cnt] [-v] [-a] [--] "
                "old_file new_file\n",
                prog,prog);
        return EXIT_FAILURE;
    }

    while (more_args && --argc > 0 && (*++argv)[0] == '-') {
        for (s=argv[0]+1; *s != '\0'; s++) {
            switch (*s) {
                case 'a':
                    allow_dups = TRUE;
                    break;
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

    memset((void *) &stats,0,sizeof(stats));

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
        if (allow_dups) {
            status = btdups(out,TRUE);
            if (status != 0) {
                print_bterror();
                return EXIT_FAILURE;
            }
        }   
        status = copy_index(copy_mode,in,out,da,vlevel,ioerror_max,
                            allow_dups);
        btcls(in);
        btcls(out);
        if (da != NULL) btcls(da);
        puts("\nBTRecovery Statistics:");
        printf("  %-26s " Z20DFMT "\n","Key Blocks Processed:",
               stats.key_blocks_processed);
        printf("  %-26s " Z20DFMT "\n","Keys Processed:",stats.keys);
        printf("  %-26s " Z20DFMT "\n","Data Records Processed:",stats.records);
        printf("  %-26s " Z20DFMT "\n","Data Record Read Errors:",
               stats.dr_read_errors);
        printf("  %-26s " Z20DFMT "\n","Record Address Loops:",
               stats.seg_addr_loops);
        printf("  %-26s " Z20DFMT "\n","Bad Blocks (invalid type):",
               stats.bad_blocks);
        if (vlevel > 0) {
            fprintf(stdout,"  %-26s %20d\n","I/O errors encountered:",
                    stats.nioerrs);
        }
    }
    
    return exit_status;
}
