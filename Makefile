CFLAGS=-O2 -Wall -pedantic
OUTNAME=cervii-reloaded-server

all: main.o
	gcc $(CFLAGS) -o $(OUTNAME) $^ -pthread ../websocket-server/websocket-server.a

%.o : %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm *.o $(OUTNAME)
