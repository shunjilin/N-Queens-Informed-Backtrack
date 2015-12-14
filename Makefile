CFLAGS=-c -Wall -g -DNDEBUG
CC = gcc

all: nqueens

nqueens: nqueens.o 
	$(CC) -o nqueens nqueens.o


nqueens.o: nqueens.c  dbg.h 
	$(CC) $(CFLAGS) nqueens.c


clean:
	rm -f nqueens






