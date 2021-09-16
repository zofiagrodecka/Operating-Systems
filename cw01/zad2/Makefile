CC=gcc
CFLAGS=-c -Wall

all: main run

main: main.o mergelib.a
	$(CC) main.o ../zad1/mergelib.a -o main

mergelib.a:
#	make -C ../zad1 clean
	make -C ../zad1 all

main.o: main.c
	$(CC) $(CFLAGS) main.c

clean:
	rm -f *o main

run:
	./main create_table 10 merge_files long77.txt:long88.txt long1.txt:long2.txt remove_row 1 3 merge_files long5.txt:long88.txt long7.txt:long8.txt remove_block 1
