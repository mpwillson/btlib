/*
  btcerr: returns last error code, io error code and appropriate
          message

  void btcerr(int *ierr,int *ioerr,char *srname,char *msg)

       ierr   returned with number of last error
       ioerr  returned with number of last i/o error (mostly)
       srname returned with name of subroutine which detected
              the error
       msg    returned with a text message for the error

*/

#include  <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "bc.h"


static int qerror;
static int qcode;
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
    " I/O error reading block",
    " An index file is already open",
    " Can't split full block",
    " Bad info block index used",
    " Unable to acquire a free memory block",
    " Stack underflow",
    " Stack overflow",
    " Can't insert key at block",
    " Replace location out of range",
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
    " Data block usage gone negative",
    " Corrupt data segment header encountered",
    " Unable to open index file",
    " Circular data segment pointer encountered",
    " Unlock operation failed",
    " LRU queue corrupt - index not in list",
    " Data record action not permitted for current root",
    " Data record must not be of zero length",
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
        sprintf(msg,"%s",msgblk[qerror]);
    }
    
    return;
}
/*
  bterr: save error and io codes and return

  void bterr(char *name,int ierr,int code)

    name   name of function which detected the error
    ierr   error code
    code   relevant additional code (usually i/o error)

  To reset saved error codes, use a call of the form bterr("",0,0);
*/

void bterr(char *name,int ierr,int code)
{

    if (qerror == 0 || strlen(name) == 0) {
        syserror = FALSE;
        strcpy(qname,name);
        qerror = ierr;
        qcode = code;
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
