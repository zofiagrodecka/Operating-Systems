CC=gcc
CFLAGS=-c -Wall

all: static shared dynamic

static: main.o mergelib.a
	$(CC) main.o ../zad1/mergelib.a -o main_static

shared: main.o libmergelib.so
	$(CC) main.o -l mergelib -L ../zad1 -o main_shared -Wl,-rpath=../zad1

dynamic: maindyn.o libmergelib.so
	$(CC) maindyn.o -l mergelib -L ../zad1 -o main_dyn -Wl,-rpath=../zad1 -ldl

mergelib.a:
libmergelib.so:
	make -C ../zad1 all

main.o:
	$(CC) $(CFLAGS) ../zad2/main.c

maindyn.o:
	$(CC) $(CFLAGS) ../zad2/maindyn.c

clean:
	rm -f *o main_static main_shared main_dyn

run: static shared
	echo "------------ STATIC library ---------------------" > results3a.txt
	./main_static create_table 10 merge_files ../zad2/long7.txt:../zad2/long8.txt ../zad2/long1.txt:../zad2/long2.txt remove_row 1 3 remove_block 1 merge_files ../zad2/long7200.txt:../zad2/long8200.txt ../zad2/long10.txt:../zad2/long7200.txt remove_row 3 21124 remove_block 2 merge_files ../zad2/long77.txt:../zad2/long88.txt ../zad2/long88.txt:../zad2/long77.txt remove_row 4 345125 remove_block 5
	cat ../zad3a/raport2.txt >> results3a.txt
	echo "------------ SHARED library ---------------------" >> results3a.txt
	./main_shared create_table 10 merge_files ../zad2/long7.txt:../zad2/long8.txt ../zad2/long1.txt:../zad2/long2.txt remove_row 1 3 remove_block 1 merge_files ../zad2/long7200.txt:../zad2/long8200.txt ../zad2/long10.txt:../zad2/long7200.txt remove_row 3 21124 remove_block 2 merge_files ../zad2/long77.txt:../zad2/long88.txt ../zad2/long88.txt:../zad2/long77.txt remove_row 4 345125 remove_block 5
	cat ../zad3a/raport2.txt >> results3a.txt
	echo "------------ DYNAMIC library ---------------------" >> results3a.txt
	./main_dyn create_table 10 merge_files ../zad2/long7.txt:../zad2/long8.txt ../zad2/long1.txt:../zad2/long2.txt remove_row 1 3 remove_block 1 merge_files ../zad2/long7200.txt:../zad2/long8200.txt ../zad2/long10.txt:../zad2/long7200.txt remove_row 3 21124 remove_block 2 merge_files ../zad2/long77.txt:../zad2/long88.txt ../zad2/long88.txt:../zad2/long77.txt remove_row 4 345125 remove_block 5
	cat ../zad3a/raport2.txt >> results3a.txt
