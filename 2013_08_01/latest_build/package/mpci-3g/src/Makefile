
TARGET2=mpci-3g

LIBS    = -lc -lm -ldl
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 

OBJS2 = mpci-3g.o

all: $(TARGET2)

$(TARGET2): $(OBJS2)
	$(CC) $(OBJS2) $(LDFLAGS) $@ 

o.c:
	$(CC) $(CFLAGS) $< 
	
clean:
	rm -f *.o  
	rm -f $(TARGET2)

