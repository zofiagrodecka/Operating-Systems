CC=gcc
CFLAGS=-Wall

all: clean main run

main: main.c
	$(CC) $(CFLAGS) main.c -o main

run: main
	./main 10

clean:
	rm -f main
