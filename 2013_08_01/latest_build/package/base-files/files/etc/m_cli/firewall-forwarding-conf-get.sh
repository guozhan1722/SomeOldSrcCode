#!/bin/sh
. /usr/lib/webif/webif.sh

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
echo -e "\tNO. \tfrom \tto"

count=0

for zone in $forwarding_cfgs; do
   count=`expr $count + 1`

   eval zone_$count=$zone
   config_get FORM_src "$zone" src
   config_get FORM_dest "$zone" dest
   #bridge mode
   if [ "$netmode" = "lan" ]; then
      if [ "$FORM_src" = "wan" ]; then
         FORM_src="router"
      fi

      if [ "$FORM_dest" = "wan" ]; then
         FORM_dest="router"
      fi
   fi

   if [ "$FORM_src" = "" ]; then
      FORM_src="router"
   fi

   if [ "$FORM_dest" = "" ]; then
      FORM_dest="router"
   fi

   echo -e "\t$count \t$FORM_src \t$FORM_dest"
done


