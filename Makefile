CFLAGS=`pkg-config libnotify --cflags`
LDFLAGS=`pkg-config libnotify --libs`
OPTS=-O2 -DLIBNOTIFY -DINOTIFY

procman: procman.h procman.c process.o notifications.o
	gcc $(OPTS) procman.c process.o notifications.o $(CFLAGS) $(LDFLAGS) -o procman

process.o: process.h process.c
	gcc $(OPTS) -c process.c $(CFLAGS)
	
notifications.o: notifications.h notifications.c
	gcc $(OPTS) -c notifications.c $(CFLAGS)

clean:
	rm -f procman *.o 
