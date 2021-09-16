CC=gcc
CFLAGS=-Wall

main: main.c
	$(CC) $(CFLAGS) main.c -o main

run_short: main
	echo "------------ short files ---------------------" >> pomiar_zad_1.txt
	./main file1.txt file2.txt

run_medium: main
	echo "------------ medium files ---------------------" >> pomiar_zad_1.txt
	./main long7.txt long8.txt

run_long: main
	echo "------------ long files ---------------------" >> pomiar_zad_1.txt
	./main long88.txt long77.txt

clean:
	rm -f main

cleanpomiary:
	rm pomiar_zad_1.txt