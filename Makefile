procman: procman.c process.o
	gcc -ggdb procman.c process.o -o procman

process.o: process.h process.c
	gcc -ggdb -c process.c

clean:
	rm -f procman *.o 
