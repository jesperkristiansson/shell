CC = gcc
IGNOREWARNINGS = -Wno-implicit-fallthrough
CFLAGS = -g -Wall -Wextra -pedantic $(IGNOREWARNINGS)
SRC=../src
SRCFILES=$(SRC)/*.c

debug : $(SRCFILES)
	$(CC) $(CFLAGS) -DDEBUG -o shell $(SRCFILES)

production : $(SRCFILES)
	$(CC) $(CFLAGS)  -o shell $(SRCFILES)