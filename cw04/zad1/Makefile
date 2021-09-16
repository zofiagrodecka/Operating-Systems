CC=gcc
CFLAGS=-Wall

all: clean main run

main: main.c execmain
	$(CC) $(CFLAGS) main.c -o main

execmain: execmain.c
	$(CC) $(CFLAGS) execmain.c -o execmain

run: main
	./main pending

clean:
	rm -f main
	rm -f execmain