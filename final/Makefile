# CC = g++
CC = gcc

CFLAGS = -Wall -O2 -I .
LDLIBS = -lpthread

PROGS =	myclient\
	myserver

all: $(PROGS) 
$(PROGS): csapp.o
csapp.o: csapp.c csapp.h

# Programs that need more than one .o file
echoserveri: echoserveri.o echo.o
 
clean:
	rm -f $(PROGS) *.o *~
