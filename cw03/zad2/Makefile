CC=gcc
CFLAGS=-c -Wall

all: clean main run

main: main.o mergelib.a
	$(CC) main.o mergelib.a -o main

mergelib.a: mergelib.o
	ar rcs mergelib.a mergelib.o

mergelib.o: mergelib.c
	$(CC) $(CFLAGS) mergelib.c

main.o: main.c
	$(CC) $(CFLAGS) main.c

run: main
	./main create_table 10 merge_files long7.txt:long8.txt long1.txt:long2.txt long7200.txt:long8200.txt long10.txt:long7200.txt long77.txt:long88.txt long88.txt:long77.txt

clean:
	rm -f mergelib.a
	rm -f main
	rm -f *\.o

cleanmerged:
	rm merged*\.txt
