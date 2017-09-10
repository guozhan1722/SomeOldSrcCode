#
# Copyright (C) 2010 Microhard Systems Inc
#
#

define Profile/VIP4215800XEXP
  NAME:=VIP4215800XEXP
  PACKAGES:=kmod-madwifi
endef

define Profile/VIP4215800XEXP/Description
	Microhard IXP4xx based standard board with madwifi driver Profile
endef
$(eval $(call Profile,VIP4215800XEXP))

