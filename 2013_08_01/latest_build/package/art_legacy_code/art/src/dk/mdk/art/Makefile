ROOT := ../../../../..
SW_ROOT := $(ROOT)/sw

LD	:= $(CROSS_COMPILE)gcc
CC	:= $(CROSS_COMPILE)gcc

CFLAGS +=-Wall -Werror

CFLAGS	+= \
	-I. -I../common/include -I../devlib \
	$(foreach chip,ar2413 ar5210 ar5211 ar5212 ini,-I../devlib/$(chip)) \
	-DCUSTOMER_REL -DART_BUILD -DLINUX -DLinux -D_MLD -DNO_LIB_PRINT \
	-I../mdk -I../../../include -I../client/include \
	-I../client/soc_linux/include \
	-Werror

ifeq ($(DEBUG),1)
    CFLAGS += -D_DEBUG -g
endif

ifeq ($(GDB),1)
    CFLAGS += -g
endif


ifeq ($(PREDATOR_BUILD),1)
    CFLAGS += -DPREDATOR_BUILD
endif

LDFLAGS	+= -L../devlib/Linux_cust -ldevlib  -lpthread -lm
ifneq ($(NO_GPIB),1)
LDFLAGS += -L../devlib/linux_gpib -lgpibenet
endif


VPATH	:= ../common:../client

EXTRA_SRCS = linux_ansi.c

SRCS	:= \
	art_if.c cal_com.c dynarray.c eeprom.c maui_cal.c hw_routines.c \
	linux_hw.c mlibif.c parse.c osWrap_linux.c test.c rssi_power.c \
	mathRoutines.c nrutil.c art_comms.c dynamic_optimizations.c \
	cmdTest.c cal_gen3.c cal_gen5.c endian_func.c \
	parseLoadEar.c dk_mem.c misc_routines.c $(EXTRA_SRCS)

#IQ_Test.c?

OBJS	:=	$(patsubst %.c,Linux_cust/%.o,$(SRCS))

BIN		:=  Linux_cust/art.out

all:	dirs $(BIN)

dirs:
	@mkdir -p Linux_cust

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)	

Linux_cust/%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJS) art.out 
	rm -rf Linux_cust

.PHONY:
	clean all
