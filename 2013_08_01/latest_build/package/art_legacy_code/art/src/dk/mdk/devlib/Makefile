ROOT := ../../../../..
SW_ROOT := $(ROOT)/sw
CP  :=  cp -f
SIM	?= 0

ifeq ($(SIM),0)
	OUTDIR := Linux_cust
else
	OUTDIR := Linux-Sim
endif

LD	:= $(CROSS_COMPILE)gcc
CC	:= $(CROSS_COMPILE)gcc

SUBDIRS=ar2413 ar5210 ar5211 ar5212 ini linux_gpib

CFLAGS	+=	-I. -I../../../../src/include -I../common/include -I../devmld \
		$(foreach i,$(SUBDIRS),-I$(i) ) \
		-DLINUX -DCUSTOMER_REL -DLinux -D_cplusplus \
		-Werror -fPIC

ifneq ($(NO_GPIB),1)
CFLAGS += -I./linux_gpib
endif

ifeq ($(DEBUG),1)
    CFLAGS += -D_DEBUG -g
else
#    CFLAGS += -DNO_LIB_PRINT
endif

ifeq ($(GDB),1)
    CFLAGS += -g
endif

ifeq ($(PREDATOR_BUILD),1)
    CFLAGS += -DPREDATOR_BUILD
endif

ifeq ($(THIN_CLIENT_BUILD),1)
    CFLAGS += -DTHIN_CLIENT_BUILD
endif

ifeq ($(SIM),1)
	CFLAGS += -DSIM
endif

LDFLAGS	+= -shared
ifneq ($(NO_GPIB),1)
LDFLAGS	+= -L./linux_gpib -lgpibenet 
endif

ifeq ($(INCLUDE_DRIVER_CODE),1)
    CFLAGS += -I../commoninit/ -DINCLUDE_DRIVER_CODE
endif

VPATH	:= ../common
VPATH   += $(SUBDIRS)
EXTRA_SRCS := 
SRCS	:= 	artEar.c art_ani.c athreg.c mAlloc.c mCal.c mConfig.c mCont.c mData.c mDevtbl_rel.c mEeprom.c mMask.c mMaskMath.c mMaskPhys.c mCfg210.c mData210.c mEEP210.c mCfg211.c mData211.c mEEP211.c mData212.c mCfg212.c mCfg212d.c mCfg413.c mEEPROM_d.c mAni212.c mEEPROM_g.c stats_routines.c rate_constants.c $(EXTRA_SRCS)

ifeq ($(SIM),0) 
	SRCS += mInst.c
endif
OBJS	:=	$(patsubst %.c,$(OUTDIR)/%.o,$(SRCS))

LIB		   := $(OUTDIR)/libdevlib.so

all: dirs $(LIB)

dirs:
	for a in $(SUBDIRS); do mkdir -p $(OUTDIR)/$$a; done

$(LIB): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)	

$(OUTDIR)/%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJS) $(LIB)
	rm -rf $(OUTDIR)

.PHONY:
	clean
