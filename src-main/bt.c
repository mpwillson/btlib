/*
 * $Id: bt.c,v 1.17 2010-06-07 16:45:00 mark Exp $
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
#include <signal.h>
#include <setjmp.h>

#ifdef READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"

/* Structures for handling active bt context pointers */
struct bt_plist {
    char *fid;
    BTA *b;
    struct bt_plist *next;
};

struct bt_plist *phead = NULL;

/* Structures for handling data block definitions */
struct bt_blist {
    char *name;
    char *bptr;
    char *fn;
    int size;
    struct bt_blist *next;
};

struct bt_blist *bhead = NULL;

#define EMPTY ""

/*------------------------------------------------------------------------
 * Local function prototypes
 *------------------------------------------------------------------------
 */
BTA *get(char*);
void list();
void add_data_file(char*,char*);
struct bt_blist *get_data(char*);
int add_data(char*,int);
int add(char *, BTA *);
int del_data(char *);
int del(BTA *);
struct bt_blist *cpfm(char*);
char *strsave(char*);
int pushcf(FILE*);
FILE* pullcf();
/*
 *------------------------------------------------------------------------
 */

/* Setup for handling interrupts */
jmp_buf env;

void break_handler (int sig)
{
    longjmp(env,1);
}

/* bt command routines */

/* create data Buffer */
int block(CMDBLK* c)
{
    /* delete any previous definition */
    del_data(c->arg);
    if (c->qual_int == 0) {
        /* assume filename given */
        add_data_file(c->arg,c->qual);
    }
    else {
        if (!add_data(c->arg,c->qual_int)) {
            fprintf(stderr,"bt: cannot create data block: %s\n",c->arg);
            return 1;
        }
    }
    return 0;
}

/* delete data Buffer */
int block_delete(CMDBLK* c)
{
    if (!del_data(c->arg)) {
        fprintf(stderr,"bt: unable to delete data block: %s\n",c->arg);
        return 1;
    }
    return 0;
}

/* list known data blocks */
int block_list(CMDBLK* c)
{
    struct bt_blist *b;

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
    return 0;
}

/* create index file */
int create_file(CMDBLK* c)
{
    BTA* svbtp = btp;

    btp = btcrt(c->arg,0,c->qualifier[0] == 's');
    if (btp != NULL)  {
        add(c->arg,btp);
    }
    else {
        btp = svbtp;
        return 1;
    }
    return 0;
}

/* open index file */
int open_file(CMDBLK* c)
{
    BTA* svbtp = btp;

    btp = btopn(c->arg,0,c_>qualifier[0] == 's');
    if (btp != NULL) {
        add(c->arg,btp);
    }
    else {
        btp = svbtp;
        return 1;
    }
}

/* quit command */
int quit(CMDBLK* c)
{
    int ierr = 0;

    while (phead != NULL && ierr == 0) {
        ierr = btcls(phead->b);
        phead = phead->next;
    }
    if (ierr != 0) {
        fprintf(stderr,"Failed to close all btree index files\n");
    }
    return ierr;
}

/* close current index file */
int close_file(CMDBLK* c)
{
    int ierr;
    
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

int find_key(CMDBLK* c)
{
    int ierr;
    BTint val;
    
    ierr = bfndky(btp,c->arg,&val);
    if (ierr == 0)
        printf("Key: '%s' = " ZINTFMT "\n",c->arg,val);
    else if (ierr == QNOKEY) {
        if (strcmp(c->arg,EMPTY) != 0) {
            printf("No such key as '%s'\n",c->arg);
        }
        else {
            ierr = 0;
        }
    }
    return ierr;
}

int define_key(CMDBLK* c)
{
    int ierr;
    
    ierr = binsky(btp,c->arg,c->qual_int);
    if (ierr == QDUP) printf("Key '%s' exists\n",c->arg);
    return ierr;
}

int show_debug(CMDBLK* c)
{
    return bdbug(btp,c->arg,c->qual_int);
}

int next_key(CMDBLK* c)
{
    int ierr;
    BTint val;
    char key[ZKYLEN];
    
    ierr = bnxtky(btp,key,&val);
    if (ierr == QNOKEY) {
        printf("No more keys\n");
    }
    else {
        printf("Key: '%s' = " ZINTFMT "\n",key,val);
    }
    return ierr;
}

int list_keys(CMDBLK* c)
{
    int ierr = 0;
    BTint val;
    char key[ZKYLEN];
    
    while (ierr == 0) {
        ierr = bnxtky(btp,key,&val);
        if (ierr == 0) printf("Key: '%s' = " ZINTFMT "\n",key,val);
    }
    return ((ierr==0||ierr==QNOKEY)?0:ierr);
}

int list_keys_only(CMDBLK* c)
{
    int ierr = 0;
    BTint val;
    char key[ZKYLEN];
    
    while (ierr == 0) {
        ierr = bnxtky(btp,key,&val);
        if (ierr == 0) printf("%s\n",key);
    }
    return ((ierr==0||ierr==QNOKEY)?0:ierr);
}

int remove_key(CMDBLK* c)
{
    int ierr;
    
    ierr = bdelky(btp,c->arg); 
    if (ierr != 0) printf("Key: '%s' not found\n",c->arg);
    return ierr;
}

int define_root(CMDBLK* c)
{
    int ierr;
    
    ierr = btcrtr(btp,c->arg);
    if (ierr != 0) printf("Can't create root: '%s'\n",c->arg);
    return err;
}

int change_root(CMDBLK* c)
{
    int ierr;
    
    ierr = btchgr(btp,c->arg);
    if (ierr == QNOKEY) printf("Can't change root to: '%s'\n",c->arg);
    return ierr;
}

int remove_root(CMDBLK* c)
{
    int ierr;
    
    ierr = btdelr(btp,arg[1]);
    if (ierr != 0) printf("No such root as '%s'\n",arg[1]);
    return ierr;
}

/* print list of active index files */
int file_list(CMDBLK* c)
{
    struct bt_plist *p;

    p = phead;
    while (p != NULL) {
        printf("%s\n",p->fid);
        p = p->next;
    }
    return 0;
}

int use_file(CMDBLK* c)
{
    btp = get(c->arg);
    if (btp == NULL) {
        fprintf(stderr,"bt: %s not found; nothing current\n",arg[1]);
    }
    return 0;
    
}

int define_data(CMDBLK* c)
{
    int ierr;
    struct bt_blist *blk;
    
    /* check for use of data block */
    if (c->qualifier[0] == '*') {
        blk = get_data((c->qualifier)+1);
        if (blk != NULL) {
            ierr = btins(btp,c->arg,blk->bptr,blk->size);
        }
        else {
            fprintf(stderr,"bt: no such data block: %s\n",(c->qualifier)+1);
        }
    }
    else {
        ierr = btins(btp,c->arg,c->qualifier,strlen(c->qualifier));
    }
    if (ierr == QDUP) printf("Key '%s' exists\n",c->arg);
    return ierr;
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
                    rbuf = (char *) malloc(i+1);
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
                i = 0;
                ierr = btsel(btp,arg[1],buff,80,&i);
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
                fprintf(stderr,"No such key as '%s'\n",arg[1]);
            }
            else {
                printf("Key '%s' record size: %d bytes\n",arg[1],size);
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
            ierr = bupdky(btp,arg[1],atoi(arg[2])); /* only int
                                                       update! */
            if (ierr != 0) {
                fprintf(stderr,"Can't update value for: '%s'\n",arg[1]);
            }
        }
        else if (strcmp(arg[0],"da") == 0) {
            ierr = bfndky(btp,arg[1],&val);
            if (ierr == 0) {
                BTint dblk;
                int offset;
                cnvdraddr(val,&dblk,&offset);
                printf("Key: '%s' = " ZINTFMT ", dblk: " ZINTFMT
                       ", offset; %d\n",arg[1],val,dblk,offset);
            }
            else if (ierr == QNOKEY) {
                if (strcmp(arg[1],EMPTY) != 0)
                    printf("No such key as '%s'\n",arg[1]);
            }
        }
        /* empty command */
        else if (strlen(arg[0]) == 0)
            continue;
        else {
            printf("eh? - type ? for help\n");
        }

CMDBLK bt_cmds[] = {
  { "block","b",block,"name size",2,"Create data block of size bytes.\n" },
  { "block","b",block,"name file",2,"Creates data block with contents of file.\n" },
  { "block-delete","bd",block_delete,"name",1,"Delete data block." },
  { "block-list","bl",block_list,"",0,"List data blocks." },
  { "create","c",create_file,"file [s]",1,"Create index file. s qualifier indicates shared mode." },
  { "change-root","cr",change_root,"rootname",1,"Change to named root." },
  { "define","d",define_key,"key val",2,"Define key with associated value." },
  { "decode-address","da",decode_addr,"key",1,"Print decoded data segment address for key." },
  { "define-data","dd",define_data,"key data",2,"Define key with data. Use *<name> to refer to data block." },
  { "define-root","rd",define_root,"root",1,"Define new root.\n" },
  { "find","f",find_key,"key",1,"Find key." },
  { "find-data","fd",fine_data,"key [d]",1,"Find key with data. Use d qualifier to display whole data record." },
  { "file-list","fl",file_list,"",0,"List open index files." },
  { "list","l",list_keys,"",0,"List all keys and values following last find operation." },
  { "list-data","ld",list_data,"",0,"List all keys and data following last find operation." },
  { "lock","lk",lock_file,"",0,"Lock current index file." },
  { "list-keys-only","lko",list_keys_only,"",0,"List all keys (only) following last find operation." },
  { "next","n",next_key,"",0,"Display next key and value." },
  { "next-data","nd",next_data,"",0,"Display next key and associated data." },
  { "open","o",open_file,"file [s]",1,"Open existing index file.  s qualifier indicates shared mode." },
  { "quit","q",quit,"",0,"Quit bt program." },
  { "remove","r",remove_key,"key",1,"Remove key." },
  { "remove-data","rd",remove_data,"key",1,"Remove key and associated data." },
  { "remove-root","rr",remove_root,"root",1,"Remove root." },
  { "show","s",_show_debug,"arg",1,"Show debug info. <arg> any of control,super,stats,space,stack,block <n>." },
  { "size-data","sd",size_data,"key",1,"Display size of data record for <key>." },
  { "use","u",use_file,"file",1,"Make <file> current in-use index file." },
  { "update-data","ud",update_data,"key data",2,"Update key with new data record. Use *<name> to refer to data block" },
  { "unlock","ul",unlock_file,"",0,"Unlock current index file." },
  { "update-value","uv",update_value,"key val",2," Update value of <key> to <val>." },
  { "x","x",close_file,"",0,"Close current index file." },
};

int main(int argc,char *argv[])
{
    char buff[ZMSGSZ],*arg[4],key[ZKYLEN],fid[ZMSGSZ],name[ZRNAMESZ],*ps;
    char *cp = NULL,*rlbuf = NULL;
    
    BTint val;
    int i,ierr,ioerr;
    int found,prompt,svp,quit,size;
    FILE *unit;
    char *rbuf;
    
    BTA *btp = NULL, *svbtp;
    struct bt_blist *blk;

    prompt = TRUE;
    quit = FALSE;
    unit = stdin;
    ps = "bt: ";

    btinit();

    /* catch interrupts and always return here */
    setjmp(env);
    signal(SIGINT,break_handler);
    fflush(stdout);
    
    /* read command from command stream (issue prompt if required) */
    while (!quit) {
        if (tty_input(unit)) {
#ifdef READLINE
            if (prompt) 
                rlbuf = readline(ps);
            else    
                rlbuf = readline(NULL);
            strcpy(buff,rlbuf);
#else
            if (prompt) printf("%s",ps);
            cp = fgets(buff,80,unit);
#endif            
        }
        else {
            cp = fgets(buff,80,unit);
        }
        if (cp == NULL && rlbuf == NULL) {
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
        if ((cp = strchr(buff,'!')) != NULL) {
            system(cp+1);
            continue;
        }

        /* get first token from command buffer */
        arg[0] = strtok(buff," \n");

        /* ignore empty line or comment (line starting with '#') */
        if (arg[0] == NULL || *arg[0] == '#') continue;
#ifdef READLINE
        if (tty_input(unit)) {
            add_history(rlbuf);
            free(rlbuf);
            rlbuf = NULL;
        }
#endif        
        /* extract up to four arguments from buff */
        for (i=1;i<4;i++) {
            arg[i] = strtok(NULL," \n");
            if (arg[i] == NULL) arg[i] = EMPTY;
        }
        
        
        /* check for error in B tree */
        if (ierr != 0 && ierr != QDUP && ierr != QNOKEY)  {
            btcerr(&ierr,&ioerr,name,buff);
            fprintf(stdout,"(%s) [%d] %s\n",name,ierr,buff);
        }
    }
    signal(SIGINT,SIG_DFL);
    return (0);
}

/* remember active index file */
int add(char *f, BTA* b)
{
    struct bt_plist *p;

    p = (struct bt_plist *) malloc(sizeof(struct bt_plist));
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
    struct bt_plist *p;

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
    struct bt_plist *p,*lp;

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


/* create new data block */
int add_data(char *f, int size)
{
    struct bt_blist *b;

    b = (struct bt_blist *) malloc(sizeof(struct bt_blist));
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
struct bt_blist *get_data(char *f)
{
    struct bt_blist *b;

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
    struct bt_blist *b,*lb;

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



/* add data block from file contents */
void add_data_file(char *bn, char *fn)
{
    struct bt_blist *b;

    b = cpfm(fn);
    if (b != NULL) {
        b->name = strsave(bn);
        b->next = bhead;
        bhead = b;
    }
}


#define BUFSZ 256

/* create data block from named file contents */
struct bt_blist *cpfm(char *fn)
{
    struct bt_blist *b;
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
    if (fstat(fileno(in),&statbuf) != 0) {
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
    b = (struct bt_blist *) malloc(sizeof(struct bt_blist));
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
