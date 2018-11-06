CFLAGS=-O2 -pedantic -Wall -Wwrite-strings -Wconversion -Wno-unused-function -DENABLE_TRACE=0

cervii-reloaded-server: phony
	gcc $(CFLAGS) -o $@ $@.c -lm -lpthread ../websocket-server/websocket-server.a
phony:
