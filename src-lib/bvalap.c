/*
  bvalap -  Validates the index context pointer passed

     int bvalap(char *fn,BTA *b);

         fn  - calling routine (for proxy error message)
         b   - index context pointer
*/

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

int bvalap(char *fn,BTA *b)
{
    if (b < btat || b >= btat+ZMXACT) {
        bterr(fn,QBADAP,0);
        return(QBADAP);
    }
    if (b->idxunt == NULL) {
        bterr(fn,QBADAP,0);
        return(QBADAP);
    }
    return(0);
}
