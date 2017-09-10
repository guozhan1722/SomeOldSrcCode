#
# Copyright (C) 2010 Microhard Systems Inc
#
#

define Profile/VIP421900EXP
  NAME:=VIP421900EXP
  PACKAGES:=kmod-madwifi
endef

define Profile/VIP421900EXP/Description
	Microhard IXP4xx based standard board with madwifi driver Profile
endef
$(eval $(call Profile,VIP421900EXP))

