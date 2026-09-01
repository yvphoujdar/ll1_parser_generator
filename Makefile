CC=gcc
CFLAGS=-Wall

all: parser

parser: main.o grammar.o firstfollow.o table.o parser.o
	$(CC) $(CFLAGS) -o parser main.o grammar.o firstfollow.o table.o parser.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

grammar.o: grammar.c grammar.h
	$(CC) $(CFLAGS) -c grammar.c

firstfollow.o: firstfollow.c firstfollow.h
	$(CC) $(CFLAGS) -c firstfollow.c

table.o: table.c table.h
	$(CC) $(CFLAGS) -c table.c

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c parser.c

clean:
	rm -f *.o parser
