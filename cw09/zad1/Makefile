CC=gcc
CFLAGS=-Wall

all: clean main run

main: main.c
	$(CC) $(CFLAGS) main.c -o main -lpthread

run: main
	./main

clean:
	rm -f main