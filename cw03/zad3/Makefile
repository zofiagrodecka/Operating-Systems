CC=gcc
CFLAGS=-c -Wall

all: clean main run

main: main.o dirsearching.o
	$(CC) -Wall main.o dirsearching.o -o main

main.o:
	$(CC) $(CFLAGS) main.c

dirsearching.o:
	$(CC) $(CFLAGS) dirsearching.c

run:
	./main Test fire 3

clean:
	rm -f main
	rm -f *\.o