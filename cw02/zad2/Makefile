CC=gcc
CFLAGS=-Wall

main: main.c
	$(CC) $(CFLAGS) main.c -o main

run_short: clean main
	echo "------------ short files ---------------------" >> pomiar_zad_2.txt
	./main I file1.txt

run_medium: clean main
	echo "------------ medium files ---------------------" >> pomiar_zad_2.txt
	./main f long7.txt

run_long: clean main
	echo "------------ long files ---------------------" >> pomiar_zad_2.txt
	./main e long77.txt

clean:
	rm -f main

cleanpomiary:
	rm pomiar_zad_2.txt