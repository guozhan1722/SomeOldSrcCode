# Makefile for Modbus device slave service
#
# Author : Alex D
#
# $Id: Makefile,v 1.1.2.1 2012/11/29 17:59:03 AlexDing Exp $

TARGETS := modbusds

LIBS    = -lc -lm  -lpthread -luci -lmserial
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 

all: $(TARGETS)

modbusds: modbusds.o serialport_driver.o
	 $(CC) modbusds.o serialport_driver.o $(LDFLAGS) $@ $(LDLIBS)

modbusds.o:
	$(CC) $(CFLAGS) -c -o modbusds.o modbusds.c 

serialport_driver.o:
	$(CC) $(CFLAGS) -c -o serialport_driver.o serialport_driver.c 

clean:
	rm -f *.o  
	rm -f modbusds




