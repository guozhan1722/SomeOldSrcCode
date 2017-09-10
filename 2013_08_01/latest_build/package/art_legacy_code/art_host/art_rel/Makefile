CC := $(TOOL_PREFIX)gcc
LD := $(TOOL_PREFIX)gcc  
STRIP := $(TOOL_PREFIX)strip

include ../rules.mk

TARGET1 = ../bin/art_host
TARGET2 =
TARGET3 =

COMMON_SRC :=
COMMON_SRC += ../src/art_util.c
COMMON_SRC += ../src/convert.c
COMMON_SRC += ../src/hw_routines.c

LOCAL_SRC :=
LOCAL_SRC += ./src/test.c
LOCAL_SRC += ./src/art_if.c
LOCAL_SRC += ./src/MLIBif.c
LOCAL_SRC += ./src/cal_com.c
LOCAL_SRC += ./src/art_comms.c
LOCAL_SRC += ./src/cal_gen3.c
LOCAL_SRC += ./src/cal_gen5.c
LOCAL_SRC += ./src/cmdTest.c
LOCAL_SRC += ./src/dk_mem.c
LOCAL_SRC += ./src/dynamic_optimizations.c
LOCAL_SRC += ./src/dynarray.c
LOCAL_SRC += ./src/eeprom.c
LOCAL_SRC += ./src/mathRoutines.c
LOCAL_SRC += ./src/maui_cal.c
LOCAL_SRC += ./src/nrutil.c
LOCAL_SRC += ./src/parse.c
LOCAL_SRC += ./src/parseLoadEar.c
LOCAL_SRC += ./src/rssi_power.c

SRC1 = $(COMMON_SRC) $(LOCAL_SRC)
OBJS1 = $(SRC1:.c=.o)

INCLUDES1  = -I../include -I./include -I../devlib/include
LIBS    = -lc -lm -ldl
CFLAGS += $(INCLUDES1) $(COMMONCFLAG) 
LDFLAGS += $(LIBDIRS) $(LDLIBS) $(LIBS) 

all: $(TARGET1)

$(TARGET1):$(OBJS1)
	$(CC) -L../lib -ldevlib -o $@  $(LDFLAGS) $(OBJS1)

clean:
	$(RM) $(TARGET1) $(TARGET2) $(TARGET3)
	$(RM) $(OBJS)
	$(RM) src/*~ *~ 
	$(RM) depend.mk

%.o : %.c 
	$(CC) -c $(CFLAGS) $< -o $@

include ../dep.mk


