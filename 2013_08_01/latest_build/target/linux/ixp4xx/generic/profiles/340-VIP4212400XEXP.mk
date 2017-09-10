#
# Copyright (C) 2010 Microhard Systems Inc
#
#

define Profile/VIP4212400XEXP
  NAME:=VIP4212400XEXP
  PACKAGES:=kmod-madwifi
endef

define Profile/VIP4212400XEXP/Description
	Microhard IXP4xx based standard board with madwifi driver Profile
endef
$(eval $(call Profile,VIP4212400XEXP))

