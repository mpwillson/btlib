/*
 * $Id: bigt.c,v 1.3 2004/10/05 17:47:31 mark Exp $
 * 
 * NAME
 *      bigt - a stress test for the B Tree library, to ensure the 
 *      largest files are handled properly.
 *
 * SYNOPSIS
 *      bigt [-n record_count]
 *
 *      record_count defines the number of records to be written to
 *      the B Tree index file.  If not specified, INT_MAX records are
 *      attempted to be written.
 *
 *  DESCRIPTION
 *      Bigt creates a database named test_db in the working directory
 *      and inserts up to record_count keys and associated data
 *      records.  The default data record is 1024 bytes, filled with
 *      the character 'D'.
 * 
 *      Bigt should fail gracefully at around 2GB on a machine with 32
 *      bit integers.  
 *
 *  MODIFICATION HISTORY
 *  Mnemonic        Rel Date    Who
 *  BIGT            1.0 040923  mpw
 *      Created.
 *  BIGT            1.1 041004  mpw
 *      Added -n command switch.
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


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "btree.h"

#define TRUE 1
#define FALSE 0

#define DATASIZE 1024

int main(int argc, char *argv[])
{
    int status;
    int errorcode,ioerror,i;
    char *s;
    char data[DATASIZE];
    char key[ZKYLEN],rname[ZKYLEN],msg[132];
    int nrecs = INT_MAX;
    BTA *bt;

    while (--argc > 0 && (*++argv)[0] == '-') {
        for (s=argv[0]+1;*s != '\0'; s++) {
            switch (*s) {
                case 'n':
                    nrecs = atoi(*++argv);
                    --argc;
                    break;
                default:
                    fprintf(stderr,"bigt: unknown command switch: -%c\n",*s);
                    return EXIT_FAILURE;
            }
        }
    }
                    
    btinit();

    bt = btcrt("test_db",0,FALSE);

    for (i=0;i<DATASIZE;i++) {
        data[i] = 'D';
    }

    for (i=0;i<nrecs;i++) {
        sprintf(key,"%d",i);
        status = btins(bt,key,data,DATASIZE);
        if (status != 0) {
            btcerr(&errorcode,&ioerror,rname,msg);
            printf("While attempting to insert key: %s;\n",key);
            printf("\tBTree error: %d [%s]: %s\n",errorcode,rname,msg);
            btcls(bt);
            return EXIT_FAILURE;
        }
    }

    btcls(bt);
    return EXIT_SUCCESS;
}

        