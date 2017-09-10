#
# Copyright (C) 2010 Microhard Systems Inc
#
#

define Profile/VIP4212400X
  NAME:=VIP4212400X
  PACKAGES:=kmod-madwifi
endef

define Profile/VIP4212400X/Description
	Microhard IXP4xx based standard board with madwifi driver Profile
endef
$(eval $(call Profile,VIP4212400X))

