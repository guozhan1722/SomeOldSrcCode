
CC := $(TARGET_CROSS)gcc
LD := $(TARGET_CROSS)gcc  
STRIP := $(TARGET_CROSS)strip

CFLAGS        = $(MCAST_INCLUDE) $(SNMPDEF) $(RSRRDEF) $(INCLUDES) $(DEFS) $(USERCOMPILE)
CFLAGS       += -O2 -W -Wall -Werror
OBJS          = $(IGMP_OBJS) $(ROUTER_OBJS) $(MAPPER_OBJS) $(MRINFO_OBJS) \
		$(MTRACE_OBJS) $(MSTAT_OBJS)
SRCS          = $(OBJS:.o=.c)

LINT          = splint
LINTFLAGS     = $(MCAST_INCLUDE) $(filter-out -W -Wall -Werror, $(CFLAGS)) -posix-lib -weak -skipposixheaders

all: ripconfread
	echo rmqctl build

.c.o:
	@printf "  CC      $@\n"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -v -c -o $@ $<

ripconfread: ripconfread.o
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	-@$(RM) ripconfread
