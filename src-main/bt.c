/*
 * $Id: bt.c,v 1.18 2010-06-13 20:00:40 mark Exp $
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

#include "bc.h"
#include "bt.h"
#include "btree.h"
#include "btree_int.h"
#include "btcmd.h"

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

BTA *btp = NULL;

#define EMPTY ""
#define DATABUFSZ 80

/* Setup for handling interrupts */
jmp_buf env;

void break_handler (int sig)
{
    longjmp(env,1);
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

/* return info on data block */
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

/* delete data block */
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

/* bt command routines */

/* create data block */
int create_block(CMDBLK* c)
{
    /* delete any previous definition */
    del_data(c->arg);
    if (c->qual_int == 0) {
        /* assume filename given */
        add_data_file(c->arg,c->qualifier);
    }
    else {
        if (!add_data(c->arg,c->qual_int)) {
            fprintf(stderr,"bt: cannot create data block: %s\n",c->arg);
        }
    }
    return 0;
}

/* delete data block */
int block_delete(CMDBLK* c)
{
    if (!del_data(c->arg)) {
        fprintf(stderr,"bt: unable to delete data block: %s\n",c->arg);
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

    btp = btopn(c->arg,0,c->qualifier[0] == 's');
    if (btp != NULL) {
        add(c->arg,btp);
    }
    else {
        btp = svbtp;
        return 1;
    }
    return 0;
}

/* quit command */
int quit(CMDBLK* c)
{
    int status = 0;

    signal(SIGINT,SIG_DFL);

    while (phead != NULL && status == 0) {
        status = btcls(phead->b);
        phead = phead->next;
    }
    if (status != 0) {
        fprintf(stderr,"Failed to close all btree index files\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
    return status;
}

/* close current index file */
int close_file(CMDBLK* c)
{
    int status;
    
    if (btp != NULL) {
        status = btcls(btp);
        if (status == 0) {
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
    return status;
}

int find_key(CMDBLK* c)
{
    int status;
    BTint val;
    
    status = bfndky(btp,c->arg,&val);
    if (status == 0) {
        printf("Key: '%s' = " ZINTFMT "\n",c->arg,val);
    }
    else if (strlen(c->arg) == 0) {
        status = 0; /* suppress error on empty key, probably a
                       position find */
    }
    return status;
}

int define_key(CMDBLK* c)
{
    int status = 0;

    if (strlen(c->arg) == 0) {
        fprintf(stderr,"No key specified.\n");
    }
    else {
        status = binsky(btp,c->arg,c->qual_int);
    }
    return status;
}

int show_debug(CMDBLK* c)
{
    return bdbug(btp,c->arg,c->qual_int);
}

int next_key(CMDBLK* c)
{
    int status;
    BTint val;
    char key[ZKYLEN];
    
    status = bnxtky(btp,key,&val);
    if (status == QNOKEY) {
        printf("No more keys\n");
        status = 0;
    }
    else {
        printf("Key: '%s' = " ZINTFMT "\n",key,val);
    }
    return status;
}

int list_keys(CMDBLK* c)
{
    int status = 0;
    BTint val;
    char key[ZKYLEN];
    
    while (status == 0) {
        status = bnxtky(btp,key,&val);
        if (status == 0) printf("Key: '%s' = " ZINTFMT "\n",key,val);
    }
    return ((status==0||status==QNOKEY)?0:status);
}

int list_keys_only(CMDBLK* c)
{
    int status = 0;
    BTint val;
    char key[ZKYLEN];
    
    while (status == 0) {
        status = bnxtky(btp,key,&val);
        if (status == 0) printf("%s\n",key);
    }
    return ((status==0||status==QNOKEY)?0:status);
}

int remove_key(CMDBLK* c)
{
    int status;
    
    status = bdelky(btp,c->arg); 
    return status;
}

int define_root(CMDBLK* c)
{
    int status;
    
    status = btcrtr(btp,c->arg);
    if (status == QNOKEY) {
        fprintf(stderr,"Can't create root: '%s'\n",c->arg);
        status = 0;
    }
    return status;
}

int change_root(CMDBLK* c)
{
    int status;
    
    status = btchgr(btp,c->arg);
    if (status == QNOKEY) {
        fprintf(stderr,"Can't change root to: '%s'\n",c->arg);
        status = 0;
    }
    return status;
}

int remove_root(CMDBLK* c)
{
    int status;
    
    status = btdelr(btp,c->arg);
    if (status == QNOKEY) {
        fprintf(stderr,"No such root as '%s'\n",c->arg);
        status = 0;
    }
    return status;
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
        fprintf(stderr,"File %s not found; nothing current\n",c->arg);
    }
    return 0;
    
}

int define_data(CMDBLK* c)
{
    int status;
    struct bt_blist *blk;
    
    /* check for use of data block */
    if (c->qualifier[0] == '*') {
        blk = get_data((c->qualifier)+1);
        if (blk != NULL) {
            status = btins(btp,c->arg,blk->bptr,blk->size);
        }
        else {
            fprintf(stderr,"bt: no such data block: %s\n",(c->qualifier)+1);
            status = 0;
        }
    }
    else {
        status = btins(btp,c->arg,c->qualifier,strlen(c->qualifier));
    }
    return status;
}

int find_data(CMDBLK* c)
{
    int status = 0;
    int datasize;
    char* dbuf;
    
    if (strcmp(c->qualifier,"d") == 0) {
        /* determine record size */
        status = btrecs(btp,c->arg,&datasize);
    }
    else {
        /* use default buffer size for limited display */
        datasize = DATABUFSZ;
    }
    if (status == 0) {
        dbuf = (char *) malloc(datasize+1);
        if (dbuf == NULL) {
            fprintf(stderr,"Unable to allocate memory\n");
            status = 0;
        }
        status = btsel(btp,c->arg,dbuf,datasize,&datasize);
        *(dbuf+datasize) = '\0';
        printf("Data record:\n%s\n",dbuf);
        free(dbuf);
    }
    
    if (status == QNOKEY && strlen(c->arg) != 0) {
            fprintf(stderr,"No such key as '%s'\n",c->arg);
    }
    else {
        status = 0;  /* looking for empty string is not an error */
    }
    return status;
}       

int update_data(CMDBLK* c)
{
    int status;
    struct bt_blist *blk;
    
    if ((c->qualifier)[0] == '*') {
        blk = get_data((c->qualifier)+1);
        if (blk != NULL) {
            status = btupd(btp,c->arg,blk->bptr,blk->size);
        }
        else {
            fprintf(stderr,"No such data block: %s\n",(c->qualifier)+1);
            status = 0;
        }
    }
    else {
        status = btupd(btp,c->arg,c->qualifier,strlen(c->qualifier));
    }
    return status;
}

int remove_data(CMDBLK* c)
{
    int status;
    
    status = btdel(btp,c->arg);
    return status;
}

int size_data(CMDBLK* c)
{
    int status, size;
    
    status = btrecs(btp,c->arg,&size);
    if (status == 0) {
        printf("Key '%s' record size: %d bytes\n",c->arg,size);
    }
    return status;
}

int list_data(CMDBLK* c)
{
    char buf[DATABUFSZ];
    char key[ZKYLEN];
    int found = TRUE,
        status = 0,
        size;
    
    while (found && status == 0) {
        status = btseln(btp,key,buf,DATABUFSZ,&size);
        if (status == 0) {
            buf[(size==DATABUFSZ?size-1:size)] = '\0';
            printf("Key: '%s' - Data: '%s'\n",key,buf);
        }
    }
    return status;
}

int next_data(CMDBLK* c)
{
    char buf[DATABUFSZ];
    char key[ZKYLEN];
    int  status,size;
    
    status = btseln(btp,key,buf,DATABUFSZ,&size);
    if (status == 0) {
        buf[(size==DATABUFSZ?size-1:size)] = '\0';
        printf("Key: '%s' - Data: '%s\n",key,buf);
    }
    return status;
}

int lock_file(CMDBLK* c)
{
    return btlock(btp);
}

int unlock_file(CMDBLK* c)
{
    return btunlock(btp);
}

int update_value(CMDBLK* c)
{
    int status;
    
    status = bupdky(btp,c->arg,c->qual_int); /* only int update! */
    return status;
}

int decode_addr(CMDBLK* c)
{
    int status;
    BTint val;
    
    status = bfndky(btp,c->arg,&val);
    if (status == 0) {
        BTint dblk;
        int offset;
        cnvdraddr(val,&dblk,&offset);
        printf("Key: '%s' = " ZINTFMT ", dblk: " ZINTFMT
               ", offset; %d\n",c->arg,val,dblk,offset);
    }
    return status;
}

CMDENTRY bt_cmds[] = {
  { "block","b",create_block,"{b n|b f}",2,"Create data block named b, with either n bytes, or contents of file f." },
  { "block-delete","bd",block_delete,"b",1,"Delete data block named b." },
  { "block-list","bl",block_list,"",0,"List defined data blocks." },
  { "create","c",create_file,"file [s]",1,"Create index file. s qualifier indicates shared mode." },
  { "change-root","cr",change_root,"rootname",1,"Change to named root." },
  { "define","d",define_key,"key [val]",0,"Define key with associated value." },
  { "decode-address","da",decode_addr,"key",1,"Print decoded data segment address for key." },
  { "define-data","dd",define_data,"key {s|*b}",2,"Define key with data.  Specify string s or use *b to refer to previously defined data block." },
  { "define-root","dr",define_root,"root",1,"Define new root." },
  { "find","f",find_key,"key",0,"Find key." },
  { "find-data","fd",find_data,"key [d]",0,"Find key with data. Use d qualifier to display entire data record." },
  { "file-list","fl",file_list,"",0,"List open index files." },
  { "list","l",list_keys,"",0,"List all keys and values following last find operation." },
  { "list-data","ld",list_data,"",0,"List all keys and data following last find operation." },
  { "lock","lk",lock_file,"",0,"Lock current index file." },
  { "list-keys-only","lko",list_keys_only,"",0,"List all keys (only) following last find operation." },
  { "next","n",next_key,"",0,"Display next key and value." },
  { "next-data","nd",next_data,"",0,"Display next key and associated data." },
  { "open","o",open_file,"file [s]",0,"Open existing index file.  s qualifier indicates shared mode." },
  { "quit","q",quit,"",0,"Quit bt program." },
  { "remove","r",remove_key,"key",1,"Remove key." },
  { "remove-data","rd",remove_data,"key",1,"Remove key and associated data." },
  { "remove-root","rr",remove_root,"root",1,"Remove root." },
  { "show","s",show_debug,"arg",0,"Show debug info. arg one of control,super,stats,space,stack,block n." },
  { "size-data","sd",size_data,"key",1,"Display size of data record for key." },
  { "use","u",use_file,"file",1,"Make file current in-use index file." },
  { "update-data","ud",update_data,"key {s|*b}",2,"Update key with new data record string s. Use *b to refer to pre-defined data block." },
  { "unlock","ul",unlock_file,"",0,"Unlock current index file." },
  { "update-value","uv",update_value,"key val",2,"Update value of key to val." },
  { "x","x",close_file,"",0,"Close current index file." },
  { "","",bt_noop,"",0,"END OF COMMANDS"}
};

void report_error(void)
{
    int status,ioerr;
    char name[ZRNAMESZ];
    char msg[ZMSGSZ];

    btcerr(&status,&ioerr,name,msg);
    fprintf(stdout,"(%s) [%d] %s\n",
            name,status,msg);
}
    
int main(int argc,char *argv[])
{
    char *ps = "bt: ";
    int status;
    CMDBLK* c;
    
    if (btinit() != 0) {
        report_error();
    }
    else {
        /* catch interrupts and always return here */
        setjmp(env);
        signal(SIGINT,break_handler);
        fflush(stdout);
    
        /* read command from command stream */
        while (TRUE) {
            c = btcmd(ps,bt_cmds);
            if (c->function == NULL) {
                fprintf(stderr,"btcmd returned NULL function!\n");
            }
            else {
                status = (c->function)(c);
                /* check for error in B tree */
                if (status != 0)  {
                    report_error(); 
                }
            }
        }
        signal(SIGINT,SIG_DFL);
    }
    return EXIT_FAILURE;
}

