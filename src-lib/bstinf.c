/*
 * $id$
 *
 *
  bstinf: set information about block

  int bstinf(int blk,int type,int val)

    blk    block for which information must be set
    type   type of information to set
    val    info value

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
#define MASK 2**((ZBPW/2)*ZBYTEW)-1


#include "bc.h"
#include "bt.h"
#include "btree_int.h"
#include <math.h>

int bstinf(int blk,int type,int val)
{
    int ioerr,idx;

    if (type >= ZINFSZ)
        bterr("BSTINF",QINFER,NULL);
    else {
        ioerr = brdblk(blk,&idx);
        if (idx < 0) {
            bterr("BSTINF",QRDBLK,itostr(blk));
        }
        else {
            switch (type) {
                case ZBTYPE:
                    ((btact->memrec)+idx)->infblk[type] =
                        (ZVERS << ((ZBPW/2)*ZBYTEW)) | val;
                case ZBTVER:
                    break; /* always set implicitly by ZBTYPE */
                default:
                    ((btact->memrec)+idx)->infblk[type] = val;
            }
            ((btact->cntrl)+idx)->writes++;
        }
    }
    return(0);
}


