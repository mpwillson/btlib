/*
 * bigt is a stress test for the B Tree library, to ensure it handles
 * the largest files properly.
 *
 * The Btree should fail gracefully at around 2GB on a machine with 32
 * bit integers.  
 * 
 */

#include "btree.h"

#include <stdio.h>
#include <limits.h>

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
            exit(1);
        }
    }

    btcls(bt);
}

        
