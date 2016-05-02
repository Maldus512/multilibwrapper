
PREFIX ?= $(PWD)

DINLIBPATH= $(PREFIX)/$(DINLIB)
LIBNAME=$(subst .so,,$(DINLIB))

all: $(LIBNAME)-wrapper.o libthreadwrapper.so

$(LIBNAME)-wrapper.o: $(LIBNAME)-wrapper.c
	$(CC) -c -o $(LIBNAME)-wrapper.o -fPIC -ldl -lpthread $(CFLAGS) -Iinclude $(LIBNAME)-wrapper.c

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

install: libthreadwrapper.so
	cp libthreadwrapper.so /usr/lib
	chmod 755 /usr/lib/libthreadwrapper.so
	ldconfig

uninstall:
	rm /usr/lib/libthreadwrapper.so

.PHONY: clean all purelibc install uninstall
