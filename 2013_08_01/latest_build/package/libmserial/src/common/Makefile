#
# $Id: Makefile,v 1.101 2007/11/11 15:50:54 pkot Exp $
#
# Makefile for the GNOKII tool suite.
#
# Copyright (C) 1999 Hugh Blemings & Pavel Jan�k ml.
#               2000 Karel Zak
#		2002-2005 Pawel Kot
#

#
# For this common directory is used "subsystem.la" .la files concept.
#   (the list of object files to be linked together (to COMMON.la),
#   and other dirs is used this _one_ file (instead of all OBJS)
#

TOPDIR=..
include $(TOPDIR)/Makefile.global

# REMEMBER: if you change the numbers below, please change them in gnokii.h as well.
# Change it in the following manner
#  - if you change the code and don't change API, increase GNOKII_RELEASE
#  - if you change API in backward compatible manner (adding things),
#    increase GNOKII_MAJOR, GNOKII_MINOR, set GNOKII_RELEASE to 0
#  - if you change API in backward incompatible manner (removing things,
#    changing things), increase GNOKII_MAJOR, set GNOKII_RELEASE to 0
# Change is just once per release!
GNOKII_MAJOR = 8
GNOKII_MINOR = 5
GNOKII_RELEASE = 0
ifdef WIN32_CROSS
LIBTOOL_UNDEF = -no-undefined
LIBTOOL_VER = -version-info $(GNOKII_MAJOR):$(GNOKII_RELEASE):$(GNOKII_MINOR) -avoid-version
LIBTOOL_RPATH=-rpath $(libdir)
LDLIBS += -lws2_32 -lwinmm
else
LIBTOOL_VER = -version-info $(GNOKII_MAJOR):$(GNOKII_RELEASE):$(GNOKII_MINOR)
LIBTOOL_RPATH=-rpath $(libdir)
endif

ifndef WIN32
DATA_DIR = data
endif

DIRS += phones \
	links \
	devices \
	$(DATA_DIR)

LIBS   += $(LIBPCSCLITE_LIBS)

OBJS =	libfunctions.lo \
	gsm-api.lo \
	gsm-error.lo \
	gsm-statemachine.lo \
	cfgreader.lo \
	device.lo \
	gsm-sms.lo \
	gsm-encoding.lo \
	gsm-common.lo \
	pkt.lo \
	compat.lo \
	misc.lo \
	snprintf.lo

ifdef NEED_LCHARSET
OBJS += ../intl/localcharset.lo
endif

all: $(DIRS) libmserial.la

libmserial.la: $(OBJS) phones/libPHONES.la links/libLINKS.la devices/libDEVICES.la
	$(LIBTOOL) --mode=link $(CC) $(LDFLAGS) $(LIBTOOL_UNDEF) -o libmserial.la $(OBJS) phones/libPHONES.la links/libLINKS.la devices/libDEVICES.la $(LIBTOOL_VER) $(LIBTOOL_RPATH) $(LDLIBS)

$(DIRS):
	$(MAKE) -C $@

clean:
	$(MAKE) -C data clean
	$(MAKE) -C phones clean
	$(MAKE) -C links clean
	$(MAKE) -C devices clean
	$(LIBTOOL) --mode=clean $(RM) libmserial.la $(OBJS)
	$(RM) *~ depend

distclean:
	$(RM) gnvcal.c

install:
	$(INSTALL) -d $(DESTDIR)$(libdir)
	$(LIBTOOL) --mode=install $(INSTALL) libmserial.la $(DESTDIR)$(libdir)
	@echo

depend dep:
	$(CC) $(CFLAGS) -MM *.c >depend

ifeq (depend,$(wildcard depend))
include depend
endif


.PHONY: all install clean dep depend $(DIRS)
