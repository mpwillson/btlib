/*    bacfre: Frees malloc'd memory acquired by bacini for index context
 *
 *      b - pointer to BT context
 */

#include <stdlib.h>

#include "bc.h"
#include "bt.h"

void bacfre(BTA *b)
{
    if (b != NULL) {
        b->idxfid[0] = '\0';
        b->idxunt = NULL;
        if (b->memrec != NULL) free(b->memrec);
        if (b->cntrl != NULL) free(b->cntrl);
        if (b->cntxt != NULL) free(b->cntxt);
    }
}

