
PREFIX ?= $(PWD)

DINLIBPATH= $(PREFIX)/$(DINLIB)
LIBNAME=$(subst .so,,$(DINLIB))

CFLAGS+=
all: $(LIBNAME)-wrapper.o libthreadwrapper.so

$(LIBNAME)-wrapper.o: $(LIBNAME)-wrapper.c
	$(CC) -c -o $(LIBNAME)-wrapper.o -fPIC $(CFLAGS) -Iinclude $(LIBNAME)-wrapper.c

$(LIBNAME)-wrapper.c: multilibparser.py
	python multilibparser.py $(DINLIBPATH) $(HEADERS) 

libthreadwrapper.so: threadwrapper.o
	$(CC) -shared threadwrapper.o -o libthreadwrapper.so

threadwrapper.o:
	$(CC) -c -fPIC $(CFLAGS) -Iinclude threadwrapper.c

clean:
	rm -f test *.o *.a *-wrapper.* *.so

.PHONY: clean all
