CC=gcc
CFLAGS=-Wall

all: clean main test

main: main.c
	$(CC) $(CFLAGS) main.c -o main -lpthread -lm

run: main
	./main 30 block lena.ascii.pgm negative_image.ascii.pgm

test: main
	echo "------------ WERSJA 1 ---------------------" > Times.txt
	./main 1 numbers pepper.ascii.pgm negative_pepper.ascii.pgm
	./main 2 numbers pepper.ascii.pgm negative_pepper.ascii.pgm
	./main 4 numbers pepper.ascii.pgm negative_pepper.ascii.pgm
	./main 8 numbers pepper.ascii.pgm negative_pepper.ascii.pgm
	./main 16 numbers pepper.ascii.pgm negative_pepper.ascii.pgm
	echo "~*~" >> Times.txt
	./main 1 numbers mona_lisa.ascii.pgm negative_monalisa.ascii.pgm
	./main 2 numbers mona_lisa.ascii.pgm negative_monalisa.ascii.pgm
	./main 4 numbers mona_lisa.ascii.pgm negative_monalisa.ascii.pgm
	./main 8 numbers mona_lisa.ascii.pgm negative_monalisa.ascii.pgm
	./main 16 numbers mona_lisa.ascii.pgm negative_monalisa.ascii.pgm
	echo "~*~" >> Times.txt
	./main 1 numbers lena.ascii.pgm negative_lena.ascii.pgm
	./main 2 numbers lena.ascii.pgm negative_lena.ascii.pgm
	./main 4 numbers lena.ascii.pgm negative_lena.ascii.pgm
	./main 8 numbers lena.ascii.pgm negative_lena.ascii.pgm
	./main 16 numbers lena.ascii.pgm negative_lena.ascii.pgm
	echo " " >> Times.txt
	echo "------------ WERSJA 2 ---------------------" >> Times.txt
	./main 1 block pepper.ascii.pgm negative_pepper.ascii.pgm
	./main 2 block pepper.ascii.pgm negative_pepper.ascii.pgm
	./main 4 block pepper.ascii.pgm negative_pepper.ascii.pgm
	./main 8 block pepper.ascii.pgm negative_pepper.ascii.pgm
	./main 16 block pepper.ascii.pgm negative_pepper.ascii.pgm
	echo "~*~" >> Times.txt
	./main 1 block mona_lisa.ascii.pgm negative_monalisa.ascii.pgm
	./main 2 block mona_lisa.ascii.pgm negative_monalisa.ascii.pgm
	./main 4 block mona_lisa.ascii.pgm negative_monalisa.ascii.pgm
	./main 8 block mona_lisa.ascii.pgm negative_monalisa.ascii.pgm
	./main 16 block mona_lisa.ascii.pgm negative_monalisa.ascii.pgm
	echo "~*~" >> Times.txt
	./main 1 block lena.ascii.pgm negative_lena.ascii.pgm
	./main 2 block lena.ascii.pgm negative_lena.ascii.pgm
	./main 4 block lena.ascii.pgm negative_lena.ascii.pgm
	./main 8 block lena.ascii.pgm negative_lena.ascii.pgm
	./main 16 block lena.ascii.pgm negative_lena.ascii.pgm

clean:
	rm -f main