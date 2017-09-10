
#
# Makefile for the BSP driver.
#

#ifeq ($(obj),)
#obj=	.
#endif

#TOP = $(obj)/.

#include $(TOP)/Makefile.inc

#obj-${CONFIG_MPCI_BSP}	+= mpci-bsp.o
obj-$(CONFIG_INPUT_ADXL34X)             += adxl34x.o
obj-$(CONFIG_INPUT_ADXL34X_I2C)         += adxl34x-i2c.o

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
	
