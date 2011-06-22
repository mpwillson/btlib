/*
 * $Id: btthresh.c,v 1.1 2011-06-13 19:34:46 mark Exp $
 *
 * bthresh: sets write through threshold for disk blocks
 *
 * Parameters:
 *    bta    btree index to set write through threshold for
 *    n      number of updates that causes block to be written to disk
 *
 * Copyright (C) 2011 Mark Willson.
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

#include "bc.h"
#include "bt.h"

int btthresh(BTA *b, int n)
{
    b->wt_threshold = n;
    return 0;
}
