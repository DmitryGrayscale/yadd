CC = gcc
CFLAGS = $(shell neon-config --cflags)
LIBS = $(shell neon-config --libs)

daemon: daemon.o
	$(CC) -o daemon daemon.o $(LIBS)
daemon.o: daemon.c
	$(CC) $(CFLAGS) -c daemon.c -o daemon.o
