CC=gcc
CFLAGS=-Wall

customer: utils.o customer.o
	$(CC) utils.o customer.o -o customer

server: utils.o server.o
	$(CC) utils.o server.o -o server

customer.o: customer.c
	$(CC) -c -D_GNU_SOURCE $(CFLAGS) customer.c

server.o: server.c
	$(CC) -c $(CFLAGS) server.c

utils.o: utils.c
	$(CC) -c $(CFLAGS) utils.c

run_customer: clean customer
	./customer

run_server: clean server
	./server

clean:
	rm -f customer
	rm -f server
	rm -f *\.o