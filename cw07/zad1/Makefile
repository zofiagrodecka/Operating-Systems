CC=gcc
CFLAGS=-c -Wall

all: clean run

main: utils.o main.o cook deliveryman
	$(CC) utils.o main.o -o main

main.o: main.c
	$(CC) $(CFLAGS) main.c

cook.o: cook.c
	$(CC) $(CFLAGS) cook.c

cook: cook.o utils.o
	$(CC) utils.o cook.o -o cook

deliveryman.o: deliveryman.c
	$(CC) $(CFLAGS) deliveryman.c

deliveryman: deliveryman.o utils.o
	$(CC) utils.o deliveryman.o -o deliveryman

utils.o: utils.c
	$(CC) $(CFLAGS) utils.c

run: main
	./main 10 3

clean:
	rm -f main
	rm -f cook
	rm -f deliveryman
	rm -f *\.o
