makefile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
PREFIX := $(shell dirname $(makefile_path))

CFLAGS+= -g
DINLIB= libcounter.so
HEADERS= $(PREFIX)/counter.h
CFLAGS+= -I$(PREFIX)
WRAPPERFLAGS= -ldl -lpthread
 
all: test

test: main.o $(DINLIB) libcounter-wrapper.o
	$(CC) $(CFLAGS) -o test main.o libcounter-wrapper.o $(WRAPPERFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c 

$(DINLIB): counter.o
	$(CC) $(CFLAGS) -shared -o $(DINLIB) counter.o 

counter.o: counter.c
	$(CC) $(CFLAGS) -fPIC -c counter.c

libcounter-wrapper.o:
	make -C ./multilibwrapper PREFIX=$(PREFIX) HEADERS="$(HEADERS)" DINLIB=$(DINLIB) CFLAGS="$(CFLAGS)"
	cp ./multilibwrapper/libcounter-wrapper.o .
	cp ./multilibwrapper/libcounter-wrapper.h .


clean:
	rm -f test *.so *.o *.a 
	make -C ./multilibwrapper clean DINLIB=$(DINLIB)

.PHONY: clean all
