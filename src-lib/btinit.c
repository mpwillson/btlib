/*
 btinit: initialise B tree tables 
*/

#include <stdio.h>
#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

/*  Declare context structure for tracking index usage */
BTA btat[ZMXACT];

/* Pointer to active index info */
BTA *btact = NULL;

int btinit()
{
    int i;
    static int btinited = FALSE;

    if (btinited) {
        bterr("BTINIT",QINERR,0);
        return(QINERR);
    }
    
    for (i=0;i<ZMXACT;i++) {
        btat[i].idxunt = NULL;
        btat[i].idxfid[0] = '\0';
        btat[i].cntrl = NULL;
        btat[i].memrec = NULL;
        btat[i].cntxt = NULL;
        btat[i].fd = -1;
    }

    /* use block size to set data record address field widths (block
       and offset) */
    setaddrsize(ZBLKSZ);
    
    btinited = TRUE;
    return(0);
}
