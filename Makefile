

all: libthreadwrapper.so mlgen

libthreadwrapper.so: threadwrapper.o
	$(CC) -shared threadwrapper.o -o libthreadwrapper.so

threadwrapper.o:
	$(CC) -c -fPIC $(CFLAGS) -Iinclude threadwrapper.c

mlgen: mlgen.c
	$(CC) mlgen.c -g -o mlgen

clean:
	rm -f test *.o *.a *.so mlgen

install: libthreadwrapper.so mlgen
	cp libthreadwrapper.so /usr/lib
	chmod 755 /usr/lib/libthreadwrapper.so
	ldconfig
	mkdir -p /usr/share/multilibwrapper
	cp -r templates /usr/share/multilibwrapper
	install multilibparser /usr/share/multilibwrapper
	cp -r include /usr/share/multilibwrapper
	install Makefile_script /usr/share/multilibwrapper/
	install mlgen /usr/local/bin
	chmod u+s /usr/local/bin/mlgen

uninstall:
	rm /usr/lib/libthreadwrapper.so
	rm -r /usr/share/multilibwrapper
	rm /usr/local/bin/mlgen

.PHONY: clean all install uninstall
