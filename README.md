# BT Library README File

3rd May, 2024

## CONTENTS
- INTRODUCTION
- UPDATING
- COMPILING
- INSTALLING
- NOTES
- BUGS
- ERROR REPORTING
- RELEASE HISTORY

## INTRODUCTION
The BT library is a set of C language functions that implement a
generalised index file capability, based on the B Tree indexing
scheme. The B Tree was originally described by Bayer and
McCraight. A B tree is a multi-way balanced tree: i.e. there is
more than one key per node, and all leaf nodes are the same
distance from the root node.

The BT functions implement a 'classical' B tree, not one of the
later variants (B* or B+ tree).

The B Tree is stored in a disk file. The file can contain many B
Trees, each of which is referred to by a unique name assigned by
the user (or application). The system allows many such files to
exist.

In order for BT to function efficiently on many hardware
platforms, the important variables relating to disk block size,
maximum number of keys per block, etc. are defined as constants,
and may be modified prior to compilation (see inc/bc.h).

BT supports large files (> 2GB), if built with the Large File
Support (LFS) option.

The BT library has been tested on GNU/Linux (Debian Bookworm),
OpenBSD 7.5 and FreeBSD 14.0.

## UPDATING
The BT Recovery program (btr) enables the migration of btree
index files created in earlier versions.

If migrating from a pre-3.0 release of the BT library, be aware
that the function btcerr always zero-pads the srname and msg
buffers to BT size constants `ZRNAMESZ` and `ZMSGSZ`
respectively. Declaring these arrays to be smaller than the BT
constants will ensure btcerr acts as a very effective stack
smasher.

## COMPILING
First, clone this repository or extract from from the release tarball.

The key sub-directories under the bt source directory are as follows:

    src-lib             C source for BT library routines
    src-main            C source for BT test harness and copy utility
    inc                 C header files
    lib                 holds BT library archive
    samples             simple programs showing library API usage
    Testcases           driver and master output files for testing
    doc                 API documentation (AsciiDoc and HTML)

The BT library, a test harness (bt), additional testing tools for
large files (bigt and bigtdel), recovery tool (btr) and a copy
utility (kcp) are built by GNU make.  The test harness, bt, will
be built with readline support, if the make file detects support
on the build platform. To build the BT library and associated
programs for up to 2GiB file support, cd to the BT source
directory and issue the following command:

    make clean && make

To build the LFS version of BT (supports files >2GiB), issue the
following command:

    make clean && make LFS=1

The BT library is a standard UNIX object archive, built by ar.

It is possible to smoke test the newly built library by:

    make test_run

All tests should pass.  Note that the last test (bigdata) will
consume some 2.4GB of disk space and take some time, if the LFS
version of BT is built.

If you need to rebuild, issue the command "make clean && make".

## INSTALLING
Copy the BT library archive (./lib/libbt.a) to a convenient system
library location (e.g. /usr/local/lib).  The standard BT header
files (./inc/{bc.h,bt.h,btree.h}) must also be placed in a local
system include directory (e.g. /usr/local/include).

Programs can then be compiled and linked against the library by a
command of the following form (NB: the define for
`_FILE_OFFSET_BITS` may be required when using a BT library built
with Large File Support):

    cc -o yak -lbt yak.c [-D_FILE_OFFSET_BITS=64]

Alternatively, just put the include and library files somewhere
convenient in your local development hierarchy and use matching
compiler arguments.

## NOTES
Documentation on the BT API is available in AsciiDoc format and
HTML, also downloadable from https://hydrus.org.uk.

BT is dependent on very little, but it does require a C compiler
and GNU make.

## BUGS
On all platforms, shared file handling is clunky.

## ERROR REPORTING
Should you encounter errors during use of the BT library, please
email cdr.nil@gmail.com with relevant details.

## LICENCE
Copyright (C) 2003, 2004, 2005, 2008, 2010, 2011, 2012,
2020, 2024.  Mark Willson

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.

## RELEASE HISTORY
	15th December,  2003        1.0     First public release
	31st May,       2004        2.0     Major API revision
	 8th September, 2004        2.0.1   Bug fix release
	 5th October,   2004        2.0.2   Bug fix release
	28th December,  2005        2.0.3   Bug fix release
	10th May,       2008        2.0.4   Minor bug fixes/OpenSolaris tested
	 4th June,      2010        3.0.0   Large file support (> 2GiB)
	 2nd July,      2010        3.0.1   Added readline support; bug fixes
	10th December,  2010        3.1.0   Support duplicate keys; previous key
	21st December,  2010        3.1.1   Fix broken test scripts
	 3rd January,   2011        3.1.2   Shared mode/duplicate key fixes
	24th June,      2011        4.0.0   Added recovery capabilities
	26th November,  2012        5.0.0   Duplicate key handling re-written
	 4th July,      2020        5.0.1   Bug fixes; accomodate toolchain mods
	 3rd May,       2024        5.0.2   Documentation converted to AsciiDoc
