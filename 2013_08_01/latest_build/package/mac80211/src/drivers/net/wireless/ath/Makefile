obj-$(CONFIG_ATH5K)		+= ath5k/
obj-$(CONFIG_ATH9K_HW)		+= ath9k/
obj-$(CONFIG_AR9170_USB)        += ar9170/

obj-$(CONFIG_ATH_COMMON)	+= ath.o

ath-objs :=	main.o \
		regd.o \
		hw.o \
		key.o

ath-$(CONFIG_ATH_DEBUG) += debug.o

ifeq ($(CONFIG_MICROHARD),y)
ccflags-y += -DMICROHARD
endif
