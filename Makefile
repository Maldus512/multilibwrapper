
PREFIX ?= $(PWD)

DINLIBPATH= $(PREFIX)/$(DINLIB)
LIBNAME=$(subst .so,,$(DINLIB))

CFLAGS+=
all: $(LIBNAME)-wrapper.o libthreadwrapper.so

$(LIBNAME)-wrapper.o: $(LIBNAME)-wrapper.c
	$(CC) -c -o $(LIBNAME)-wrapper.o -fPIC $(CFLAGS) -Iinclude $(LIBNAME)-wrapper.c
#TODO: remove the 2 following lines
	cp $(LIBNAME)-wrapper.o ../
	cp $(LIBNAME)-wrapper.h ../

$(LIBNAME)-wrapper.c: multilibparser.py
	python multilibparser.py $(DINLIBPATH) $(HEADERS) 

libthreadwrapper.so: threadwrapper.o
	$(CC) -shared threadwrapper.o -o libthreadwrapper.so

threadwrapper.o:
	$(CC) -c -fPIC $(CFLAGS) -Iinclude threadwrapper.c

clean:
	rm -f test *.o *.a *-wrapper.* *.so

purelibc:
	gcc -c -fPIC purelibctrace.c
	gcc -shared -o purelibctrace.so purelibctrace.o
	echo "export LD_PRELOAD=libpurelibc.so:$(PREFIX)/purelibctrace.so"

.PHONY: clean all purelibc
