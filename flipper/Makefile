objects = flipper

CC = gcc
CFLAGS = -Wall -std=c11 -g

all: $(objects)

flipper: flipper.o
	$(CC) flipper.o -o flipper

flipper.o: flipper.c
	$(CC) -c flipper.c $(CFLAGS)

.PHONY: clean

clean:
	rm -fdr flipper.o $(objects) flipped/


