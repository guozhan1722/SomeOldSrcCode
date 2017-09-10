
#
# $Id: Makefile,v 1.31 2007/11/02 13:40:43 dforsi Exp $
#
# Makefile for the GNOKII tool suite.
#
# Copyright (C) 1999 Hugh Blemings & Pavel Janík ml.
#               2000 Karel Zak
#

TOPDIR=..
include $(TOPDIR)/Makefile.global

OBJS =	gnokii.o gnokii-sms.o gnokii-utils.o

ifdef OWN_GETOPT
	OBJS += $(TOPDIR)/getopt/libgetopt.a
endif

ifndef WIN32_CROSS
PRGNAME=gnokii
else
PRGNAME=gnokii.exe
TARGET=-mconsole
CFLAGS+=-DGNOKIIDLL_IMPORTS
endif

LDLIBS += $(TOPDIR)/common/libmserial.la

all: gnokii

gnokii: $(OBJS) $(TOPDIR)/common/libmserial.la
	$(LIBTOOL) --mode=link $(CC) $(TARGET) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(PRGNAME)

clean:
	$(LIBTOOL) --mode=clean $(RM) gnokii $(OBJS)
	$(RM) *~ depend *.exe core *.bak
ifdef OWN_GETOPT
	$(MAKE) -C $(TOPDIR)/getopt clean
endif

install: gnokii
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(LIBTOOL) --mode=install $(INSTALL) $(PRGNAME) $(DESTDIR)$(bindir)

depend dep:
	$(CC) $(CFLAGS) -MM *.c >depend

ifeq (depend,$(wildcard depend))
include depend
endif

.PHONY: all install clean dep depend
