CC=gcc
CFLAGS=-c -Wall
#CFLAGS=-c -Wall -O1
#CFLAGS=-c -Wall -O2
#CFLAGS=-c -Wall -O3

all: clean static shared

shared: libmergelib.o
	$(CC) -shared -Wl,-soname,libmergelib.so.1 -o libmergelib.so.1.0 libmergelib.o
	rm -f libmergelib.so.1
	ln -s libmergelib.so.1.0 libmergelib.so.1
	rm -f libmergelib.so
	ln -s libmergelib.so.1 libmergelib.so

libmergelib.o: mergelib.c
	$(CC) -fPIC -c mergelib.c -o libmergelib.o

static: mergelib.o
	ar rcs mergelib.a mergelib.o

mergelib.o: mergelib.c
	$(CC) $(CFLAGS) mergelib.c

clean:
	rm -f *o mergelib.a lib*\.so\.*
