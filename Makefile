CC=gcc
CFLAGS=-Wall -g -Wextra
OBJS=main.o
BIN=main

all:$(BIN)

main: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o main

main.o: main.c
	$(CC) -c main.c

clean:
	del *.o main.exe