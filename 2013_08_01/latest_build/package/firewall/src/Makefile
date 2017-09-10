# Makefile for soip
#
# Author : LL
#
# $Id: Makefile,v 1.1 2011/10/05 14:51:27 Jim Exp $

TARGETS := mh_firewall 

LIBS    = -lc -lm  -lpthread -luci
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 

OBJS1 = mh_firewall.o 

all: $(TARGETS)

mh_firewall: $(OBJS1)  
	$(CC) $(OBJS1) $(LDFLAGS) $@ $(LDLIBS)
$(OBJS1):
	$(CC) $(CFLAGS) -c $*.c

clean:
	rm -f *.o  
	rm -f mh_firewall

