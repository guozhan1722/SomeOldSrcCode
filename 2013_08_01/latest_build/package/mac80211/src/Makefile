export KMODDIR?=       updates
KMODDIR_ARG:=   "INSTALL_MOD_DIR=$(KMODDIR)"
ifneq ($(origin KLIB), undefined)
KMODPATH_ARG:=  "INSTALL_MOD_PATH=$(KLIB)"
else
export KLIB:=          /lib/modules/$(shell uname -r)
endif
export KLIB_BUILD ?=	$(KLIB)/build
# Sometimes not available in the path
MODPROBE := /sbin/modprobe
MADWIFI=$(shell $(MODPROBE) -l ath_pci)
OLD_IWL=$(shell $(MODPROBE) -l iwl4965)

DESTDIR?=

ifneq ($(KERNELRELEASE),)

NOSTDINC_FLAGS := -I$(M)/include/ -include $(M)/include/linux/compat-2.6.h $(CFLAGS)

obj-y := compat/

obj-$(CONFIG_COMPAT_RFKILL) += net/rfkill/

ifeq ($(BT),)
obj-$(CONFIG_COMPAT_WIRELESS) += net/wireless/ net/mac80211/
obj-$(CONFIG_COMPAT_WIRELESS_MODULES) += drivers/net/wireless/

obj-$(CONFIG_COMPAT_NET_USB_MODULES) += drivers/net/usb/

obj-$(CONFIG_COMPAT_NETWORK_MODULES) += drivers/net/
obj-$(CONFIG_COMPAT_VAR_MODULES) +=  drivers/misc/eeprom/
endif

obj-$(CONFIG_COMPAT_BLUETOOTH) += net/bluetooth/
obj-$(CONFIG_COMPAT_BLUETOOTH_MODULES) += drivers/bluetooth/

else

export PWD :=	$(shell pwd)

# These exported as they are used by the scripts
# to check config and compat autoconf
export COMPAT_CONFIG=config.mk
export CONFIG_CHECK=.$(COMPAT_CONFIG)_md5sum.txt
export COMPAT_AUTOCONF=include/linux/compat_autoconf.h
export CREL=$(shell cat $(PWD)/compat-release)
export CREL_PRE:=.compat_autoconf_
export CREL_CHECK:=$(CREL_PRE)$(CREL)

include $(PWD)/$(COMPAT_CONFIG)

all: modules

modules: $(CREL_CHECK)
	@./scripts/check_config.sh
	$(MAKE) -C $(KLIB_BUILD) M=$(PWD) modules
	@touch $@

bt: $(CREL_CHECK)
	@./scripts/check_config.sh
	$(MAKE) -C $(KLIB_BUILD) M=$(PWD) BT=TRUE modules
	@touch $@

# With the above and this we make sure we generate a new compat autoconf per
# new relase of compat-wireless-2.6 OR when the user updates the 
# $(COMPAT_CONFIG) file
$(CREL_CHECK):
	@# Force to regenerate compat autoconf
	@rm -f $(CONFIG_CHECK)
	@./scripts/check_config.sh
	@touch $@
	@md5sum $(COMPAT_CONFIG) > $(CONFIG_CHECK)

btinstall: btuninstall bt-install-modules

bt-install-modules: bt
	$(MAKE) -C $(KLIB_BUILD) M=$(PWD) $(KMODDIR_ARG) $(KMODPATH_ARG) BT=TRUE \
		modules_install
	@/sbin/depmod -ae
	@echo
	@echo "Currently detected bluetooth subsystem modules:"
	@echo
	@$(MODPROBE) -l sco
	@$(MODPROBE) -l l2cap
	@$(MODPROBE) -l hidp
	@$(MODPROBE) -l rfcomm
	@$(MODPROBE) -l bnep
	@$(MODPROBE) -l btusb
	@$(MODPROBE) -l bluetooth
	@echo
	@echo Now run:
	@echo
	@echo sudo make btunload:
	@echo
	@echo And the load the needed bluetooth modules. If unsure reboot.
	@echo

btuninstall:
	@# New location, matches upstream
	@rm -rf $(KLIB)/$(KMODDIR)/net/bluetooth/
	@rm -rf $(KLIB)/$(KMODDIR)/drivers/bluetooth/
	@# Lets only remove the stuff we are sure we are providing
	@# on the misc directory.
	@/sbin/depmod -ae
	@echo
	@echo "Your old bluetooth subsystem modules were left intact:"
	@echo
	@$(MODPROBE) -l sco
	@$(MODPROBE) -l l2cap
	@$(MODPROBE) -l hidp
	@$(MODPROBE) -l rfcomm
	@$(MODPROBE) -l bnep
	@$(MODPROBE) -l btusb
	@$(MODPROBE) -l bluetooth
	@
	@echo

btclean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) BT=TRUE clean
	@rm -f $(CREL_PRE)*

install: uninstall install-modules install-scripts

install-modules: modules
	$(MAKE) -C $(KLIB_BUILD) M=$(PWD) $(KMODDIR_ARG) $(KMODPATH_ARG) \
		modules_install
	@./scripts/update-initramfs

install-scripts:
	@# All the scripts we can use
	@mkdir -p $(DESTDIR)/usr/lib/compat-wireless/
	@install scripts/modlib.sh	$(DESTDIR)/usr/lib/compat-wireless/
	@install scripts/madwifi-unload	$(DESTDIR)/usr/sbin/
	@# This is to allow switching between drivers without blacklisting
	@install scripts/athenable	$(DESTDIR)/usr/sbin/
	@install scripts/b43enable	$(DESTDIR)/usr/sbin/
	@install scripts/iwl-enable	$(DESTDIR)/usr/sbin/
	@install scripts/athload	$(DESTDIR)/usr/sbin/
	@install scripts/b43load	$(DESTDIR)/usr/sbin/
	@install scripts/iwl-load	$(DESTDIR)/usr/sbin/
	@if [ ! -z "$(MADWIFI)" ] && [ -z "$(DESTDIR)" ]; then \
		echo ;\
		echo -n "Note: madwifi detected, we're going to disable it. "  ;\
		echo "If you would like to enable it later you can run:"  ;\
		echo "    sudo athenable madwifi"  ;\
		echo ;\
		echo Running athenable ath5k...;\
		/usr/sbin/athenable ath5k ;\
	fi
	@if [ ! -z "$(OLD_IWL)" ] && [ -z "$(DESTDIR)" ]; then \
		echo ;\
		echo -n "Note: iwl4965 detected, we're going to disable it. "  ;\
		echo "If you would like to enable it later you can run:"  ;\
		echo "    sudo iwl-load iwl4965"  ;\
		echo ;\
		echo Running iwl-enable iwlagn...;\
		/usr/sbin/iwl-enable iwlagn ;\
	fi
	@# If on distributions like Mandriva which like to
	@# compress their modules this will find out and do
	@# it for you. Reason is some old version of modutils
	@# won't know mac80211.ko should be used instead of
	@# mac80211.ko.gz
	@./scripts/compress_modules
	@# Mandrake doesn't have a depmod.d/ conf file to prefer
	@# the updates/ dir which is what we use so we add one for it
	@# (or any other distribution that doens't have this).
	@./scripts/check_depmod
	@# Udev stuff needed for the new compat_firmware_class.
	@./compat/scripts/compat_firmware_install
	@/sbin/depmod -a
	@echo
	@echo "Currently detected wireless subsystem modules:"
	@echo 
	@$(MODPROBE) -l mac80211
	@$(MODPROBE) -l cfg80211
	@$(MODPROBE) -l lib80211
	@$(MODPROBE) -l adm8211
	@$(MODPROBE) -l ar9170usb
	@$(MODPROBE) -l at76c50x-usb
	@$(MODPROBE) -l ath
	@$(MODPROBE) -l ath5k
	@$(MODPROBE) -l ath9k
	@$(MODPROBE) -l b43
	@$(MODPROBE) -l b43legacy
	@$(MODPROBE) -l b44
	@$(MODPROBE) -l cdc_ether
	@$(MODPROBE) -l eeprom_93cx6
	@$(MODPROBE) -l ipw2100
	@$(MODPROBE) -l ipw2200
	@$(MODPROBE) -l iwl3945
	@$(MODPROBE) -l iwlagn
	@$(MODPROBE) -l iwlcore
	@$(MODPROBE) -l lib80211_crypt_ccmp
	@$(MODPROBE) -l lib80211_crypt_tkip
	@$(MODPROBE) -l lib80211_crypt_wep
	@$(MODPROBE) -l libertas
	@$(MODPROBE) -l libertas_cs
	@$(MODPROBE) -l libertas_sdio
	@$(MODPROBE) -l libertas_spi
	@$(MODPROBE) -l libertas_tf
	@$(MODPROBE) -l libertas_tf_usb
	@$(MODPROBE) -l libipw
	@$(MODPROBE) -l mac80211_hwsim
	@$(MODPROBE) -l mwl8k
	@$(MODPROBE) -l p54common
	@$(MODPROBE) -l p54pci
	@$(MODPROBE) -l p54spi
	@$(MODPROBE) -l p54usb
	@$(MODPROBE) -l rndis_host
	@$(MODPROBE) -l rndis_wlan
	@$(MODPROBE) -l rt2400pci
	@$(MODPROBE) -l rt2500pci
	@$(MODPROBE) -l rt2500usb
	@$(MODPROBE) -l rt2x00lib
	@$(MODPROBE) -l rt2x00pci
	@$(MODPROBE) -l rt2x00usb
	@$(MODPROBE) -l rt61pci
	@$(MODPROBE) -l rt73usb
	@$(MODPROBE) -l rtl8180
	@$(MODPROBE) -l rtl8187
	@$(MODPROBE) -l ssb
	@$(MODPROBE) -l usb8xxx
	@$(MODPROBE) -l usbnet
	@$(MODPROBE) -l zd1211rw
	@echo
	@echo "Currently detected bluetooth subsystem modules:"
	@echo
	@$(MODPROBE) -l sco
	@$(MODPROBE) -l l2cap
	@$(MODPROBE) -l hidp
	@$(MODPROBE) -l rfcomm
	@$(MODPROBE) -l bnep
	@$(MODPROBE) -l btusb
	@$(MODPROBE) -l bluetooth
	@echo 
	@echo Now run:
	@echo 
	@echo sudo make unload to unload both wireless and bluetooth modules
	@echo sudo make wlunload to unload wireless modules
	@echo sudo make btunload to unload bluetooth modules
	@echo
	@echo And then load the wireless or bluetooth module you need. If unsure reboot.
	@echo Alternatively use "sudo make load/wlload/btload" to load modules
	@echo

uninstall:
	@# New location, matches upstream
	@rm -rf $(KLIB)/$(KMODDIR)/net/mac80211/
	@rm -rf $(KLIB)/$(KMODDIR)/net/rfkill/
	@rm -rf $(KLIB)/$(KMODDIR)/net/wireless/
	@rm -rf $(KLIB)/$(KMODDIR)/drivers/ssb/
	@rm -rf $(KLIB)/$(KMODDIR)/drivers/net/usb/
	@rm -rf $(KLIB)/$(KMODDIR)/drivers/net/wireless/
	@# Lets only remove the stuff we are sure we are providing
	@# on the misc directory.
	@rm -f $(KLIB)/$(KMODDIR)/drivers/misc/eeprom/eeprom_93cx6.ko*
	@rm -f $(KLIB)/$(KMODDIR)/drivers/misc/eeprom_93cx6.ko*
	@rm -f $(KLIB)/$(KMODDIR)/drivers/net/b44.ko*
	@/sbin/depmod -a
	@echo
	@echo "Your old wireless subsystem modules were left intact:"
	@echo 
	@$(MODPROBE) -l mac80211
	@$(MODPROBE) -l cfg80211
	@$(MODPROBE) -l lib80211
	@$(MODPROBE) -l adm8211
	@$(MODPROBE) -l ar9170usb
	@$(MODPROBE) -l at76c50x-usb
	@$(MODPROBE) -l ath
	@$(MODPROBE) -l ath5k
	@$(MODPROBE) -l ath9k
	@$(MODPROBE) -l b43
	@$(MODPROBE) -l b43legacy
	@$(MODPROBE) -l b44
	@$(MODPROBE) -l cdc_ether
	@$(MODPROBE) -l eeprom_93cx6
	@$(MODPROBE) -l ipw2100
	@$(MODPROBE) -l ipw2200
	@$(MODPROBE) -l iwl3945
	@$(MODPROBE) -l iwlagn
	@$(MODPROBE) -l iwlcore
	@$(MODPROBE) -l lib80211_crypt_ccmp
	@$(MODPROBE) -l lib80211_crypt_tkip
	@$(MODPROBE) -l lib80211_crypt_wep
	@$(MODPROBE) -l libertas
	@$(MODPROBE) -l libertas_cs
	@$(MODPROBE) -l libertas_sdio
	@$(MODPROBE) -l libertas_spi
	@$(MODPROBE) -l libertas_tf
	@$(MODPROBE) -l libertas_tf_usb
	@$(MODPROBE) -l libipw
	@$(MODPROBE) -l mac80211_hwsim
	@$(MODPROBE) -l mwl8k
	@$(MODPROBE) -l p54common
	@$(MODPROBE) -l p54pci
	@$(MODPROBE) -l p54spi
	@$(MODPROBE) -l p54usb
	@$(MODPROBE) -l rndis_host
	@$(MODPROBE) -l rndis_wlan
	@$(MODPROBE) -l rt2400pci
	@$(MODPROBE) -l rt2500pci
	@$(MODPROBE) -l rt2500usb
	@$(MODPROBE) -l rt2x00lib
	@$(MODPROBE) -l rt2x00pci
	@$(MODPROBE) -l rt2x00usb
	@$(MODPROBE) -l rt61pci
	@$(MODPROBE) -l rt73usb
	@$(MODPROBE) -l rtl8180
	@$(MODPROBE) -l rtl8187
	@$(MODPROBE) -l ssb
	@$(MODPROBE) -l usb8xxx
	@$(MODPROBE) -l usbnet
	@$(MODPROBE) -l zd1211rw
	@echo
	@echo "Your old bluetooth subsystem modules were left intact:"
	@echo
	@$(MODPROBE) -l sco
	@$(MODPROBE) -l l2cap
	@$(MODPROBE) -l hidp
	@$(MODPROBE) -l rfcomm
	@$(MODPROBE) -l bnep
	@$(MODPROBE) -l btusb
	@$(MODPROBE) -l bluetooth
	@
	@echo 

clean:
	@if [ -d net -a -d $(KLIB_BUILD) ]; then \
		$(MAKE) -C $(KLIB_BUILD) M=$(PWD) clean ;\
	fi
	@rm -f $(CREL_PRE)*
unload:
	@./scripts/unload.sh

load: unload
	@./scripts/load.sh

btunload:
	@./scripts/btunload.sh

btload: btunload
	@./scripts/btload.sh

wlunload:
	@./scripts/wlunload.sh

wlload: wlunload
	@./scripts/wlload.sh

.PHONY: all clean install uninstall unload load btunload btload wlunload wlload modules bt

endif

clean-files += Module.symvers Module.markers modules modules.order
clean-files += $(CREL_CHECK) $(CONFIG_CHECK)
