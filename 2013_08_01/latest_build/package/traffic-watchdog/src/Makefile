
GXX=mips-openwrt-linux-uclibc-g++

TARGET=t_watchdog

LIBS    = -lc -lm -lpthread -luci -lstdc++
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) -o 
#CXXFLAGS += -O2 -g -DW_DEBUG 
CXXFLAGS += -O2 -g  
CFLAGS += -g 

OBJS = traffic.o t_watchdog.o 

all: $(TARGET)

$(TARGET): $(OBJS)
	${GXX} $(OBJS) ${LDFLAGS} $@ 

o.c:
	$(CC) $(CFLAGS) -I./include $< 

o.cpp: 
	${GXX} $(CXXFLAGS) -c $<

clean:
	rm -f *.o  
	rm -f $(TARGET)

