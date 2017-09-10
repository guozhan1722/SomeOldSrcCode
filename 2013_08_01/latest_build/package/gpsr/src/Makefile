# Makefile for eurd
#
# Author : A D
#


TARGETS := gpsreport gps_webget gpsr_sms trackersmsif gpstracker gpsrecorder gpsloadrecord

LIBS    = -lc -lm  -lpthread -luci  -lmserial
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 


all: $(TARGETS)

gpsreport: gpsreport.o gpsrlib.o database.o
	 $(CC) gpsreport.o gpsrlib.o database.o $(LDFLAGS) $@ $(LDLIBS)

gpsr_sms: gpsr_sms.o gpsrlib.o database.o
	 $(CC) gpsr_sms.o gpsrlib.o database.o $(LDFLAGS) $@ $(LDLIBS)

gps_webget: gps_webget.o
	 $(CC) gps_webget.o $(LDFLAGS) $@ $(LDLIBS)

trackersmsif: trackersmsif.o
	 $(CC) trackersmsif.o $(LDFLAGS) $@ $(LDLIBS)

gpstracker: gpstracker.o gpsrlib.o database.o
	 $(CC) gpstracker.o gpsrlib.o database.o $(LDFLAGS) $@ $(LDLIBS)

gpsrecorder: gpsrecorder.o gpsrlib.o database.o
	 $(CC) gpsrecorder.o gpsrlib.o database.o $(LDFLAGS) $@ $(LDLIBS)

gpsloadrecord: gpsloadrecord.o gpsrlib.o database.o
	 $(CC) gpsloadrecord.o gpsrlib.o database.o $(LDFLAGS) $@ $(LDLIBS)

gps_webget.o:
	$(CC) $(CFLAGS) -c -o gps_webget.o gps_webget.c 

gpsreport.o:
	$(CC) $(CFLAGS) -c -o gpsreport.o gpsr.c

gpsr_sms.o:
	$(CC) $(CFLAGS) -c -o gpsr_sms.o gpsr_sms.c

gpsrlib.o:
	$(CC) $(CFLAGS) -c -o gpsrlib.o gpsrlib.c

database.o:
	$(CC) $(CFLAGS) -c -o database.o database.c 

gpstracker.o:
	$(CC) $(CFLAGS) -c -o gpstracker.o gpstracker.c 

gpsrecorder.o:
	$(CC) $(CFLAGS) -c -o gpsrecorder.o gpsrecorder.c 

gpsloadrecord.o:
	$(CC) $(CFLAGS) -c -o gpsloadrecord.o gpsloadrecord.c 

trackersmsif.o:
	$(CC) $(CFLAGS) -c -o trackersmsif.o trackersmsif.c 


clean:
	rm -f *.o  
	rm -f gpsreport
	rm -f gpsr_sms
	rm -f gps_webget
	rm -f gpsrecorder
	rm -f gpstracker
	rm -f trackersmsif





