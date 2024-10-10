OBJECTS = server client
CC = gcc
CFLAGS = -Wall -std=c11 -pthread -g

all: $(OBJECTS)

server: server.o chat.o
	$(CC) server.o chat.o -o server $(CFLAGS)

server.o: server.c
	$(CC) -c server.c $(CFLAGS)

client: client.o chat.o
	$(CC) client.o chat.o -o client $(CFLAGS)

client.o: client.c
	$(CC) -c client.c $(CFLAGS)

chat.o:
	$(CC) -c chat.c $(CFLAGS)

.PHONY: clean

clean:
	rm -f chat.o client.o server.o $(OBJECTS)