/*
 *     Standard btree header
 *
 *     Defines all API functions
 *
 */

#include "bt.h"

extern int btcls(BTA *);
extern BTA *btcrt(char *,int, int);
extern int btchgr(BTA *,char *);
extern int btcrtr(BTA *,char *);
extern int btinit(void);
extern BTA *btopn(char *,int,int);
extern int bdbug(BTA *,char *,int);
extern int bfndky(BTA *,char *,int *);
extern int binsky(BTA *,char *,int);
extern int bnxtky(BTA *,char *,int *);
extern int bdelky(BTA *,char *);
extern int btdelr(BTA*,char *);
extern void btcerr(int *,int *,char *,char *);
extern int bupdky(BTA *,char *,int);

extern int btins(BTA *,char *,char *,int);
extern int btsel(BTA *,char *,char *,int,int *);
extern int btupd(BTA *,char *,char *,int);
extern int btdel(BTA *,char *);
extern int btseln(BTA *,char *,char *,int,int *);
extern int btrecs(BTA *,char *,int *);

extern int btlock(BTA *);
extern int btunlock(BTA *);











