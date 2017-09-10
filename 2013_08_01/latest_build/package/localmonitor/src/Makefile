# Makefile for local monitor
#
# Author : A D
#


TARGETS := localmonitor

LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 

all: $(TARGETS)

localmonitor: localmonitor.o
	 $(CC) localmonitor.o $(LDFLAGS) $@ $(LDLIBS)

localmonitor.o:
	$(CC) $(CFLAGS) -c -o localmonitor.o localmonitor.c 

clean:
	rm -f *.o  
	rm -f localmonitor





