
#
# Makefile for the BSP driver.
#

#ifeq ($(obj),)
#obj=	.
#endif

#TOP = $(obj)/.

#include $(TOP)/Makefile.inc

obj-${CONFIG_VIP_BSP}	+= vip-bsp.o
#vipbsp-objs	:= vip-bsp.o IxTimersCodelet.o

#IXP400_2_3 := $(TOP)/../../../linux/snapgear/modules/ixp425/ixp400-2.3

#INCS += -I$(TOP) -I$(IXP400_2_3)/ixp400_xscale_sw/src/include
#INCS += -I$(IXP400_2_3)/ixp_osal/include
#INCS += -I$(IXP400_2_3)/ixp_osal/os/linux/include/core
#INCS += -I$(IXP400_2_3)/ixp_osal/include/modules/ioMem
#INCS += -I$(IXP400_2_3)/ixp_osal/include/modules/bufferMgt
#INCS += -I$(IXP400_2_3)/ixp_osal/os/linux/include/modules/bufferMgt
#INCS += -I$(IXP400_2_3)/ixp_osal/include/platforms/ixp400
#INCS += -I$(IXP400_2_3)/ixp_osal/os/linux/include/platforms/ixp400
#INCS += -I$(IXP400_2_3)/ixp_osal/os/linux/include/platforms/ixp400/ixp425

#EXTRA_CFLAGS+= $(INCS) $(COPTS)

#-include $(TOPDIR)/Rules.make

#all:
#	$(MAKE) -C $(KERNELPATH) TARGET=$(TARGET) TOOLPRFEIX=$(TOOLPRFEIX) CROSS_COMPILE=$(CROSS_COMPILE) SUBDIRS=$(shell pwd) modules
#
#install:
#	test -d $(DESTDIR)/$(KMODPATH) || mkdir -p $(DESTDIR)/$(KMODPATH)
#	cp vipbsp.$(KMODSUF) $(DESTDIR)/$(KMODPATH)
#
#clean:
#	rm -f *~ *.o *.ko *.mod.c .*.cmd
#	rm -f .depend .version .*.o.flags .*.o.d
#       rm -rf .tmp_versions
	
