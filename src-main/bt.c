/*
 * $Id: bt.c,v 1.5 2004/09/26 13:07:39 mark Exp $
 * 
 * =====================================================================
 * test harness for B Tree routines
 * =====================================================================
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "bc.h"
#include "bt.h"
#include "btree.h"

/* Structures for handling active bt context pointers */
struct _plist {
    char *fid;
    BTA *b;
    struct _plist *next;
};

struct _plist *phead = NULL;

/* Structures for handling data block definitions */
struct _blist {
    char *name;
    char *bptr;
    char *fn;
    int size;
    struct _blist *next;
};

struct _blist *bhead = NULL;

#define EMPTY ""

/*------------------------------------------------------------------------
 * Local function prototypes
 *------------------------------------------------------------------------
 */
BTA *get();
void list();
void list_data();
void add_data_file();
struct _blist *get_data();
int add_data();
int add(char *, BTA *);
int del_data(char *);
int del(BTA *);
struct _blist *cpfm();
char *strsave();
int pushcf();
FILE* pullcf();


/*
 *------------------------------------------------------------------------
 */


int main(int argc,char *argv[])
{
    char buff[80],*arg[4],key[32],fid[72],name[6],*cp,*ps;
    int i,ierr,ioerr;
    int found,prompt,svp,quit,size;
    FILE *unit;
    struct stat statbuf;
    char *rbuf;
    
    BTA *btp = NULL, *svbtp;
    struct _blist *blk;

    prompt = FALSE;
    quit = FALSE;
    unit = stdin;

    if (fstat(unit->FILENO,&statbuf) == 0) {
        if ((statbuf.st_mode & S_IFMT) == S_IFCHR) prompt = TRUE;
    }
    else {
        fprintf(stderr,"bt: unable to fstat stdin\n");
        exit(-1);
    }

    ps = "bt: ";
    btinit();

    /* read command from command stream (issue prompt if required) */
    while (!quit) {
        if (prompt) printf(ps);
        cp = fgets(buff,80,unit);
        if (cp == NULL) {
            if (unit == stdin) {
                quit = TRUE;
            }
            else {
                /*  handle end-of-file on command file */
                fclose(unit);
                unit = pullcf();
                if (unit == NULL) {
                    fprintf(stderr,"bt: command file stack underflow\n");
                    unit = stdin;
                }
                else if (unit == stdin) {
                    prompt = svp;
                }
            }
            continue;
        }

        ierr = 0;
        ioerr = 0;

        /* check for system command */
        cp = strchr(buff,'!');

        if (cp != NULL) {
            system(cp+1);
            continue;
        }

        /* get first token from command buffer */
        arg[0] = strtok(buff," \n");

        /* ignore empty line or comment (line starting with '#') */
        if (arg[0] == NULL || *arg[0] == '#') continue;
        /* extract up to four arguments from buff */
        for (i=1;i<4;i++) {
            arg[i] = strtok(NULL," \n");
            if (arg[i] == NULL) arg[i] = EMPTY;
        }
        /* create data Buffer */
        if (strcmp(arg[0],"b") == 0) {
            del_data(arg[1]);           /* delete any previous
                                           definition */
            size = atoi(arg[2]);
            if (size == 0) {
                /* assume filename given */
                add_data_file(arg[1],arg[2]);
            }
            else {
                if (!add_data(arg[1],size)) {
                    fprintf(stderr,"bt: cannot create data block\n");
                }
            }
            
        }
        /* delete data Buffer */
        else if (strcmp(arg[0],"bd") == 0) {
            if (!del_data(arg[1])) {
                fprintf(stderr,"bt: unable to delete data block: %s\n",arg[1]);
            }
        }
        /* list data Buffers */
        else if (strcmp(arg[0],"bl") == 0) {
            list_data();
        }
        
        /*  check for Create */
        else if (strcmp(arg[0],"c") == 0) {
            svbtp = btp;
            btp = btcrt(arg[1],0,arg[2][0] == 's');
            if (btp != NULL)  {
                add(arg[1],btp);
            }
            else {
                btp = svbtp;
                ierr = 1;
            }
        }
        /*  check for Open */
        else if (strcmp(arg[0],"o") == 0){
            svbtp = btp;
            btp = btopn(arg[1],0,arg[2][0] == 's');
            if (btp != NULL) {
                add(arg[1],btp);
            }
            else {
                btp = svbtp;
                ierr = 1;
            }
        }
        /*  check for Quit */
        else if (strcmp(arg[0],"q") == 0) {
            ierr = 0;
            while (phead != NULL && ierr == 0) {
                ierr = btcls(phead->b);
                phead = phead->next;
            }
            
            if (ierr != 0) {
                fprintf(stderr,"Failed to close all btree index files\n");
            }
            
            return(0);
        }
        /* check for X (close) */
        else if (strcmp(arg[0],"x") == 0) {
            if (btp != NULL) {
                ierr = btcls(btp);
                if (ierr == 0) {
                    del(btp);
                    /* force head of file list as in-use */
                    if (phead != NULL) {
                        btp = phead->b;
                        fprintf(stdout,"btree file %s now in use\n",phead->fid);
                    }
                    else {
                        btp = NULL;
                        fprintf(stdout,"warning: no in-use btree file\n");
                    }
                }
            }
        }
        /* check for Find */
        else if (strcmp(arg[0],"f") == 0) {
            ierr = bfndky(btp,arg[1],&i);
            if (ierr == 0)
                printf("Key: '%s' = %d\n",arg[1],i);
            else if (ierr == QNOKEY) {
                if (strcmp(arg[1],EMPTY) != 0)
                    printf("No such key as '%s'\n",arg[1]);
            }
        }
        /* check for Define */
        else if (strcmp(arg[0],"d") == 0 || strcmp(arg[0],"def") == 0) {
            i = atoi(arg[2]);
            ierr = binsky(btp,arg[1],i);
            if (ierr == QDUP) printf("Key '%s' exists\n",arg[1]);
        }
        /*  check for Stats */
        else if (strcmp(arg[0],"s") == 0) {
            i = atoi(arg[2]);
            ierr = bdbug(btp,arg[1],i);
        }
        /*  check for Next (key) */
        else if (strcmp(arg[0],"n") == 0) {
            ierr = bnxtky(btp,key,&i);
            if (ierr == QNOKEY) {
                printf("No more keys\n");
            }
            else {
                printf("Key: '%s' = %d\n",key,i);
            }   
        }
        /*  check for List (of keys) */
        else if (strcmp(arg[0],"l") == 0) {
            ierr = 0;
            while (ierr == 0) {
                ierr = bnxtky(btp,key,&i);
                if (ierr == 0) printf("Key: '%s' = %d\n",key,i);
            }
        }
        /*  check for ? (help) */
        else if (strcmp(arg[0],"?") == 0) {
            printf("b <name> <size> create data block of size bytes\n");
            printf("b <name> <file> creates data block with contents of file\n");
            printf("bd <name>       delete data block\n");
            printf("bl              list data blocks\n");
            printf("c <file> [s]    create index file\n");
            printf("                s qualifier indicates shared mode\n");
            printf("cr <root>       change to new root\n");
            printf("d <key> [<val>] define key\n");
            printf("dd <key> <data> define key with data\n");
            printf("                use *<name> to refer to data block\n");
            printf("dr <root>       define new root\n");
            printf("e <file>        obey commands from file\n");
            printf("f <key>         find key\n");
            printf("fd <key> [d]    find key with data\n");
            printf("                use d qualifier to display whole record\n");
            printf("fl              list open index files\n");
            printf("l               list keys from last find\n");
            printf("lk              lock index file\n");
            printf("ld              list keys and data from last find\n");
            printf("n               next key\n");
            printf("nd              next key and data\n");
            printf("o <file> [s]    open index file\n");
            printf("                s qualifier indicates shared mode\n");
            printf("p               toggle prompt\n");
            printf("q               quit\n");
            printf("r <key>         remove key\n");
            printf("rd <key>        remove key with data\n");
            printf("rr <root>       remove root\n");
            printf("s <arg>         show debug info\n");
            printf("                <arg> any of control,super,stats,\n");
            printf("                space,stack,block <n>\n");
            printf("sd <key>        print size of data record for <key>\n");
            printf("u <file>        make <file> current\n");
            printf("ud <key> <data> update key with new data\n");
            printf("ulk             unlock index file\n");
            printf("uv <key> <val>  update value of <key> to <val>\n");
            printf("x               close open index\n");

            printf("?               print this message\n");
            printf("#               comment line\n");
        }
        /*  check for Remove (key) */
        else if (strcmp(arg[0],"r") == 0) {
            ierr = bdelky(btp,arg[1]); 
            if (ierr != 0) printf("Key: '%s' not found\n",arg[1]);
        }
        /*  check for Prompt (toggle) */
        else if (strcmp(arg[0],"p") == 0) {
            prompt = !prompt;
        }
        /*  check for Execute (command file) */
        else if (strcmp(arg[0],"e") == 0) {
            if (!pushcf(unit)) {
                fprintf(stderr,"bt: command file stack exhausted at: %s\n",
                        arg[1]);
                continue;
            }
            if (unit == stdin) {
                svp = prompt;
                prompt = FALSE;
            }
            strcpy(fid,arg[1]);
            unit = fopen(fid,"rt");
            if (unit == NULL) {
                printf("Unable to open execute file: %s\n",fid);
                unit = pullcf();
                if (unit == stdin) {
                    prompt = svp;
                }
            }
        }
        /* check for Define Root command */
        else if (strcmp(arg[0],"dr") == 0) {
            ierr = btcrtr(btp,arg[1]);
            if (ierr != 0) printf("Can't create root: '%s'\n",arg[1]);
        }
        /* check for Change Root command */
        else if (strcmp(arg[0],"cr") == 0) {
            ierr = btchgr(btp,arg[1]);
            if (ierr == QNOKEY) printf("Can't change root to: '%s'\n",arg[1]);
        }
        /* check for Remove Root command */
        else if (strcmp(arg[0],"rr") == 0) {
            ierr = btdelr(btp,arg[1]);
            if (ierr != 0) printf("No such root as '%s'\n",arg[1]);
        }
        /* check for File List command */
        else if (strcmp(arg[0],"fl") == 0) {
            list();
        }
        /* check for Use command */
        else if (strcmp(arg[0],"u") == 0) {
            btp = get(arg[1]);
            if (btp == NULL) {
                fprintf(stderr,"bt: %s not found; nothing current\n",arg[1]);
            }
        }
        /* check for Define Data (dd) command */
        else if (strcmp(arg[0],"dd") == 0) {
            /* check for use of data block */
            if (arg[2][0] == '*') {
                blk = get_data(arg[2]+1);
                if (blk != NULL) {
                    ierr = btins(btp,arg[1],blk->bptr,blk->size);
                }
                else {
                    fprintf(stderr,"bt: no such data block: %s\n",arg[2]+1);
                }
            }
            else {
                ierr = btins(btp,arg[1],arg[2],strlen(arg[2]));
            }
            if (ierr == QDUP) printf("Key '%s' exists\n",arg[1]);
        }
        /* check for Find Data (fd) command */
        else if (strcmp(arg[0],"fd") == 0) {
            if (strcmp(arg[2],"d") == 0) {
                /* display retrieved record */
                ierr = btrecs(btp,arg[1],&i);
                if (ierr == QNOKEY) {
                    if (strcmp(arg[1],EMPTY) != 0) 
                        fprintf(stderr,"No such key as '%s'\n",arg[1]);
                }
                else {
                    rbuf     = (char *) malloc(i+1);
                    if (rbuf == NULL) {
                        fprintf(stderr,"bt: unable to allocate memory\n");
                        continue;
                    }
                    ierr = btsel(btp,arg[1],rbuf,i,&i);
                    *(rbuf+i) = '\0';
                    printf("%s",rbuf);
                    free(rbuf);
                    printf("\n");
                }
            }
            else {
                ierr = btsel(btp,arg[1],buff,80,&i);
                if (ierr == 0 && i >= 0) {
                    if (ierr == 0) {
                        buff[(i==80?i-1:i)] = '\0';
                        printf("Data returned: '%s'\n",buff);
                    }
                    else if (ierr == QNOKEY) {
                        if (strcmp(arg[1],EMPTY) != 0)
                            fprintf(stderr,"No such key as '%s'\n",arg[1]);
                    }
                }
            }
        }
        
        /* check for Update Data (ud) command */
        else if (strcmp(arg[0],"ud") == 0) {
            found = TRUE;
            if (arg[2][0] == '*') {
                blk = get_data(arg[2]+1);
                if (blk != NULL) {
                    ierr = btupd(btp,arg[1],blk->bptr,blk->size);
                }
                else {
                    fprintf(stderr,"bt: no such data block: %s\n",arg[2]+1);
                }
            }
            else {
                ierr = btupd(btp,arg[1],arg[2],strlen(arg[2]));
            }
            if (ierr == QNOKEY) {
                fprintf(stderr,"No such key as '%s'\n",arg[1]);
            }
        }
        /* check for Remove Data (rd) command */
        else if (strcmp(arg[0],"rd") == 0) {
            ierr = btdel(btp,arg[1]);
            if (ierr == QNOKEY) printf("Key '%s' not found\n",arg[1]);
        }
        /* Size data command? */
        else if (strcmp(arg[0],"sd") == 0) {
            ierr = btrecs(btp,arg[1],&size);
            if (ierr == QNOKEY) {
                printf("Key '%s' record size: %d bytes\n",arg[1],size);
            }
            else {
                fprintf(stderr,"No such key as '%s'\n",arg[1]);
            }
        }
        /* list all keys and data from current key */
        else if (strcmp(arg[0],"ld") == 0) {
            found = TRUE;
            ierr = 0;
            while (found && ierr == 0) {
                ierr = btseln(btp,key,buff,80,&size);
                if (ierr == 0) {
                    buff[(size==80?size-1:size)] = '\0';
                    printf("Key: '%s' - Data: '%s'\n",key,buff);
                }
            }
        }
        /* print next key and data */
        else if (strcmp(arg[0],"nd") == 0) {
            ierr = btseln(btp,key,buff,80,&size);
            if (ierr == 0) {
                buff[size] = '\0';
                printf("Key: '%s' - Data: '%s\n",key,buff);
            }
        }
        /* lock command */
        else if (strcmp(arg[0],"lk") == 0) {
            ierr = btlock(btp);
        }
        /* unlock command */
        else if (strcmp(arg[0],"ulk") == 0) {
            ierr = btunlock(btp);
        }
        /* update value command */
        else if (strcmp(arg[0],"uv") == 0) {
            ierr = bupdky(btp,arg[1],atoi(arg[2]));
            if (ierr != 0) {
                fprintf(stderr,"Can't update value for: '%s'\n",arg[1]);
            }
        }
        /* empty command */
        else if (strlen(arg[0]) == 0)
            continue;
        else {
            printf("eh? - type ? for help\n");
        }
        
        /* check for error in B tree */
        if (ierr != 0 && ierr != QDUP && ierr != QNOKEY)  {
            btcerr(&ierr,&ioerr,name,buff);
            fprintf(stdout,"(%s) [%d] %s\n",name,ierr,buff);
        }
    }
    return (0);
}

/* remember active index file */
int add(char *f, BTA* b)
{
    struct _plist *p;

    p = (struct _plist *) malloc(sizeof(struct _plist));
    if (p == NULL) {
        fprintf(stderr,"bt: no memory for active file entry\n");
        return(1);
    }
    p->fid = strsave(f);
    p->b = b;
    p->next = phead;
    phead = p;
    return(0);
}

/* return context pointer for named file */
BTA *get(char *f)
{
    struct _plist *p;

    p = phead;
    while (p != NULL) {
        if (strcmp(p->fid,f) == 0) return(p->b);
        p = p->next;
    }
    return(NULL);
}

/* delete record of passed pointer from context list */
int del(BTA *b)
{
    struct _plist *p,*lp;

    p = phead;
    lp = NULL;
    while (p != NULL) {
        if (b == p->b) {
            free(p->fid);
            if (lp != NULL)
                lp->next = p->next;
            else
                phead = p->next;
            free(p);
            return(0);
        }
        lp = p;
        p = p->next;
    }
    return(1);
}

/* print list of active index files */
void list(char *f)
{
    struct _plist *p;

    p = phead;
    while (p != NULL) {
        printf("%s\n",p->fid);
        p = p->next;
    }
    return;
}

/* create new data block */
int add_data(char *f, int size)
{
    struct _blist *b;

    b = (struct _blist *) malloc(sizeof(struct _blist));
    if (b == NULL) {
        fprintf(stderr,"bt: no memory for new data block header\n");
        return(FALSE);
    }
    
    b->name = strsave(f);
    b->fn = NULL;
    b->bptr = (char *) malloc(size);
    if (b->bptr == NULL) {
        fprintf(stderr,"bt: unable to allocate enough memory\n");
        free(b);
        return(FALSE);
    }
    memset(b->bptr,*f,size);
    b->size = size;
    b->next = bhead;
    bhead = b;
    return(TRUE);
}

/* return info on data block named */
struct _blist *get_data(char *f)
{
    struct _blist *b;

    b = bhead;
    while (b != NULL) {
        if (strcmp(b->name,f) == 0) return(b);
        b = b->next;
    }
    return(NULL);
}

/* delete data block named */
int del_data(char *n)
{
    struct _blist *b,*lb;

    b = bhead;
    lb = NULL;
    while (b != NULL) {
        if (strcmp(n,b->name) == 0) {
            free(b->name);
            if (lb != NULL)
                lb->next = b->next;
            else
                bhead = b->next;
            free(b);
            return(TRUE);
        }
        lb = b;
        b = b->next;
    }
    return(FALSE);
}

/* list known data blocks */
void list_data()
{
    struct _blist *b;

    b = bhead;
    while (b != NULL) {
        printf("%s \t[%d bytes]",b->name,b->size);
        if (b->fn != NULL) {
            printf("  (%s)\n",b->fn);
        }
        else {
            printf("\n");
        }
        b = b->next;
    }
    return;
}

/* add data block from file contents */
void add_data_file(char *bn, char *fn)
{
    struct _blist *b;

    b = cpfm(fn);
    if (b != NULL) {
        b->name = strsave(bn);
        b->next = bhead;
        bhead = b;
    }
}


#define BUFSZ 256

/* create data block from named file contents */
struct _blist *cpfm(char *fn)
{
    struct _blist *b;
    struct stat statbuf;
    FILE *in;
    char buf[BUFSZ+1];
    int nitems, offset, eof;

    in = fopen(fn,"r");
    if (in == NULL) {
        fprintf(stderr,"bt: unable to open file '%s'\n",fn);
        return(NULL);
    }
    /* get file info*/
    if (fstat(in->FILENO,&statbuf) != 0) {
        fprintf(stderr,"bt: fstat failed for file '%s'\n",fn);
        fclose(in);
        return(NULL);
    }
    /* is this a regular file? */
    if (!S_ISREG(statbuf.st_mode)) {
        fprintf(stderr,"bt: file '%s' is not a regular file\n",fn);
        fclose(in);
        return(NULL);
    }

    /* need new buffer entry */
    b = (struct _blist *) malloc(sizeof(struct _blist));
    if (b == NULL) {
        fprintf(stderr,"bt: no memory for buffer index entry\n");
        fclose(in);
        return(NULL);
    }

    /* remember file name of buffer; size is file size */
    b->fn = strsave(fn);
    b->size = statbuf.st_size;

    /* acquire memory to hold file contents */
    b->bptr = (char *) malloc(b->size);
    if (b->bptr == NULL) {
        fprintf(stderr,"bt: no memory for buffer\n");
        fclose(in);
        free(b);
        return(NULL);
    }

    /* read in contents of file */
    eof = FALSE;
    offset = 0;
    while (!eof) {
        nitems = fread(buf,sizeof(char),BUFSZ,in);
        eof = (nitems != BUFSZ);
        memcpy(b->bptr+offset,buf,nitems);
        offset += nitems;
        if (offset > b->size) {
            fprintf(stderr,"bt: internal error - buffer overflow\n");
            break;
        }
    }
    fclose(in);
    return(b);
}

/* save string in a safe place */
char *strsave(char *s)
{
    char *p;

    p = (char *) malloc(strlen(s)+1);
    if (p == NULL) {
        fprintf(stderr,"bt: no memory for string\n");
        return (NULL);
    }
    strcpy(p,s);    
    return (p);
}

/* Nested command file handling variables and functions */

#define CMDFILEMAX 5

FILE* cmdinput[CMDFILEMAX];
int cmdtop = 0;

int pushcf(FILE* cf)
{
    if (cmdtop < CMDFILEMAX) {
        cmdinput[cmdtop++] = cf;
        return TRUE;
    }
    else {
        return FALSE;
    }
}

FILE* pullcf()
{
    if (cmdtop <= 0) {
        return NULL;
    }
    else {
        return cmdinput[--cmdtop];
    }
}
