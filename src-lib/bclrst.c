/*
 *   bclrst: initialises B tree statistic counters
 */

#include <stdio.h>
#include <string.h>
#include "bc.h"
#include "bt.h"

void bclrst()
{
    memset(&btact->cntxt->stat,0,sizeof(struct _stat));
    return;
}
