/*
 *  NAME
 *      kcp - copies B Tree index/data files
 *  
 *  SYNOPSIS
 *      kcp old_file new_file
 *  
 *  DESCRIPTION
 *      kcp will perform a logical copy of a B Tree index/data file to
 *      a new file.  This operation can be used to compact an existing
 *      B Tree index/data file, reclaiming deleted, but inaccessible,
 *      record segments.  All roots found within the old_file are copied.
 *
 *  NOTES
 *      kcp will only function successfully on B Tree files in which
 *      all keys have associated data records. It will not function
 *      where key values have been assigned arbitrary values. 
 *
 *  MODIFICATION HISTORY
 *  Mnemonic        Rel Date    Who
 *  KCP             1.0 020115  mpw
 *      Created.
 *  KCP             2.0 030722  mpw
 *      Operates on all roots in a B Tree file, not just $$default.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "btree.h"

#define SUPER "$$super"

void kalloc();
int copyroot();

char *prog;

int main(int argc, char *argv[])
{
    int found,
        i,
        ierr, ioerr,
        moreroots;
    BTA *in, *out;
    char current_root[ZKYLEN];
    char fname[20],msg[80];

    if (argc < 3) {
        fprintf(stderr,"%s: usage: %s old_file new_file\n",argv[0],argv[0]);
        exit(-1);
    }

    prog = argv[0];
    btinit();
    
    /* Open the original file */
    in = btopn(argv[1],0,FALSE);
    if (in == NULL) {
        fprintf(stderr,"%s: unable to open old file '%s'\n",prog,argv[1]);
        exit(-1);
    }
    /* Create the new file */
    out = btcrt(argv[2],0,FALSE);
    if (out == NULL) {
        fprintf(stderr,"%s: unable to open new file '%s'\n",prog,argv[2]);
        exit(-1);
    }
    ierr = btchgr(in,SUPER,&found);
    if (ierr != 0 || !found) goto fin;

    /* position at beginning of superroot */
    current_root[0] = '\0';
    ierr = bfndky(in,current_root,&i,&found);
    if (ierr != 0) goto fin;
    /* get first entry */
    ierr = bnxtky(in,current_root,&i,&found);
    if (ierr != 0 || !found) goto fin;
    moreroots = TRUE;
    /* cycle through the roots */
    while (moreroots) {
        if (strcmp(current_root,SUPER) != 0) {
            if ((ierr=copyroot(in,out,current_root)) != 0) goto fin;
            ierr = btchgr(in,SUPER,&found);
            if (ierr != 0) goto fin;
        }
        /* no context maintained when switching roots, so need to
           position afresh */
        ierr = bfndky(in,current_root,&i,&found);
        ierr = bnxtky(in,current_root,&i,&moreroots);
        if (ierr != 0) goto fin;
    }
  fin:
    if (ierr != 0) {
        btcerr(&ierr,&ioerr,fname,msg);
        fprintf(stderr,"%s: btree error (%d) [%s] - %s\n",
                argv[0],ierr,fname,msg);
    }
    ierr = btcls(in);
    if (ierr == 0) ierr = btcls(out);
    
    return(0);

}

int copyroot(BTA* in, BTA* out, char *rootname)
{
    static int bufsiz = 1024;
    int ierr, ok, found, i, rsiz;
    static char *buf = NULL;
    char key[ZKYLEN];

    if (buf == NULL) {
        kalloc(&buf,bufsiz);
    }

    ierr = btchgr(in,rootname,&ok);
    if (ierr != 0) return ierr;
    ierr = btcrtr(out,rootname,&ok);
    if (ierr != 0) return ierr;
    
    ierr = bfndky(in,"",&i,&found);
    if (ierr != 0) return ierr;
    
    found = TRUE;
    while (found) {
        ierr = btseln(in,key,buf,bufsiz,&rsiz,&found);
        if (ierr != 0) return ierr;
        if (!found) break;
        
        while (bufsiz == rsiz) {
            /* assume the record is bigger than the buffer,
             * repeat until we read ok, or run out of memory */
            free(buf);
            bufsiz += bufsiz;
            kalloc(&buf,bufsiz);
            ierr = btsel(in,key,buf,bufsiz,&rsiz,&found);
            if (ierr != 0) return ierr;
        }
        ierr = btins(out,key,buf,rsiz,&ok);
        if (ierr != 0) return ierr;
    }
    return 0;
}

void kalloc(char **buf,int bufsiz)
{
    *buf = (char *) malloc(bufsiz);
    /* fprintf(stderr,"..allocating %d bytes\n",bufsiz); */
    if (buf == NULL) {
        fprintf(stderr,"%s: cannot acquire enough memory (%d bytes)\n",
                prog,bufsiz);
        exit(-1);
    }
}
