/*
 *     Standard btree header
 *
 *     Defines all API functions
 *
 */

#include "bt.h"

extern int btcls(BTA *);
extern BTA *btcrt(char *,int, int);
extern int btchgr(BTA *,char *,int *);
extern int btcrtr(BTA *,char *,int *);
extern int btinit(void);
extern BTA *btopn(char *,int,int);
extern int bdbug(BTA *,char *,int);
extern int bfndky(BTA *,char *,int *, int *);
extern int binsky(BTA *,char *,int,int *);
extern int bnxtky(BTA *,char *,int *,int *);
extern int bdelky(BTA *,char *,int *);
extern int btdelr(BTA*,char *,int *);
extern void btcerr(int *,int *,char *,char *);
extern int bupdky(BTA *,char *,int,int *);

extern int btins(BTA *,char *,char *,int,int *);
extern int btsel(BTA *,char *,char *,int,int *,int *);
extern int btupd(BTA *,char *,char *,int,int *);
extern int btdel(BTA *,char *,int *);
extern int btseln(BTA *,char *,char *,int,int *,int *);
extern int btrecs(BTA *,char *,int *,int *);

extern int btlock(BTA *);
extern int btunlock(BTA *);











