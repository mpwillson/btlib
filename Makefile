#
# (GNU) Makefile for btree library and test harness
#
# MODIFICATION HISTORY
# Mnemonic	Date	Rel	Who
# BTMAKE	01Feb24	1.0	mpw
#	Created.
# BTMAKE	02Jan29	1.1	mpw
#	Heavily simplified to use implicit rules
# BTMAKE    03Jan14 1.2 mpw
#   Add -Wno-long-long to CFLAGS for use under FreeBSD
# BTMAKE	03Dec12	1.3	mpw
#	Revised dependency handling - use 'make depend' to recreate if necessary
# BTMAKE    04Oct02 1.4 mpw
#   Added compile for big file tester

DEBUG=-g

# Headers in INC_DIR
INC_DIR=./inc
# BT library source in $SRC_DIR
SRC_DIR=./src-lib
# Main programs (test harness, copy utility) in SRC_MAIN
SRC_MAIN=./src-main
# BT archive in LIB_DIR
LIB_DIR=./lib
LIB_FILE=${LIB_DIR}/libbt.a
# Testcases in TESTCASES
TESTCASES=./Testcases
# Computed dependencies in DEP
DEP=.dep

CFLAGS=-pedantic-errors -Wall -Wno-long-long ${DEBUG} -I${INC_DIR}

LIBS=

# activate this macro to ignore ld errors
#LDFLAGS=-Xlinker -noinhibit-exec

SRC := ${wildcard ${SRC_DIR}/*.c}

OBJ := ${patsubst %.c,%.o,${SRC}}

HDR := ${wildcard ${INC_DIR}/*.h}


.PHONY: all clean test_init test_run depend 

.INTERMEDIATE: ${OBJ}

all:	bt kcp TAGS

include 	${DEP}

${LIB_FILE}:	${LIB_FILE}(${OBJ})

bt:	${SRC_MAIN}/bt.c ${LIB_FILE} 
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^

kcp:	${SRC_MAIN}/kcp.c ${LIB_FILE}
	${CC} ${CFLAGS} -o $@ $^

clean:
	rm -f bt bt.exe ${LIB_FILE} kcp kcp.exe .dep

TAGS:	${SRC} ${HDR} ${wildcard ${SRC_MAIN}/*.c}
	@etags $^

depend:
	./depend.sh ${SRC_DIR} ${CFLAGS}  ${SRC} > ${DEP}

.dep: 
	./depend.sh ${SRC_DIR} ${CFLAGS}  ${SRC} > ${DEP}

test_run:
	cd ${TESTCASES};sh test_control.sh
	find ${TESTCASES} -type f -name "test_db" -exec rm  {} \;

test_init:
	cd ${TESTCASES};sh ./create_output_masters.sh
	find ${TESTCASES} -type f -name "test_db" -exec rm  {} \;

# Build big file tester
bigt: ${TESTCASES}/bigt.c ${LIB_FILE}
	${CC} ${CFLAGS} ${LIB_FILE} -o $@ $^

bigtdel:  ${TESTCASES}/bigtdel.c ${LIB_FILE}
	${CC} ${CFLAGS} ${LIB_FILE} -o $@ $^
