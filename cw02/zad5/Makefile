CC=gcc
CFLAGS=-Wall

main: main.c
	$(CC) $(CFLAGS) main.c -o main

run_short: clean main
	echo "------------ short files ---------------------" >> pomiar_zad_5.txt
	./main file1.txt output1.txt

run_medium: clean main
	echo "------------ medium files ---------------------" >> pomiar_zad_5.txt
	./main long8.txt output8.txt

run_long: clean main
	echo "------------ long files ---------------------" >> pomiar_zad_5.txt
	./main long88.txt output88.txt

clean:
	rm -f main

cleanpomiary:
	rm pomiar_zad_5.txt

cleanoutput:
	rm output*\.txt