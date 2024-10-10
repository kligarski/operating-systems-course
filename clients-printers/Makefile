OBJECTS = initialize_queue client printer

CC = gcc
CFLAGS = -lrt -pthread -Wall

all: $(OBJECTS)

initialize_queue: initialize_queue.o printers.o
	$(CC) initialize_queue.o printers.o $(CFLAGS) -o initialize_queue

initialize_queue.o: initialize_queue.c
	$(CC) -c initialize_queue.c $(CFLAGS)

client: client.o printers.o
	$(CC) client.o printers.o $(CFLAGS) -o client

client.o: client.c
	$(CC) -c client.c $(CFLAGS)

printer: printer.o printers.o
	$(CC) printer.o printers.o $(CFLAGS) -o printer

printer.o: printer.c
	$(CC) -c printer.c $(CFLAGS)

printers.o: printers.c
	$(CC) -c printers.c $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJECTS) initialize_queue.o client.o printer.o printers.o