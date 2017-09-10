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

if [ "$netmode" != "lan" ]; then
   echo ""
   echo "Firewall Status | Port Forwarding "
   for rule in $redirect_cfgs; do
      config_get FORM_protocol "$rule" proto
      config_get FORM_src_ip "$rule" src_ip
      config_get FORM_dest_ip "$rule" dest_ip
      config_get FORM_src_dport "$rule" src_dport
      config_get FORM_dest_port "$rule" dest_port
      FORM_port_select_redirect=custom

      echo "$rule" |grep -q "cfg*****" && name="" || name="$rule"
      echo -e "\tName            \t$name"
      echo -e "\tProtocol        \t$FORM_protocol"
      echo -e "\tSource IP       \t$FORM_src_ip"
      echo -e "\tDestination Port\t$FORM_src_dport"
      echo -e "\tTo IP Addr      \t$FORM_dest_ip"
      echo -e "\tTo Port         \t$FORM_dest_port"
      echo -e "\t******************************"

   done
else
   echo "No Port Forwarding for bridge mode"
fi

