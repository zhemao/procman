CFLAGS=`pkg-config libnotify --cflags`
LDFLAGS=`pkg-config libnotify --libs`

procman: procman.h procman.c process.o
	gcc -ggdb procman.c process.o $(CFLAGS) $(LDFLAGS) -o procman

process.o: process.h process.c
	gcc -ggdb -c process.c

clean:
	rm -f procman *.o 
