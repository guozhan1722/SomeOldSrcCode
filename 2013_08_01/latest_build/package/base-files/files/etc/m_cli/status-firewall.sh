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
echo "Firewall Status | Forwarding Configuration "
echo -e "\tAllow traffic originating"
echo -e "\tfrom \tto"

for zone in $forwarding_cfgs; do
   config_get FORM_src "$zone" src
   config_get FORM_dest "$zone" dest
   #bridge mode
   if [ "$netmode" = "lan" ]; then
      if [ "$FORM_src" = "wan" ]; then
         FORM_src=""
      fi

      if [ "$FORM_dest" = "wan" ]; then
         FORM_dest=""
      fi
   fi
   echo -e "\t$FORM_src \t$FORM_dest"
done

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
fi

