CC=gcc
CFLAGS=-c -Wall

all: run

main: main.o functions.o
	$(CC) -Wall main.o functions.o -o main

main.o:
	$(CC) $(CFLAGS) main.c

functions.o:
	$(CC) $(CFLAGS) functions.c

run: clean main
	./main dane.txt

clean:
	rm -f main
	rm -f *\.o

cleanpomiary:
	rm pomiar_zad_3.txt

cleanabc:
	rm a.txt
	rm b.txt
	rm c.txt