CC = gcc
IGNOREWARNINGS = -Wno-implicit-fallthrough
CFLAGS = -g -Wall -Wextra -pedantic $(IGNOREWARNINGS)
SRC=../src
SRCFILES=$(SRC)/*.c

build : $(SRCFILES)
	$(CC) $(CFLAGS) -o shell $(SRCFILES)