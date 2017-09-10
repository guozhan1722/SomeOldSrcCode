# Makefile for sdpserver
#
# Author : LL
#
# $Id: Makefile,v 1.1.4.1 2013/05/10 17:51:15 AlexDing Exp $


TARGETS := checksync checktimesync

LIBS    = -luci
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 


all: $(TARGETS)

checksync: checksync.o  
	$(CC) checksync.o $(LDFLAGS) $@ $(LDLIBS)

checktimesync: checktimesync.o
	$(CC) checktimesync.o $(LDFLAGS) $@ $(LDLIBS)

o.c:
	$(CC) $(CFLAGS) $< 
	
clean:
	rm -f *.o  
	rm -f $(TARGETS)   
