/*
 * $Id$
 *
 * bdelky:  deletes key in index
 *
 * Parameters:
 *     b       index context pointer
 *     key     name of key to delete 
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
 * along with Foobar; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

int bdelky(BTA *b,char *key)
{

    int val,status;

    bterr("",0,NULL);
    if ((status=bvalap("BDELKY",b)) != 0) return(status);
    btact = b;

    if (btact->shared) {
        if (!block()) {
            bterr("BDELKY",QBUSY,NULL);
            goto fin;
        }
    }

    if (b->cntxt->super.smode != 0) {
        bterr("BDELKY",QNOWRT,NULL);
    }
    else {
        status = bfndky(b,key,&val);
        if (status == 0) {
            status = bdelk1(key);
        }
    }
fin:
    if (btact->shared) bulock();
    return(btgerr());
}
