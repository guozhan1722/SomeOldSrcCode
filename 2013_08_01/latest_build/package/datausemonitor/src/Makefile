# Makefile for datausemonitor: carrier data usage monitor
#
# Author : A D
#
# $Id: Makefile,v 1.1.2.1 2013/06/27 20:57:18 AlexDing Exp $

TARGETS := datausemonitor

LIBS    = -lc -lm  -lpthread -luci -lmserial
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 

all: $(TARGETS)

datausemonitor: datausemonitor.o msmslib.o
	 $(CC) datausemonitor.o msmslib.o $(LDFLAGS) $@ $(LDLIBS)

datausemonitor.o:
	$(CC) $(CFLAGS) -c -o datausemonitor.o datausemonitor.c 
msmslib.o:
	$(CC) $(CFLAGS) -c -o msmslib.o msmslib.c 

clean:
	rm -f *.o  
	rm -f datausemonitor




