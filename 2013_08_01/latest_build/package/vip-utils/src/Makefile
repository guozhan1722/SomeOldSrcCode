
# Makefile for mhx-config and mhx
#
# Author : LL
#
# $Id: Makefile,v 1.4 2011/10/19 15:41:57 Bin Exp $
TARGET1=vip-config
TARGET2=vip
TARGET3=tip

LIBS    = -lc -lm -ldl
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 

OBJS1 = vip-config.o
OBJS2 = vip.o
OBJS3 = tip.o

all: $(TARGET1) $(TARGET2) $(TARGET3) 

$(TARGET1): $(OBJS1)
	$(CC) $(OBJS1) $(LDFLAGS) $@ 

$(TARGET2): $(OBJS2)
	$(CC) $(OBJS2) $(LDFLAGS) $@ 

$(TARGET3): $(OBJS3)
	$(CC) $(OBJS3) $(LDFLAGS) $@ 



o.c:
	$(CC) $(CFLAGS) $< 
	
clean:
	rm -f *.o  
	rm -f $(TARGET1)   
	rm -f $(TARGET2)
	rm -f $(TARGET3)

