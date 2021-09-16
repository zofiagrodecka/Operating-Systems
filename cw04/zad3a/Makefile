CC=gcc
CFLAGS=-Wall

all: clean catcher run

catcher: catcher.c sender
	$(CC) $(CFLAGS) catcher.c -o catcher

sender: sender.c
	$(CC) $(CFLAGS) sender.c -o sender

run: catcher
	./catcher 100 KILL

clean:
	rm -f catcher
	rm -f sender