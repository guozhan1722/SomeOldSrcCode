
# Makefile for mhx-config and mhx
#
# Author : LL
#
# $Id: Makefile,v 1.1 2011/01/21 16:10:23 Jim Exp $


#CFLAGS ?= -O2 -g
#CFLAGS += -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration

#TARGET1=ledcon

#LIBS    = -lc -lm -ldl
#LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 

#OBJS1 = ledcon.o

#all: $(TARGET1)

#$(TARGET1): $(OBJS1)
#	$(CC) $(OBJS1) $(LDFLAGS) $@ 

#o.c:
#	$(CC) $(CFLAGS) $< 
	
#clean:
#	rm -f *.o  
#	rm -f $(TARGET1)   




OBJS = mpci-led.o 

ALL = mpci-led

NL1FOUND := $(shell $(PKG_CONFIG) --atleast-version=1 libnl-1 && echo Y)
NL2FOUND := $(shell $(PKG_CONFIG) --atleast-version=2 libnl-2.0 && echo Y)

ifeq ($(NL1FOUND),Y)
NLLIBNAME = libnl-1
endif

ifeq ($(NL2FOUND),Y)
CFLAGS += -DCONFIG_LIBNL20
LIBS += -lnl-genl
NLLIBNAME = libnl-2.0
endif

ifeq ($(NLLIBNAME),)
$(error Cannot find development files for any supported version of libnl)
endif

LIBS += $(shell $(PKG_CONFIG) --libs $(NLLIBNAME))
CFLAGS += $(shell $(PKG_CONFIG) --cflags $(NLLIBNAME))

all: version_check $(ALL)

version_check:
ifeq ($(NL2FOUND),Y)
else
ifeq ($(NL1FOUND),Y)
else
	$(error No libnl found)
endif
endif

%.o: %.c iw.h nl80211.h
	$(CC) $(CFLAGS) -c -o $@ $<

mpci-led:	$(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o mpci-led


clean:
	rm -f *.o  
	rm -f mpci-led   

