
# Makefile for package smodem source code 
#
# Author : LL
# 
#
# $Id: Makefile,v 1.3 2011/10/12 21:10:29 Larry Exp $


TARGET1=mtip
TARGET2=mdtip

LIBS    = -lc -lm -ldl
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 

OBJS1 = mtip.o
OBJS2 = mdtip.o

all: $(TARGET1) $(TARGET2) 

$(TARGET1): $(OBJS1)
	$(CC) $(OBJS1) $(LDFLAGS) $@ 

$(TARGET2): $(OBJS2)
	$(CC) $(OBJS2) $(LDFLAGS) $@ 


o.c:
	$(CC) $(CFLAGS) $< 
	
clean:
	rm -f *.o  
	rm -f $(TARGET1)   
	rm -f $(TARGET2)

