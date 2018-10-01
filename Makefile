CFLAGS=-O2 -ffunction-sections -fdata-sections -Wl,--gc-sections -pedantic -Wall -Wwrite-strings -Wconversion
OUTNAME=cervii-reloaded-server

all: common.o llist.o bqueue.o main.o
	gcc $(CFLAGS) -o $(OUTNAME) $^ -pthread -lm ../websocket-server/websocket-server.a

%.o : %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm *.o $(OUTNAME)
