CFLAGS = -Wall -pthread -g -I/opt/homebrew/Cellar/json-c/0.17/include
LDFLAGS = -L/opt/homebrew/Cellar/json-c/0.17/lib -ljson-c

all: station-server

station-server: main.o server.o udp.o tcp.o
	gcc $(CFLAGS) -o station-server main.o server.o udp.o tcp.o $(LDFLAGS)

main.o: main.c
	gcc $(CFLAGS) -c main.c

server.o: server.c
	gcc $(CFLAGS) -c server.c

udp.o: udp.c
	gcc $(CFLAGS) -c udp.c

tcp.o: tcp.c
	gcc $(CFLAGS) -c tcp.c

clean:
	rm -f *.o station-server
