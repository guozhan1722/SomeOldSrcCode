
#
# $Id: Makefile,v 1.26 2007/10/30 17:07:00 dforsi Exp $
#
# Makefile for the GNOKII tool suite.
#
# Copyright (C) 1999 Hugh Blemings & Pavel Janík ml.
#               2000 Karel Zak
#

#
# For this common directory is used "subsystem.la" .la files concept.
#   (the list of object files to be linked together (to COMMON.la),
#   and other dirs is used this _one_ file (instead of all OBJS)
#

TOPDIR=../..
include $(TOPDIR)/Makefile.global

CFLAGS += $(LIBPCSCLITE_CFLAGS)

OBJS =	generic.lo \
	atgen.lo

all: libPHONES.la

libPHONES.la: $(OBJS)
	$(LIBTOOL) --mode=link $(CC) -o libPHONES.la $(OBJS)

clean:
	$(LIBTOOL) --mode=clean $(RM) libPHONES.la $(OBJS)
	$(RM) *~ depend

install:
	@echo

depend dep:
	$(CC) $(CFLAGS) -MM *.c >depend

ifeq (depend,$(wildcard depend))
include depend
endif


.PHONY: all install clean dep depend

