/*
 * $id$
 *
 * bremky: deletes key (and rlink) at pos from blk
 *
 * Parameters:
 *   blk   number of block from which removal required
 *   pos   position of key and rlink 
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

#include <string.h>

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

void bremky(int blk,int pos)
{
    int idx,i,ioerr;

    ioerr = brdblk(blk,&idx);
    if (idx < 0) {
        bterr("BREMKY",QRDBLK,itostr(blk));
        goto fin;
    }
    ((btact->memrec)+idx)->infblk[ZNKEYS]--;
    for (i=pos;i<((btact->memrec)+idx)->infblk[ZNKEYS];i++) { 
        strcpy(((btact->memrec)+idx)->keyblk[i],
               ((btact->memrec)+idx)->keyblk[i+1]);
        ((btact->memrec)+idx)->valblk[i] = ((btact->memrec)+idx)->valblk[i+1];
        ((btact->memrec)+idx)->lnkblk[i+1] = ((btact->memrec)+idx)->lnkblk[i+2];
    }

    ((btact->cntrl)+idx)->writes++;
fin:
    return;
}
