
PREFIX= $(PWD)

DINLIB1= libpicotcp.so
DINLIB2= libcounter.so
HEADERS1+= $(PREFIX)/picotcp/include/pico_stack.h
HEADERS1+= $(PREFIX)/picotcp/build/include/pico_dev_vde.h
HEADERS1+= $(PREFIX)/picotcp/include/pico_socket.h
HEADERS1+= $(PREFIX)/picotcp/build/include/pico_ipv4.h

HEADERS2= $(PREFIX)/dincounter/counter.h

CFLAGS+= -I$(PREFIX)/picotcp/build/include -I$(PREFIX)/picotcp/include -I$(PREFIX)/dincounter -g 

LIBNAME1=$(subst .so,,$(DINLIB1))
LIBNAME2=$(subst .so,,$(DINLIB2))

WRAPPERFLAGS= -L. -ldl -lpthread
 
SRC=picotcp/build/modules/*.c picotcp/build/lib/*.c
OBJS=$(SRC:.c=.o)

all: test

test: $(DINLIB1) $(DINLIB2) libthreadwrapper.so $(LIBNAME1)-wrapper.o $(LIBNAME2)-wrapper.o picowebserver.o  
	$(CC) $(CFLAGS)   -o test picowebserver.o $(LIBNAME1)-wrapper.o $(LIBNAME2)-wrapper.o   $(WRAPPERFLAGS) -lvdeplug       

$(LIBNAME1)-wrapper.o: 
	make -C ./multilibwrapper PREFIX=$(PREFIX) HEADERS="$(HEADERS1)" DINLIB=$(DINLIB1) CFLAGS="$(CFLAGS)"
	cp ./multilibwrapper/$(LIBNAME1)-wrapper.o .
	cp ./multilibwrapper/$(LIBNAME1)-wrapper.h .
	cp ./multilibwrapper/libthreadwrapper.so .

$(LIBNAME2)-wrapper.o:
	make -C ./multilibwrapper PREFIX=$(PREFIX) HEADERS="$(HEADERS2)" DINLIB=$(DINLIB2) CFLAGS="$(CFLAGS)"
	cp ./multilibwrapper/$(LIBNAME2)-wrapper.o .
	cp ./multilibwrapper/$(LIBNAME2)-wrapper.h .
	cp ./multilibwrapper/libthreadwrapper.so .

$(DINLIB1): $(OBJS)
	gcc -shared $(OBJS) -lvdeplug -o $(DINLIB1)

$(OBJS):
	make -C ./picotcp UNIT_TEST=1 ARCH=shared PREFIX=$(PREFIX)/picotcp/build posix

$(DINLIB2):
	make -C ./dincounter
	cp ./dincounter/$(DINLIB2) .

picowebserver.o: picowebserver.c
	$(CC) $(CFLAGS) -c picowebserver.c

clean:
	rm -f test *.o *.a *-wrapper.* *.so
	make -C ./multilibwrapper clean DINLIB=$(DINLIB1)
	make -C ./multilibwrapper clean DINLIB=$(DINLIB2)
	make -C ./dincounter clean

picoclean:
	make -C ./picotcp PREFIX=$(PREFIX)/picotcp/build clean

.PHONY: clean all picoclean
