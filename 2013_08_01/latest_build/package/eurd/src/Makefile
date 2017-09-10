# Makefile for eurd
#
# Author : A D
#

TARGETS := eurd_report0 eurd_report1 eurd_report2 eurd_report3 eurd_sms

#CFLAGS += -I./ 
#CFLAGS +="$(TARGET_CFLAGS) -I$(STAGING_DIR)/include-host -I$(STAGING_DIR_HOST)/include -include endian.h  $(TARGET_CFLAGS_EXTRA)"
LIBS    = -lc -lm  -lpthread -luci
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -lz -o 
#OBJS1 = eurd_report0.o database.0


all: $(TARGETS)

eurd_report0: eurd_report0.o database.o
	 $(CC) eurd_report0.o database.o $(LDFLAGS) $@ $(LDLIBS)
eurd_report1: eurd_report1.o database.o
	 $(CC) eurd_report1.o database.o $(LDFLAGS) $@ $(LDLIBS)
eurd_report2: eurd_report2.o database.o
	 $(CC) eurd_report2.o database.o $(LDFLAGS) $@ $(LDLIBS)
eurd_report3: eurd_report3.o database.o
	 $(CC) eurd_report3.o database.o $(LDFLAGS) $@ $(LDLIBS)
eurd_sms: eurd_sms.o database.o
	 $(CC) eurd_sms.o database.o $(LDFLAGS) $@ $(LDLIBS)

eurd_report0.o:
	$(CC) $(CFLAGS) -D AERC_REPORT_NUM=0 -c -o eurd_report0.o eurd_single.c 
eurd_report1.o:
	$(CC) $(CFLAGS) -D AERC_REPORT_NUM=1 -c -o eurd_report1.o eurd_single.c 
eurd_report2.o:
	$(CC) $(CFLAGS) -D AERC_REPORT_NUM=2 -c -o eurd_report2.o eurd_single.c 
eurd_report3.o:
	$(CC) $(CFLAGS) -D AERC_REPORT_NUM=3 -c -o eurd_report3.o eurd_single.c 
eurd_sms.o:
	$(CC) $(CFLAGS) -c -o eurd_sms.o eurd_sms.c 
database.o:
	$(CC) $(CFLAGS) -c -o database.o database.c

clean:
	rm -f *.o  
	rm -f eurd_report0
	rm -f eurd_report1
	rm -f eurd_report2
	rm -f eurd_report3
	rm -f eurd_sms




