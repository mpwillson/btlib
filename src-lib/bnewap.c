/*  Return pointer to free bt active slot, NULL if none free
 *
 *  BTA *bnewap(char *fid)
 *
 *      fid - file name for this slot
 *
 *  NULL returned if not slots free or file already open (error signalled)
 */

#include <stdio.h>
#include <string.h>

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

BTA *bnewap(char *fid)
{
    int i;

    /* file already in use? */
    for (i=0;i<ZMXACT;i++)
        if (strcmp(fid,btat[i].idxfid) == 0) break;
    if (i < ZMXACT) {
        bterr("BNEWAP",QINERR,0);
        return(NULL);
    }

    /* new file; return free context slot */
    for (i=0;i<ZMXACT;i++) 
        if (btat[i].idxunt == NULL) break;
    
    if (i < ZMXACT)
        return(btat+i);
    else
        return(NULL);
}
