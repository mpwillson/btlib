# Simple makefile for BT Library sample programs

all: btcopy btsort

btcopy: btcopy.c
	${CC} -o $@ btcopy.c -L../lib -lbt -I../inc

btsort: btsort.c
	${CC} -o $@ btsort.c -L../lib -lbt -I../inc
