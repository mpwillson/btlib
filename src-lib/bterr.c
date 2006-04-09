/*
 * $Id: bterr.c,v 1.11 2005/12/28 13:58:09 mark Exp $
 *
 * btcerr: returns last error code, io error code and appropriate
 *         message
 *
 * Parameters:
 *   ierr   returned with number of last error
 *   ioerr  returned with number of last i/o error (mostly)
 *   srname returned with name of subroutine which detected
 *          the error
 *    msg    returned with a text message for the error
 *
 * bterr:  set error code
 * bgterr: returns error code
 * 
 * Copyright (C) 2003, 2004 Mark Willson.
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

#include  <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "bc.h"


static int qerror = 0;
static int qcode = 0;
static char qarg[72];
static int syserror;

static char qname[72];

char *msgblk[] = {
    "null",
    " The block at the super root location is not a root",
    " Unable to close index file",
    " Unable to create index file",
    " Unable to read source or destination block ",
    " I/O error writing block",
    " I/O error reading super root",
    " I/O error writing super root",
    " I/O error opening index file",
    " I/O error reading block %s",
    " An index file is already open",
    " Can't split full block",
    " Bad info block index used: %s",
    " Unable to acquire a free memory block",
    " Stack underflow",
    " Stack overflow",
    " Can't insert key at block",
    " Replace location out of range: %s",
    " Split: search for middle key failed",
    " Requested write block not in memory",
    " Balance: search for key failed",
    " Exact flag not set for delete",
    " Internal inconsistency in delete operation",
    " Search for deleted key replacement failed",
    " Demote search failed",
    " Demote split failed",
    " Join search failed",
    " Cannot locate default root ($$default)",
    " Cannot delete the current root",
    " Negative in-memory index encountered",
    " No index file open for this operation",
    " Index file already in use",
    " Debug option not recognised",
    " No more index files may be opened (limit reached)",
    " Invalid index file context pointer",
    " File is busy",
    " Function prohibited in shared access mode",
    " No block available for data storage",
    " Data block usage gone bad: %s",
    " Data segment header references a non-data block: %s",
    " Unable to open index file",
    " Circular data segment pointer encountered",
    " Unlock operation failed",
    " LRU queue corrupt - index not in list",
    " Data record action not permitted for current root",
    " Data record length cannot be negative",
    " Key \"%s\" already exists in index",
    " Key \"%s\" does not exist in index",
    " Write access to index prohibited",
    " Block on free list is not marked as free",
    " Index file is incompatible with this version: %s",
    " Data capacity exceeded at block: %s",
    " Index file is at maximum size",
    " Unable to set alarm for locking",
    " No message exists for this error code"
};

#define MSGBLKMAX ((int) (sizeof(msgblk)/sizeof(msgblk[0])))

void btcerr(int *ierr,int *ioerr,char *srname,char *msg)
{
    strcpy(srname,qname);
    *ierr = qerror;
    *ioerr = qcode;

    if (qerror >= MSGBLKMAX) qerror = MSGBLKMAX-1;
    strcpy(msg,msgblk[qerror]);
    if (qcode != 0) {
        if (syserror) {
            sprintf(msg,"%s (System error: %s)",msgblk[qerror],strerror(qcode));
        }
        else {
            sprintf(msg,"%s (Info: %d)",msgblk[qerror],qcode);
        }
    }
    else {
        sprintf(msg,msgblk[qerror],qarg);
    }
    
    return;
}
/*
  bterr: save error and io codes and return

  void bterr(char *name,int ierr,char* arg)

    name   name of function which detected the error
    ierr   error code
    arg    argument to use with error message

  To reset saved error codes, use a call of the form bterr("",0,NULL);
*/

void bterr(char *name, int errorcode, char* arg)
{

    if (strlen(name) == 0) {
        errno = 0;
        syserror = FALSE;
        qname[0] = '\0';
        qerror = 0;
        qcode = 0;
        qarg[0] = '\0';
    }
    else if (qerror == 0) {
        strcpy(qname,name);
        qerror = errorcode;
        if (arg != NULL) strcpy(qarg,arg);
        if (errno != 0 && qcode == 0) {
            syserror = TRUE;
            qcode = errno; /* set to system errorcode */
            errno = 0;
        }
    }
}

/* Return last error code */

int btgerr() {
    return qerror;
}
