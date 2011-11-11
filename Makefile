CFLAGS=`pkg-config libnotify --cflags`
LDFLAGS=`pkg-config libnotify --libs`
OPTS=-O2 -DLIBNOTIFY -DINOTIFY

procman: procman.h procman.c process.o
	gcc $(OPTS) procman.c process.o $(CFLAGS) $(LDFLAGS) -o procman

process.o: process.h process.c
	gcc $(OPTS) -c process.c

clean:
	rm -f procman *.o 
