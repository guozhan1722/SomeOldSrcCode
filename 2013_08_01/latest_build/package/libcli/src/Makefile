DESTDIR =
PREFIX = /usr/local

MAJOR = 1
MINOR = 9
REVISION = 3
LIB = libcli.so

LIBS = -lcrypt -lc -lm -luci -lmserial

CC = gcc
DEBUG = -g
OPTIM = -O3
CFLAGS += $(DEBUG) $(OPTIM) -Wall -Wformat-security -Wno-format-zero-length
LIBPATH += -L.

all: $(LIB) clitest

$(LIB): libcli.o
	$(CC) -o $(LIB).$(MAJOR).$(MINOR).$(REVISION) $^ $(LDFLAGS) -shared -Wl,-soname,$(LIB).$(MAJOR).$(MINOR) $(LIBS) 
	-rm -f $(LIB) $(LIB).$(MAJOR).$(MINOR)
	ln -s $(LIB).$(MAJOR).$(MINOR).$(REVISION) $(LIB).$(MAJOR).$(MINOR)
	ln -s $(LIB).$(MAJOR).$(MINOR) $(LIB)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -o $@ -c $<

libcli.o: libcli.h

atcmdcli.o: atcmdcli.h

clitest: clitest.o $(LIB) atcmdcli.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $<  atcmdcli.o -L. -lcli 

#clitest.exe: clitest.c libcli.o  atcmdcli.o
#	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< libcli.o  atcmdcli.o -lws2_32

clean:
	rm -f *.o $(LIB)* clitest

install: $(LIB)
	install -d $(DESTDIR)$(PREFIX)/include $(DESTDIR)$(PREFIX)/lib
	install -m 0644 libcli.h $(DESTDIR)$(PREFIX)/include
	install -m 0755 $(LIB).$(MAJOR).$(MINOR).$(REVISION) $(DESTDIR)$(PREFIX)/lib
	cd $(DESTDIR)$(PREFIX)/lib && \
	    ln -sf $(LIB).$(MAJOR).$(MINOR).$(REVISION) $(LIB).$(MAJOR).$(MINOR) && \
	    ln -sf $(LIB).$(MAJOR).$(MINOR) $(LIB)

rpm:
	mkdir libcli-$(MAJOR).$(MINOR).$(REVISION)
	cp -R *.c *.h Makefile Doc README *.spec libcli-$(MAJOR).$(MINOR).$(REVISION)
	tar zcvf libcli-$(MAJOR).$(MINOR).$(REVISION).tar.gz --exclude CVS --exclude *.tar.gz libcli-$(MAJOR).$(MINOR).$(REVISION)
	rm -rf libcli-$(MAJOR).$(MINOR).$(REVISION)
	rpm -ta libcli-$(MAJOR).$(MINOR).$(REVISION).tar.gz --clean
