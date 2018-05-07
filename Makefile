EXEC=hash
cc=gcc
CFLAGS=-g -std=c99 -Wall -Wconversion -Wtype-limits -pedantic -Werror
VFLAGS=--leak-check=full --track-origins=yes --show-reachable=yes
CFILES=testing.c hash.c pruebas_catedra.c main.c lista.c
HFILES=hash.h lista.h testing.h

all:
	$(CC) $(CFLAGS) $(CFILES) -o $(EXEC)
run: all
	./$(EXEC) 500
valgrind: all
	valgrind	$(VFLAGS)	./$(EXEC)
