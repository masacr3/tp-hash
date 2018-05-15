EXEC=hash
cc=gcc
CFLAGS=-g -O2 -std=c99 -Wall -Wconversion -Wno-sign-conversion -Wbad-function-cast -Wshadow -Wpointer-arith -Wunreachable-code -Wformat=2 -Werror
VFLAGS=--leak-check=full --track-origins=yes --show-reachable=yes
CFILES=testing.c hash.c pruebas_catedra.c main.c lista.c
HFILES=hash.h lista.h testing.h

all:
	$(CC) $(CFLAGS) $(CFILES) -o $(EXEC)
run: all
	time -f %e ./$(EXEC)
valgrind: all
	time -f %e valgrind	$(VFLAGS)	./$(EXEC)
