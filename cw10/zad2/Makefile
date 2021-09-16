CC=gcc
CFLAGS=-Wall

server: server.c
	$(CC) $(CFLAGS) server.c -o server -lpthread

customer: customer.c
	$(CC) $(CFLAGS) customer.c -o customer -lpthread

run_local_customer: clean customer
	./customer customer1 local server_socket

run_net_customer: clean customer
	./customer customer2 network 127.0.0.1 32800

run_net_customer1: clean customer
	./customer customer3 network 127.0.0.1 32800

run_net_customer2: clean customer
	./customer customer4 network 127.0.0.1 32800

run_net_customer3: clean customer
	./customer customer5 network 127.0.0.1 32800

run_server: clean server
	./server 32800 server_socket

clean:
	rm -f customer
	rm -f server
	rm -f customer.o
	rm -f server.o