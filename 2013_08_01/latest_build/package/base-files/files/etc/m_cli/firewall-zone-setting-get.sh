#!/bin/sh
. /usr/lib/webif/webif.sh
./etc/m_cli/mklink.sh
config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		forwarding)
			append forwarding_cfgs "$cfg_name"
		;;
		zone)
			append zone_cfgs "$cfg_name" "$N"
		;;
		rule)
			append rule_cfgs "$cfg_name" "$N"
		;;
		redirect)
			append redirect_cfgs "$cfg_name" "$N"
		;;
		interface)
			if [ "$cfg_name" != "loopback" ]; then
				append networks "option|$cfg_name" "$N"
                                append netmode "$cfg_name" "$N"
			fi
		;;
	esac
}

uci_load firewall
uci_load network

for zone in $zone_cfgs; do
   #bridge mode
   if [ "$netmode" = "lan" ]; then
      config_get FORM_name $zone name
      if [ "$FORM_name" = "lan" ]; then
         config_get FORM_name $zone name
         config_get FORM_input $zone input
         config_get FORM_output $zone output
         config_get FORM_forward $zone forward
         config_get_bool FORM_masq $zone masq 0
         config_get_bool FORM_mtu_fix $zone mtu_fix 0
         echo ""
         echo "Firewall Status | $FORM_name Zone Settings"
         echo -e "\tInput    \t$FORM_input"
         echo -e "\tOutput   \t$FORM_output"
         echo -e "\tForward  \t$FORM_forward"
         echo -e "\tNAT      \t$FORM_masq"
         echo -e "\tMTU Fix  \t$FORM_mtu_fix"

      fi
   else
      config_get FORM_name $zone name
      config_get FORM_input $zone input
      config_get FORM_output $zone output
      config_get FORM_forward $zone forward
      config_get_bool FORM_masq $zone masq 0
      config_get_bool FORM_mtu_fix $zone mtu_fix 0
      echo ""
      echo "Firewall Status | $FORM_name Zone Settings"
      echo -e "\tInput    \t$FORM_input"
      echo -e "\tOutput   \t$FORM_output"
      echo -e "\tForward  \t$FORM_forward"
      echo -e "\tNAT      \t$FORM_masq"
      echo -e "\tMTU Fix  \t$FORM_mtu_fix"

   fi
done

