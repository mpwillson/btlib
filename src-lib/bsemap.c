/*
 * block   - locks active BT file
 * bunlock - unlocks active BT file
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

#include "bc.h"
#include "bt.h"
#include "btree_int.h"

#define ZMXTRY 5
#define ZSLEEP 1
#define MXFILE 72
/* #undef DEBUG
   #define DEBUG 2 */

static sigjmp_buf env_alrm;

static struct flock lck = {0, SEEK_SET, 0, 0, 0 };

/* handle timeouts for write access */

static void sigalrm_handler(int signo) 
                            
{
    siglongjmp(env_alrm,1);
    return;
}

/* lock current BT file
 * return FALSE if unable to lock
 */

int block(void)
{
    int i, ierr;
    
    if (btact->lckcnt > 0 ) {
        btact->lckcnt++;
#if DEBUG >= 2
        fprintf(stderr,"BLOCK: soft lock - lcknt %d\n",btact->lckcnt);
#endif      
        return (TRUE);  /* lock already in use for this process */
    }

    lck.l_type = F_WRLCK;
    if (signal(SIGALRM,sigalrm_handler) == SIG_ERR) {
        fprintf(stderr,"can't set signal SIGALRM\n");
    }   

    /* save current signal mask in environment; enables proper repeat
     * behaviour when repetive calls to block are made */
    if (sigsetjmp(env_alrm,1) != 0) {
        /* get here when alarm goes off; couldn't lock file */
        signal(SIGALRM,SIG_DFL);
        alarm(0);
        return(FALSE);
    }

    alarm(ZMXTRY);
    
    if (fcntl(btact->fd,F_SETLKW,&lck) == -1) {
        signal(SIGALRM,SIG_DFL);
        alarm(0);
        return(FALSE); 
    }
    alarm(0); 
    
#if DEBUG >= 2
    fprintf(stderr,"BLOCK: hard lock - lcknt %d\n",btact->lckcnt);
#endif
    if (btact->shared && btact->cntxt->super.scroot != ZNULL) {
        /* ensure super block info is up-to-date */
        brdsup(); 
        /* ensure current root is in memory and locked */
        /* if not creating new index file (when scroot is ZNULL) */
        ierr = brdblk(btact->cntxt->super.scroot,&i);
        if (ierr == 0) {
            bsetbs(btact->cntxt->super.scroot,1);
        }
    }
    btact->lckcnt = 1;
    return(TRUE);
}

/* unlock file: it is not an error if file is currently unlocked */

int bulock(void)
{
    if (btact->lckcnt > 1) {
        /* don't unlock if lock request nested (only top level will
         * unlock) */
#if DEBUG >= 2
        fprintf(stderr,"BULOCK: soft unlock - lckcnt %d\n",btact->lckcnt);
#endif      
        btact->lckcnt--;
        return(0);
    }
    else if (btact->lckcnt == 1) {
#if DEBUG >= 2
        fprintf(stderr,"BULOCK: hard unlock - lckcnt %d\n",btact->lckcnt);
#endif
    
        /* write any changed in-memory blocks to file */
        btsync();
    
        if (btact->fd >= 0) {
            lck.l_type = F_UNLCK;
            if (fcntl(btact->fd,F_SETLK,&lck) == -1) {
                bterr("BULOCK",QUNLCK,NULL);
            }
        }
        btact->lckcnt = 0;
    }
    
    return(0);
}   


