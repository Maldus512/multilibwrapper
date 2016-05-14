makefile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
PREFIX := $(shell dirname $(makefile_path))

CFLAGS+= -g
DINLIB= libcounter.so
HEADERS= $(PREFIX)/counter.h
CFLAGS+= -I$(PREFIX)
WRAPPERFLAGS= -ldl -lpthread
 
all: test

test: $(DINLIB) libcounter-wrapper.o main.o 
	$(CC) $(CFLAGS) -o test main.o libcounter-wrapper.o $(WRAPPERFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c 

$(DINLIB): counter.o
	$(CC) $(CFLAGS) -shared -o $(DINLIB) counter.o 

counter.o: counter.c
	$(CC) $(CFLAGS) -fPIC -c counter.c

libcounter-wrapper.o:
	mlgen --verbose $(DINLIB) $(HEADERS) || (cp ./multilibwrapper/mlgen . && ./mlgen --verbose $(DINLIB) $(HEADERS))

clean:
	rm -f test *.so *.o *.a *-wrapper.* mlgen

.PHONY: clean all
