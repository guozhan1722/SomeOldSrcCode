
#
# $Id: Makefile,v 1.6 2004/06/26 21:57:46 pkot Exp $
#
# Makefile for the GNOKII tool suite.
#
# Copyright (C) 1999 Hugh Blemings & Pavel Jan�k ml.
#               2000 Karel Zak
#

#
# For this common directory is used "subsystem.la" .la files concept.
#   (the list of object files to be linked together (to COMMON.la),
#   and other dirs is used this _one_ file (instead of all OBJS)
#

TOPDIR=../..
include $(TOPDIR)/Makefile.global

OBJS =	at-emulator.lo \
	virtmodem.lo

all: libDATA.la

libDATA.la: $(OBJS)
	$(LIBTOOL) --mode=link $(CC) -o libDATA.la $(OBJS)

clean:
	$(LIBTOOL) --mode=clean $(RM) libDATA.la $(OBJS)
	$(RM) *~ depend

install:
	@echo

depend dep:
	$(CC) $(CFLAGS) -MM *.c >depend

ifeq (depend,$(wildcard depend))
include depend
endif


.PHONY: all install clean dep depend

