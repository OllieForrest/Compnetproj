CC = gcc
CFLAGS = -Wall -pthread -g
LDFLAGS = -ljson-c

all: station-server

station-server: main.o server.o udp.o tcp.o
	$(CC) -o station-server main.o server.o udp.o tcp.o $(LDFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

udp.o: udp.c
	$(CC) $(CFLAGS) -c udp.c

tcp.o: tcp.c
	$(CC) $(CFLAGS) -c tcp.c

clean:
	rm -f *.o station-server
