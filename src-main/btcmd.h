#ifndef BTCMD_H
#define BTCMD_H

typedef struct cmd_blk CMDBLK;
typedef int(*FUN)(CMDBLK*);

struct cmd_blk {
    char* cmd;          /* command string */
    char* arg;          /* first argument */
    char* qualifier;    /* argument qualifier */
    int qual_int;       /* qualifier int value */
    char* all;          /* all arguments concatenated */
    int nargs;          /* number of args found (<0 if # args
                           mismatch) */
    FUN function;       /* function to call */
};

struct cmd_entry {
    char* cmd;
    char* abbrev;
    FUN function;
    char* args;
    int nargs;
    char* description;
};
typedef struct cmd_entry CMDENTRY;

CMDBLK* btcmd(char*,CMDENTRY[]);
int cmd_help(CMDENTRY[]);
int bt_noop(CMDBLK*);

#endif
