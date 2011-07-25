CFLAGS=`pkg-config libnotify --cflags`
LDFLAGS=`pkg-config libnotify --libs`
OPTIONS=-O2 -DLIBNOTIFY

procman: procman.h procman.c process.o
	gcc $(OPTIONS) procman.c process.o $(CFLAGS) $(LDFLAGS) -o procman

process.o: process.h process.c
	gcc $(OPTIONS) -c process.c

clean:
	rm -f procman *.o 
