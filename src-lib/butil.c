/*
 *  butil:  utility routines for the B Tree library
 */

#include "btree_int.h"

static char buf[80];

char* itostr(int v)
{
    sprintf(buf,"%d",v);
    return buf;
}
