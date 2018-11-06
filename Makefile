ENABLE_TRACE=0

ifneq ($(ENABLE_TRACE), 0)
TRACE_FLAG=-DENABLE_TRACE=1
else
TRACE_FLAG=
endif

CFLAGS=-O2 -ffunction-sections -fdata-sections -Wl,--gc-sections -pedantic -Wall -Wwrite-strings -Wconversion
OUTNAME=cervii-reloaded-server

all: common.o alist.o bqueue.o main.o
	gcc $(CFLAGS) -o $(OUTNAME) $^ -pthread -lm ../websocket-server/websocket-server.a

%.o : %.c
	gcc $(CFLAGS) $(TRACE_FLAG) -c -o $@ $<

clean:
	rm *.o $(OUTNAME)
