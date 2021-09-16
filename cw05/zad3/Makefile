CC=gcc
CFLAGS=-Wall

all: clean main run

main: main.c producer consumer
	$(CC) $(CFLAGS) main.c -o main

consumer: consumer.c
	$(CC) $(CFLAGS) consumer.c -o consumer

producer: producer.c
	$(CC) $(CFLAGS) producer.c -o producer

run_producer1: producer
	./producer fifo1 1 file1.txt 5

run_producer2: producer
	./producer fifo1 2 file2.txt 5

run_consumer: consumer
	./consumer fifo1 res1.txt 5

run: main
	./main /tmp/fifo

clean:
	rm -f producer
	rm -f consumer
	rm -f main