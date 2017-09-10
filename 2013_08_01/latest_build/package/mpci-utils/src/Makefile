
# Makefile for mhx-config and mhx
#
# Author : LL
#
# $Id: Makefile,v 1.1.4.1 2012/11/12 23:34:45 Daniel Exp $
TARGET2=mpci
TARGET3=tip

LIBS    = -lc -lm -ldl
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 

OBJS2 = mpci.o
OBJS3 = tip.o

all: $(TARGET2) $(TARGET3)  Bin_txt

$(TARGET2): $(OBJS2)
	$(CC) $(OBJS2) $(LDFLAGS) $@ 

$(TARGET3): $(OBJS3)
	$(CC) $(OBJS3) $(LDFLAGS) $@
 
Bin_txt: bin_txt.o
	$(CC) bin_txt.o $(LDFLAGS) $@ 
 
o.c:
	$(CC) $(CFLAGS) $< 
	
clean:
	rm -f *.o  
	rm -f $(TARGET2)
	rm -f $(TARGET3)

