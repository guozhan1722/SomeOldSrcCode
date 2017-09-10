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

console_read() {
      rm -rf /tmp/cli_console_input
      touch /tmp/cli_console_input
      my_read=$1
      my_read=""
      read_cnt=0
      while [ "$my_read" = "" ]; do
         my_read=`cat /tmp/cli_console_input`
         sleep 1
         read_cnt=`expr $read_cnt + 1`
         if [ "$read_cnt" -gt "25" ]; then
            rm -rf /tmp/cli_console_input
            exit 1
         fi
      done
      eval $1=$my_read
      rm -rf /tmp/cli_console_input
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

if [ "$count" -gt "0" ]; then
   echo ""
   echo "Please choose NO. to delete, press X to exit"

   console_read set_no
   
   if [ "$set_no" = "X" -o "$set_no" = "x" ]; then
      exit 0
   fi
   
   validate "int|set_no|||$set_no" 
   if [ "$?" = "0" ]; then 
      if [ "$set_no" -gt "$count" ] ;then
         echo ""
         echo "Invalidate number"
         exit 1
      fi
   else
      echo ""
      echo "Invalidate number"
      exit 1
   fi
   
   echo ""
   echo "The number is $set_no."
   echo -e "\tY ) OK    \tX ) Exit"

   console_read del_ok

   if [ "$del_ok" = "Y" -o "$del_ok" = "y" ];then

      zone=zone_$set_no      
      eval uci_remove firewall "\$$zone"
      echo ""
      echo "forwarding configuration delete is done !"
      exit 0
   else
      echo ""
      echo "Exit without delete"
   fi
else
   echo "no forwarding configuration is set, Please add one first"
fi


		

