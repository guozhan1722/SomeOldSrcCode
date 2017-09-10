
TARGET1=mpci-4g_server

LIBS    = -lc -lm -lpthread -luci
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 

OBJS1 = mpci-4g_server.o

all: $(TARGET1) setnv_4g_default
     
setnv_4g_default:setnv_4g_default.o 
	 $(CC) setnv_4g_default.o $(LDFLAGS) $@ 

$(TARGET1): $(OBJS1)
	$(CC) $(OBJS1) $(LDFLAGS) $@ 
o.c:
	$(CC) $(CFLAGS) $< 
clean:
	rm -f *.o  
	rm -f $(TARGET1)
	rm -f setnv_4g_default

