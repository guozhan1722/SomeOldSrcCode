
TARGETS := sdpClient
LIBS    = -lc -lm  -lpthread -luci
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 
CFLAGS += -DUNIX       

OBJS = sdpClient.o sockLib.o  database.o
all: $(TARGETS)
$(TARGETS): $(OBJS)  
	$(CC) $(OBJS) $(LDFLAGS) $@ $(LDLIBS)

$(OBJS):
	$(CC) $(CFLAGS) -DMPCI -c $*.c

clean:
	rm -f *.o  
	rm -f $(TARGETS)   
