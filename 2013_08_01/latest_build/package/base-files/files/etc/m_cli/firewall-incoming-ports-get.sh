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

echo ""
echo "Firewall Status | Incoming Ports "

for rule in $rule_cfgs; do
   #config_get FORM_src "$rule" src "wan"
   config_get FORM_src "$rule" src 
   config_get FORM_dest "$rule" dest
   config_get FORM_protocol "$rule" proto
   config_get FORM_src_ip "$rule" src_ip
   if [ "$FORM_src_ip" = "" ]; then
   config_get FORM_src_ip "$rule" src_mac
   fi
   config_get FORM_dest_ip "$rule" dest_ip
   config_get FORM_target "$rule" target "ACCEPT"
   config_get FORM_port "$rule" dest_port
   FORM_port_select_rule=custom

   if [ "$FORM_src" = "" ]; then
      FORM_src=Router
   fi

   if [ "$FORM_dest" = "" ]; then
      FORM_dest=Router
   fi

   echo "$rule" |grep -q "cfg*****" && name="" || name="$rule"

   echo -e "\tName            \t$name"
   echo -e "\tSource          \t$FORM_src"
   echo -e "\tDestination     \t$FORM_dest"
   echo -e "\tProtocol        \t$FORM_protocol"
   echo -e "\tSource IP / MAC \t$FORM_src_ip"
   echo -e "\tDestination IP  \t$FORM_dest_ip"
   echo -e "\tPort            \t$FORM_port"
   echo -e "\tTarget          \t$FORM_target"
   echo -e "\t******************************"
done


