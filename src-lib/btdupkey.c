/*
 * $Id:$
 *
 *
 * btdupkey:  inserts duplicate key into index
 *
 * Parameters:
 *    key    key to insert
 *    val    value of key 
 *
 * btdupkey returns non-ZERO if error occurred
 *
 * Copyright (C) 2012 Mark Willson.
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
 * along with the B Tree library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "btree.h"
#include "btree_int.h"
#include <string.h>

int btdupkey(char key[ZMXKEY], BTint val)
{
    int n =  btact->cntxt->lf.lfpos;
    int ioerr, idx;

    ioerr = brdblk(btact->cntxt->lf.lfblk,&idx);
    /* if first duplication, need to create duplicate block */
    if (((btact->memrec)+idx)->dupblk[n] == 0) {
        fprintf(stderr,"creating new duplicate block.\n");
    }
    else {
        fprintf(stderr,"adding to existing duplicate block.\n");
    }
    return 0;
}
