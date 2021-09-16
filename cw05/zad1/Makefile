CC=gcc
CFLAGS=-Wall

all: clean main run

main: main.c
	$(CC) $(CFLAGS) main.c -o main

run: main
	./main commands.txt

clean:
	rm -f main