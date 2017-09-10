#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under GNU General Public Licese v2.
# See /LICENSE for more information.
#

#all: event_test 
#
#%.o: %.c
#	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $^
#
#event_test: evtest.o
#	$(CC) -o $@ $^
#
#clean:
# 	rm -f *.o event_test 

#CFLAGS += -I./bsp 
#LIBS    =-luci 
#LDFLAGS +=$(LIBDIRS) $(LDLIBS) $(LIBS) -lz -o
LIBS    =-luci -lpthread -lm -lc
LDFLAGS +=$(LIBS)

all: event_test adxl_report dhcp_release

event_test: evtest.o 
	 $(CC) evtest.o $(LDFLAGS) -o $@ $(LDLIBS)
adxl_report: adxl_report.o 
	 $(CC) adxl_report.o $(LDFLAGS) -o $@ $(LDLIBS)
dhcp_release: dhcp_release.o 
	 $(CC) dhcp_release.o $(LDFLAGS) -o $@ $(LDLIBS)

evtest.o:
	$(CC) $(CFLAGS) -c -o evtest.o evtest.c 
adxl_report.o:
	$(CC) $(CFLAGS) -c -o adxl_report.o adxl_report.c  
dhcp_release.o:
	$(CC) $(CFLAGS) -c -o dhcp_release.o dhcp_release.c


clean:
	rm -f *.o  
	rm -f event_test
	rm -f adxl_report
	rm -f dhcp_release

