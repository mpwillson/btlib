/*
 *   bclrlf: initialises last key found info
 */

#include <stdio.h>
#include <string.h>
#include "bc.h"
#include "bt.h"

void bclrlf(void)
{
    btact->cntxt->lf.lfblk = 0;
    btact->cntxt->lf.lfpos = 0;
    btact->cntxt->lf.lfexct = FALSE;
    strcpy(btact->cntxt->lf.lfkey,"");
    return;
}
