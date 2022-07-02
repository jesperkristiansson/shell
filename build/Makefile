CC = gcc
CFLAGS = -g -Wall
SRC=../src

production : $(SRC)/shell.c
	$(CC) $(CFLAGS) -o shell $(SRC)/shell.c

debug : $(SRC)/shell.c
	$(CC) $(CFLAGS) -DDEBUG -o shell $(SRC)/shell.c