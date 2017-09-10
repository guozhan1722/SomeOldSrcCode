#
# Copyright (C) 2010 Microhard Systems Inc
#
#

define Profile/VIP4215800X
  NAME:=VIP4215800X
  PACKAGES:=kmod-madwifi
endef

define Profile/VIP4215800X/Description
	Microhard IXP4xx based standard board with madwifi driver Profile
endef
$(eval $(call Profile,VIP4215800X))

