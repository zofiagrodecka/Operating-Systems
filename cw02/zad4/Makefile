CC=gcc
CFLAGS=-c -Wall

all: main

main: main.o function.o
	$(CC) -Wall main.o function.o -o main

main.o:
	$(CC) $(CFLAGS) main.c

function.o:
	$(CC) $(CFLAGS) function.c

run_short: clean main
	echo "------------ short files ---------------------" >> pomiar_zad_4.txt
	./main file1.txt output1.txt I HELLO

run_medium: clean main
	echo "------------ medium files ---------------------" >> pomiar_zad_4.txt
	./main long7.txt output7.txt fire OGIEN

run_long: clean main
	echo "------------ long files ---------------------" >> pomiar_zad_4.txt
	./main long77.txt output77.txt fire OGIEN

clean:
	rm -f main
	rm -f *\.o

cleanpomiary:
	rm pomiar_zad_4.txt

cleanoutput:
	rm output*\.txt