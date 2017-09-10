EXEC = dhcpcd
OBJS = arp.o buildmsg.o client.o dhcpcd.o peekfd.o signals.o udpipgen.o
#LIB_CFLAGS=-D__GLIBC__=1

CFLAGS += -D_BSD_SOURCE=1 -DEMBED $(LIB_CFLAGS)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

o.c:
	$(CC) $(CFLAGS) $< 

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

