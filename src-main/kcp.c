/*
 *  $Id: kcp.c,v 1.4 2004/09/26 11:49:18 mark Exp $
 *  
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
 *  KCP             2.1 040530  mpw
 *      Uses bt-2.0 API
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
    int i,
        status, ioerr;
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
    status = btchgr(in,SUPER);
    if (status != 0) goto fin;

    /* position at beginning of superroot */
    current_root[0] = '\0';
    status = bfndky(in,current_root,&i);
    /* if a root named with the null key exists, 0 is a valid return,
       otherwise we expect QNOKEY */
    if (!(status == QNOKEY || status == 0)) goto fin;
    /* get first entry */
    status = bnxtky(in,current_root,&i);
    if (status != 0) goto fin;
    /* cycle through the roots */
    while (status == 0) {
        if (strcmp(current_root,SUPER) != 0) {
            if ((status=copyroot(in,out,current_root)) != 0) goto fin;
            status = btchgr(in,SUPER);
            if (status != 0) goto fin;
        }
        /* no context maintained when switching roots, so need to
           position afresh */
        status = bfndky(in,current_root,&i);
        status = bnxtky(in,current_root,&i);
    }
    if (status == QNOKEY) status = 0;
  fin:
    if (status != 0) {
        btcerr(&status,&ioerr,fname,msg);
        fprintf(stderr,"%s: btree error (%d) [%s] - %s\n",
                argv[0],status,fname,msg);
    }
    status = btcls(in);
    if (status == 0) status = btcls(out);
    
    return(0);

}

int copyroot(BTA* in, BTA* out, char *rootname)
{
    static int bufsiz = 1024;
    int status, i, rsiz;
    static char *buf = NULL;
    char key[ZKYLEN];

    if (buf == NULL) {
        kalloc(&buf,bufsiz);
    }

    status = btchgr(in,rootname);
    if (status != 0) return status;
    status = btcrtr(out,rootname);
    /* ok to get duplicate key error if creating $$default */
    if (!(status == QDUP || status == 0)) return status;
    /* ok to get no key error */
    status = bfndky(in,"",&i);
    if (!(status == QNOKEY || status == 0)) return status;
    status = 0;
    while (status == 0) {
        status = btseln(in,key,buf,bufsiz,&rsiz);
        if (status != 0) continue;
        while (bufsiz == rsiz) {
            /* assume the record is bigger than the buffer,
             * so increase buffer size
             * repeat until we read ok, or run out of memory */
            free(buf);
            bufsiz += bufsiz;
            kalloc(&buf,bufsiz);
            status = btsel(in,key,buf,bufsiz,&rsiz);
            if (status != 0) return status;
        }
        status = btins(out,key,buf,rsiz);
    }
    return (status==QNOKEY)?0:status;
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
