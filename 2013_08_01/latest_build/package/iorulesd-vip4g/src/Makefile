
TARGET=iorulesd

LIBS    = -lc -lm -lpthread -luci
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 

OBJS = iorules.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $@ 

o.c:
	$(CC) $(CFLAGS) $< 
clean:
	rm -f *.o  
	rm -f $(TARGET)

