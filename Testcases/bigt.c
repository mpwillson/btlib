/*
 * bigt is a stress test for the B Tree library, to ensure it handles
 * the largest files properly.
 *
 * The Btree should fail gracefully at around 2GB on a machine with 32
 * bit integers.  
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
    char data[DATASIZE];
    char key[32],rname[32],msg[132];
    BTA *bt;
    
    btinit();

    bt = btcrt("test_db",0,FALSE);

    for (i=0;i<DATASIZE;i++) {
        data[i] = 'D';
    }

    for (i=0;i<INT_MAX;i++) {
        sprintf(key,"%d",i);
        status = btins(bt,key,data,DATASIZE);
        if (status != 0) {
            btcerr(&errorcode,&ioerror,rname,msg);
            printf("BTree error: %d [%s]: %s\n",errorcode,rname,msg);
            btcls(bt);
            return EXIT_FAILURE;
        }
    }

    btcls(bt);
    return EXIT_SUCCESS;
}

        
