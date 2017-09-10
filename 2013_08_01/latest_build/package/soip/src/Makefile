# Makefile for soip
#
# Author : LL
#
# $Id: Makefile,v 1.5.4.1 2012/06/28 20:36:39 Bin Exp $

TARGETS := soipd1 soipd2 soipdusb soipdmpci soipdmpci2 encom_soipd1 encom_soipd2 encom_soipdusb 

CFLAGS += -I./bsp 
LIBS    = -lc -lm  -lpthread -luci -lmserial
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 

OBJS1 = main.o  net_com.o  com_port.o  serialport_driver.o database.o
OBJS2 = main2.o  net_com2.o  com_port2.o  serialport_driver2.o database2.o
OBJS_USB = main_usb.o  net_com_usb.o  com_port_usb.o  serialport_driver_usb.o database_usb.o
OBJS_MPCI = main_mpci.o  net_com_mpci.o  com_port_mpci.o  serialport_driver_mpci.o database_mpci.o
OBJS_MPCI2 = main_mpci2.o  net_com_mpci2.o  com_port_mpci2.o  serialport_driver_mpci2.o database_mpci2.o

ENCOM_OBJS1 = encom_main.o  encom_net_com.o  encom_com_port.o  encom_serialport_driver.o encom_database.o
ENCOM_OBJS2 = encom_main2.o  encom_net_com2.o  encom_com_port2.o  encom_serialport_driver2.o encom_database2.o
ENCOM_OBJS_USB = encom_main_usb.o  encom_net_com_usb.o  encom_com_port_usb.o  encom_serialport_driver_usb.o encom_database_usb.o
ENCOM_OBJS_MPCI = encom_main_mpci.o  encom_net_com_mpci.o  encom_com_port_mpci.o  encom_serialport_driver_mpci.o encom_database_mpci.o


all: $(TARGETS)

soipd1: $(OBJS1)  
	$(CC) $(OBJS1) $(LDFLAGS) $@ $(LDLIBS)
	cp main.c main2.c
	cp main.c main_usb.c	
	cp main.c main_mpci.c	
	cp net_com.c net_com2.c
	cp net_com.c net_com_usb.c
	cp net_com.c net_com_mpci.c
	cp com_port.c com_port2.c
	cp com_port.c com_port_usb.c
	cp com_port.c com_port_mpci.c
	cp serialport_driver.c serialport_driver2.c
	cp serialport_driver.c serialport_driver_usb.c
	cp serialport_driver.c serialport_driver_mpci.c
	cp database.c database2.c
	cp database.c database_usb.c
	cp database.c database_mpci.c
$(OBJS1):
	$(CC) $(CFLAGS) -DIXP425COM1 -c $*.c


soipd2: $(OBJS2)  
	$(CC) $(OBJS2) $(LDFLAGS) $@ $(LDLIBS)
	rm -f *.o  
	rm -f *2.c
	cp main.c encom_main.c
	cp net_com.c encom_net_com.c
	cp com_port.c encom_com_port.c
	cp serialport_driver.c encom_serialport_driver.c
	cp database.c encom_database.c

	cp main.c main_mpci2.c	
	cp net_com.c net_com_mpci2.c
	cp com_port.c com_port_mpci2.c
	cp serialport_driver.c serialport_driver_mpci2.c
	cp database.c database_mpci2.c

$(OBJS2):
	$(CC) $(CFLAGS) -DIXP425COM2 -c $*.c

soipdusb: $(OBJS_USB)  
	$(CC) $(OBJS_USB) $(LDFLAGS) $@ $(LDLIBS)
	rm -f *.o  
	rm -f *_usb.c

$(OBJS_USB):
	$(CC) $(CFLAGS) -DIXP425USB -c $*.c

soipdmpci: $(OBJS_MPCI)  
	$(CC) $(OBJS_MPCI) $(LDFLAGS) $@ $(LDLIBS)
	rm -f *.o  
	rm -f *_mpci.c

$(OBJS_MPCI):
	$(CC) $(CFLAGS) -DMPCI -c $*.c

soipdmpci2: $(OBJS_MPCI2)  
	$(CC) $(OBJS_MPCI2) $(LDFLAGS) $@ $(LDLIBS)
	rm -f *.o  
	rm -f *_mpci2.c

$(OBJS_MPCI2):
	$(CC) $(CFLAGS) -DMPCI2 -DVIP4G -c $*.c
	#$(CC) $(CFLAGS) -DMPCI2 -c $*.c


encom_soipd1: $(ENCOM_OBJS1)  
	$(CC) $(ENCOM_OBJS1) $(LDFLAGS) $@ $(LDLIBS)
	cp encom_main.c encom_main2.c
	cp encom_net_com.c encom_net_com2.c
	cp encom_com_port.c encom_com_port2.c
	cp serialport_driver.c encom_serialport_driver2.c
	cp encom_main.c encom_main_usb.c
	cp encom_net_com.c encom_net_com_usb.c
	cp encom_com_port.c encom_com_port_usb.c
	cp serialport_driver.c encom_serialport_driver_usb.c
	cp encom_database.c encom_database2.c
	cp encom_database.c encom_database_usb.c

$(ENCOM_OBJS1):
	$(CC) $(CFLAGS) -DIXP425COM1 -DENCOM -c $*.c


encom_soipd2: $(ENCOM_OBJS2)  
	$(CC) $(ENCOM_OBJS2) $(LDFLAGS) $@ $(LDLIBS)
	rm -f *.o  
	rm -f *2.c

$(ENCOM_OBJS2):
	$(CC) $(CFLAGS) -DIXP425COM2 -DENCOM -c $*.c

encom_soipdusb: $(ENCOM_OBJS_USB)  
	$(CC) $(ENCOM_OBJS_USB) $(LDFLAGS) $@ $(LDLIBS)
	rm -f *.o  
	rm -f *_usb.c

$(ENCOM_OBJS_USB):
	$(CC) $(CFLAGS) -DIXP425USB -DENCOM -c $*.c

	
clean:
	rm -f *.o  
	rm -f soipd1 soipd2 soipdusb soipdmpci soipdmpci2 encom_soipd1 encom_soipd2 encom_soipdusb

