/*
 * $Id: bc.h,v 1.11 2005/09/02 16:45:56 mark Exp $
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

#ifndef _btconst_
#define _btconst_

/*
 *  B tree constants
 */


#define DEBUG 0

#define TRUE 1
#define FALSE 0

/* Implementation defined constants */

/* number of bytes per int */
#define ZBPW 4
/* bits per byte */
#define ZBYTEW 8

/* maximum key size (bytes) */
#define ZKYLEN 32
/* number of in-memory blocks (3 is the minimum) */
#define ZMXBLK  3
/* block size in bytes */
#define ZBLKSZ 1024
/* number of keys per block */
#define ZMXKEY ((ZBLKSZ-ZBPW-ZINFSZ*ZBPW)/(ZKYLEN+2*ZBPW)) 
/* number of pad words required */
#define ZPAD ((ZBLKSZ-(ZKYLEN+2*ZBPW)*ZMXKEY-ZBPW-ZINFSZ*ZBPW)/ZBPW)
/* threshold for block joining */
#define ZTHRES 3
/* number of bt indexes that may be open concurrently */
#define ZMXACT 5

/* End implementation defined constants */

/* Indexes into block info words - first ZINFSZ words in every block */
/*
 * ZBTYPE       ZSUPER      ZROOT           ZINUSE  ZFREE   ZDATA 
 * ZMISC        #free blks  Unused          Unused  Unused  bytes free
 * ZNXBLK       free list   data blk list   Unused  flink   flink 
 * ZNKEYS       # keys      # keys          # keys  Unused  next free byte 
 * ZNBLKS       # blocks    Unused          Unused  Unused  blink
 * 
 */

/* ZVERS must be incremented when structure of B Tree index file
 * changes */

#define ZVERS 0x2

/* ZBTYPE and ZBTVER share information word 0 */
#define ZBTYPE 0
#define ZMISC  1
#define ZNXBLK 2
#define ZNKEYS 3
#define ZNBLKS 4
#define ZBTVER -1

/* max info word index */
#define ZINFSZ 5

/* block type of root */
#define ZROOT 1
/* block type of inuse */
#define ZINUSE 2
/* block type of free */
#define ZFREE 3
/* block type of data */
#define ZDATA 4
/* super block location in index file */
#define ZSUPER 0
/* null block pointer */
#define ZNULL -1

/*  Data block defines:
 *      
 *      Each data record segment is prefixed by ZDOVRH bytes of
 *      information (six bytes for a 32 bit int implementation).
 *      These are used as follows:
 *
 *          Bytes 1 and 2: the size of the data record in bytes
 *          (maximum size of a data record is therefore 65536 bytes).
 *          
 *          Bytes 3-6: data record address of the next segment of this
 *          data record (0 if the last (or only) segment)).
 */

#define ZDRSZ 2
#define ZDOVRH  (ZDRSZ+ZBPW)

/* minimum number of bytes for a segment */
#define ZDSGMN 7    

/* Perform some simple constant consistency checks */

/* Enforce 3 in-memory blocks as minimum
    Need 3 when a root block is split 
*/
#if (ZMXBLK < 3) 
#error Must have minimum of three in-memory blocks
#endif

/* Ensure block joining will take place
*/
#if (ZMXKEY-ZTHRES <= 0)
#error Definition of ZMXKEY and ZTHRES inconsistent
#endif

/*
   Error codes
*/

#define QSRNR    1
#define QCLSIO   2
#define QCRTIO   3
#define QCPBLK   4

#define QWRBLK   5
#define QRDSUP   6
#define QWRSUP   7
#define QOPNIO   8

#define QRDBLK   9
#define QIXOPN  10
#define QSPLIT  11
#define QINFER  12

#define QNOMEM  13
#define QSTKUF  14
#define QSTKOF  15
#define QBLKFL  16

#define QLOCTB  17
#define QSPKEY  18
#define QWRMEM  19
#define QBALSE  20

#define QDELEX  21
#define QDELER  22
#define QDELRP  23
#define QDEMSE  24

#define QDEMSP  25
#define QJNSE   26
#define QNODEF  27
#define QDELCR  28

#define QBADIX  29
#define QNOBTF  30
#define QINERR  31
#define QBADOP  32

#define QNOACT  33
#define QBADAP  34
#define QBUSY   35
#define QNOTOP  36

#define QNOBLK  37
#define QNEGSZ  38
#define QNOTDA  39
#define QNOOPN  40

#define QDLOOP  41
#define QUNLCK  42
#define QLRUER  43
#define QDAERR  44

#define QDNEG   45
#define QDUP    46
#define QNOKEY  47
#define QNOWRT  48

#define QNOTFR  49
#define QBADVR  50
#define QDAOVR  51
#define QF2BIG  52

/*
    To determine a file descriptor for locking, we need to examine the
    FILE struct.
    
    FILE structs vary between systems.  Cygwin and FreeBSD define file
    number as _file.  This is the default and may well cause compile
    failures for other systems.
*/


#ifdef __linux__
    #define FILENO _fileno
#else
    #define FILENO _file
#endif


#endif /* _btconst_ */
