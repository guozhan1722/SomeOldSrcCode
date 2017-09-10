# Makefile for msms_command
#
# Author : A D
#
# $Id: Makefile,v 1.1.2.3 2012/10/25 23:00:13 AlexDing Exp $

TARGETS := msmscom msmsif salert

LIBS    = -lc -lm  -lpthread -luci -lmserial
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 

all: $(TARGETS)

msmscom: msmscom.o msmslib.o
	 $(CC) msmscom.o msmslib.o $(LDFLAGS) $@ $(LDLIBS)
msmsif: msmsif.o msmslib.o
	 $(CC) msmsif.o msmslib.o $(LDFLAGS) $@ $(LDLIBS)
salert: salert.o msmslib.o
	 $(CC) salert.o msmslib.o $(LDFLAGS) $@ $(LDLIBS)

salert.o:
	$(CC) $(CFLAGS) -c -o salert.o salert.c 
msmscom.o:
	$(CC) $(CFLAGS) -c -o msmscom.o msmscom.c 
msmsif.o:
	$(CC) $(CFLAGS) -c -o msmsif.o msmsif.c
msmslib.o:
	$(CC) $(CFLAGS) -c -o msmslib.o msmslib.c

clean:
	rm -f *.o  
	rm -f msmscom
	rm -f msmsif
	rm -f salert




