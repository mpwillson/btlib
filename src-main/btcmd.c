/*
 * $$
 * 
 * =====================================================================
 * Simple parser for BT test harness
 * =====================================================================
 *
 * Copyright (C) 2010 Mark Willson.
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
 *
 * 	NAME
 *      btcmd - parses commands for bt lib test harness
 *
 * 	SYNOPSIS
 *
 *
 * 	DESCRIPTION
 *
 * 	NOTES
 *
 *
 * 	MODIFICATION HISTORY
 * 	Mnemonic	Rel	Date	Who
 *
 * 		Written.
 *
 */

#define TRUE 1
#define FALSE 0

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h> 
#include <string.h>

#ifdef READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "btcmd.h"

#define MAXBUFSZ 132
#define MAXDESCSZ 512
#define BTEOF "$EOF$"

int issue_prompt = TRUE;
int svp;
FILE* input = NULL;

/* Storage locations for command and argument tokens */
CMDBLK cblk;
char svcmd[MAXBUFSZ+1];
char cmd[MAXBUFSZ+1];
char arg[MAXBUFSZ+1];
char qual[MAXBUFSZ+1];
char all[MAXBUFSZ+1];

/* forward declaration of local_cmds (for help) */
CMDENTRY local_cmds[];
/* Pointer to current applicaton command set (for help) */
CMDENTRY* current_app_cmds;

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

/* Return char pointer to first first space or tab character before
 * the character position max in string s */
char* strbrk(char* s, int max)
{
    char* p = s;
    int len = strlen(s);

    if (len < max) {
        return s+len;
    }
    else {
        p += max;
        while (p != s && *p != ' ' && *p != '\t') p--;
        return (p==s?s+max:p);
    }
}

void display_help(CMDENTRY cmds[])
{
    const int maxdescwidth = 47;
    int i;
    char desc[MAXDESCSZ+1];
    char* cp;
    char* ed;

    fprintf(stdout,"%-20s %-10s %s\n","Command,Abbrev","Args","Description");
    for ( i=0 ; strlen(cmds[i].cmd) != 0 ; i++ ) {
        char s[MAXBUFSZ+1];
        /* ignore commands with no description (probably hidden) */
        if (strlen(cmds[i].description) == 0) continue;
        snprintf(s,MAXBUFSZ,"%s,%s",cmds[i].cmd,cmds[i].abbrev);
        strncpy(desc,cmds[i].description,MAXDESCSZ);
        ed = desc+strlen(desc);
        cp = strbrk(desc,maxdescwidth);
        *cp = '\0';cp++;
        fprintf(stdout,"%-20s %-10s %s\n",s,cmds[i].args,desc);
        while (cp < ed) {
            char* scp = cp;
            cp = strbrk(scp,maxdescwidth);
            *cp = '\0'; cp++;
            fprintf(stdout,"%20s %10s %s\n","","",scp);
        }
    }
}

int cmd_help(CMDENTRY cmds[])
{
    fprintf(stdout,"Application Commands:\n");
    display_help(cmds);
    fprintf(stdout,"\nInbuilt Commands:\n");
    display_help(local_cmds);
    return 0;
}

/* Define btcmd 'free' commands */

/* does nothing */
int bt_noop(CMDBLK* c) {
    return 0;
}

int prompt(CMDBLK* c)
{
    issue_prompt = !issue_prompt;
    return 0;
}

int execute(CMDBLK* c)
{
    if (!pushcf(input)) {
        fprintf(stderr,"command file stack exhausted at: %s\n",
                c->arg);
    }
    else {
        if (input == stdin) {
            svp = issue_prompt;
            issue_prompt = FALSE;
        }
        input = fopen(c->arg,"rt");
        if (input == NULL) {
            printf("unable to open execute file: %s\n",c->arg);
            input = pullcf();
            if (input == stdin) {
                issue_prompt = svp;
            }
        }
    }
    return 0;
}

int close_execute(CMDBLK* c)
{
    fclose(input);
    input = pullcf();
    if (input == NULL) {
        fprintf(stderr,"command file stack underflow\n");
        input = stdin;
    }
    else if (input == stdin) {
        issue_prompt = svp;
    }
    return 0;
}

int btsystem(CMDBLK* c)
{
    system(c->all);
    return 0;
}

int comment(CMDBLK* c)
{
    return 0;
}

int help(CMDBLK* c)
{
    cmd_help(current_app_cmds);
    return 0;
}

CMDENTRY local_cmds[] = {
    { "comment","#",comment,"string",0,"Following text will be ignored."},
    { "execute","e",execute,"filename",1,"Commence reading commands from "
      "file. execute commands may be nested."},
    { "help","?",help,"",0,"Provide help on supported commands."},
    { "prompt","p",prompt,"",0,"Toggle prompting before reading command."},
    { "system","!",btsystem,"string",0,"Run shell command."},
    { BTEOF,"",close_execute,"",0,""},
    { "","",bt_noop,"END OF COMMANDS"}
};

char* non_ws(char* str){
    char* cp = str;

    while (*cp == ' ' || *cp == '\t') cp++;
    return cp;
}


int tty_input(FILE* input)
{
    struct stat statbuf;
    
    if (fstat(fileno(input),&statbuf) == 0) {
        return ((statbuf.st_mode & S_IFMT) == S_IFCHR); /* character
                                                           special */
     }
    else {
        fprintf(stderr,"unable to fstat file descriptor: %d\n",
                fileno(input));
        exit(EXIT_FAILURE);
    }
}

void rl_history(FILE* input,char* cmd)
{
#ifdef READLINE
    if (tty_input(input)) {
        add_history(cmd);
    }
#endif            
    return;
}

int tokenise(char* cmdbuf, char* cmd, char* arg, char* qual, char* all)
{
    char copybuf[MAXBUFSZ+1];
    char* cp;
    int nargs = 0;
    
    strncpy(copybuf,cmdbuf,MAXBUFSZ);
    cmd[0] = '\0';
    arg[0] = '\0';
    qual[0] = '\0';
    all[0] = '\0';
    
    if ((cp = strtok(copybuf," \n")) != NULL) {
        strncpy(cmd,cp,MAXBUFSZ);
        if ((cp = strtok(NULL,"\n")) != NULL) {
            strncpy(all,cp,MAXBUFSZ);
            /* now we've got all the command args, retokenise for arg
             * and qualifier */
            strncpy(copybuf,all,MAXBUFSZ);
            if ((cp = strtok(copybuf," \n")) != NULL) {
                strncpy(arg,cp,MAXBUFSZ);
                nargs++;
                if ((cp = strtok(NULL," \n")) != NULL) {
                    strncpy(qual,cp,MAXBUFSZ);
                    nargs++;
                }
            }
        }
    }
    return nargs;
}
void bad_args(char* s)
{
    fprintf(stderr,"%s: incorrect number of arguments.\n",s);
    return;
}

void find_cmd(char* cmdbuf,CMDENTRY cmds[])
{
    int i;
    char* cp;
    
    cp = non_ws(cmdbuf); /* locate first non-whitespace character */

    /* check for special command (one chararacter, not alphanumeric) */
    for ( i=0; strlen(cmds[i].cmd) != 0 ; i++ ) {
        if (strlen(cmds[i].abbrev) == 1 &&
            cmds[i].abbrev[0] < 'A') {
            if (*cp == cmds[i].abbrev[0]) {
                cblk.function = cmds[i].function;
                cblk.cmd = cmds[i].cmd;
                cblk.arg = "";
                cblk.qualifier = "";
                cblk.all = cp+1;
                cblk.nargs = 0;
                cblk.qual_int = 0;
                return;
            }
        }
    }
    /* now normal commands */
    cblk.nargs = tokenise(cmdbuf,cmd,arg,qual,all);
    cblk.arg = arg;
    cblk.qualifier = qual;
    cblk.qual_int = atoi(qual);
    cblk.all = all;
    cblk.cmd = cmd;
    cblk.function = NULL;
    for ( i=0 ; strlen(cmds[i].cmd) != 0; i++ ) {
        if (strcmp(cmd,cmds[i].cmd) == 0 || strcmp(cmd,cmds[i].abbrev) == 0) {
            cblk.function = cmds[i].function;
            cblk.cmd = cmds[i].cmd;
            /* does # args match that required? */
            if (cmds[i].nargs != 0 &&
                cmds[i].nargs != cblk.nargs) {
                    cblk.nargs = -1;
            }
            return;
        }
    }
    /* no valid command found, return cblk.function as NULL */
    return;
}
    
CMDBLK* btcmd(char* prompt_string,CMDENTRY app_cmds[])
{
    char* rlbuf = NULL;
    char* cp = NULL;
    static char cmdbuf[MAXBUFSZ+1];

    if (input == NULL) input = stdin;
    current_app_cmds = app_cmds;
    cblk.function = NULL;
    
    while (cblk.function == NULL) {
        if (rlbuf != NULL) {
            free(rlbuf);
            rlbuf = NULL;
        }
        if (tty_input(input)) {
#ifdef READLINE
            if (issue_prompt) 
                rlbuf = readline(prompt_string);
            else    
                rlbuf = readline(NULL);
            if (strcmp(rlbuf,"") == 0) continue;
            strncpy(cmdbuf,rlbuf,MAXBUFSZ);
#else
            if (issue_prompt) printf("%s",prompt_string);
            cp = fgets(cmdbuf,MAXBUFSZ,input);
#endif            
        }
        else {
            cp = fgets(cmdbuf,MAXBUFSZ,input);
        }
        if (cp == NULL && rlbuf == NULL) {
            if (input == stdin) {
                return NULL; /* end of file in tty command stream */
            }
            else {
                /* end-of-file on command file - insert peusdo command */
                strcpy(cmdbuf,BTEOF);
            }
        }
        find_cmd(cmdbuf,local_cmds);
        if (cblk.function != NULL) {
            if (cblk.nargs < 0) {
                bad_args(cblk.cmd);
            }
            else {
                rl_history(input,rlbuf);
                (cblk.function)(&cblk);
            }
            cblk.function = NULL;
        }
        else {
            find_cmd(cmdbuf,app_cmds);
            if (cblk.function != NULL) {
                if (cblk.nargs < 0) {
                    bad_args(cblk.cmd);
                    cblk.function = NULL;
                }
                else {
                    rl_history(input,rlbuf);
                }
            }
            else {
                fprintf(stderr,"unknown command: %s - type ? for help.\n",
                        cblk.cmd);
                cblk.function = NULL;
            }
        }
    }   
    return &cblk;
}


