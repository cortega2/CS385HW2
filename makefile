#CS HW2 makefile
CC=gcc
CFLAGS= -Wall

all: master.out worker.out

master.out: master.c worker.h
	gcc master.c -Wall -o master.out

worker.out: worker.c worker.h
	gcc worker.c -Wall -o worker.out
