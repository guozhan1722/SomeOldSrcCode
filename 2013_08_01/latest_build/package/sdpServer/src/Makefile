# Makefile for sdpserver
#
# Author : LL
#
# $Id: Makefile,v 1.1 2011/01/31 22:33:50 Jim Exp $

TARGETS := sdpServer
LIBS    = -lc -lm  -lpthread -luci
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 
CFLAGS += -DUNIX       

OBJS = sdpServer.o database.o 

all: $(TARGETS)
$(TARGETS): $(OBJS)  
	$(CC) $(OBJS) $(LDFLAGS) $@ $(LDLIBS)

$(OBJS):
	$(CC) $(CFLAGS) -DMPCI -c $*.c

clean:
	rm -f *.o  
	rm -f $(EXEC)   
	rm -f ../../bin/bin/$(EXEC)

