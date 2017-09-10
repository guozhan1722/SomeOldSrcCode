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

display() {

   outloop=0
   while [ "$outloop" = "0" ]; do

      echo ""
      if [ "$netmode" = "lan" ]; then
         echo "Please select source :"
         echo -e "\tA) lan   \tB) router \tC) exit"
      else
         echo "Please select source :"
         echo -e "\tA) lan   \tB) wan \tC) router \tD) exit"
      fi

     console_read FORM_src

     echo "your select: $FORM_src"

     if [ "$netmode" = "lan" ]; then
        if [ "$FORM_src" = "A" -o "$FORM_src" = "a" ]; then
           FORM_src=lan
            outloop=1
            eval uci_set firewall "\$$1" src "$FORM_src"
         elif [ "$FORM_src" = "B" -o "$FORM_src" = "b" ]; then
            FORM_src=""
            outloop=1
            eval uci_set firewall "\$$1" src "$FORM_src"
         elif [ "$FORM_src" = "C" -o "$FORM_src" = "c" ]; then
            exit 0
         else
            echo "$FORM_src is Invalidate input, Please try again"
   
            outloop=0
         fi
      else
        if [ "$FORM_src" = "A" -o "$FORM_src" = "a" ]; then
           FORM_src=lan
            outloop=1
            eval uci_set firewall "\$$1" src "$FORM_src"
         elif [ "$FORM_src" = "B" -o "$FORM_src" = "b" ]; then
            FORM_src=wan
            outloop=1
            eval uci_set firewall "\$$1" src "$FORM_src"
         elif [ "$FORM_src" = "C" -o "$FORM_src" = "c" ]; then
            FORM_src=""
            outloop=1
            eval uci_set firewall "\$$1" src "$FORM_src"
         elif [ "$FORM_src" = "D" -o "$FORM_src" = "d" ]; then
            exit 0
         else
            echo "$FORM_src is Invalidate input, Please try again"
            echo ""
            outloop=0
         fi
      fi

   done

   outloop=0

   while [ "$outloop" = "0" ]; do

      echo ""
      if [ "$netmode" = "lan" ]; then
         echo "Please select destination :"
         echo -e "\tA) lan   \tB) router \tC) exit"
      else
         echo "Please select destination :"
         echo -e "\tA) lan   \tB) wan \tC) router \tD) exit"
      fi

      console_read FORM_src

      echo "your select: $FORM_src"

     if [ "$netmode" = "lan" ]; then
        if [ "$FORM_src" = "A" -o "$FORM_src" = "a" ]; then
           FORM_src=lan
            outloop=1
            eval uci_set firewall "\$$1" dest "$FORM_src"
         elif [ "$FORM_src" = "B" -o "$FORM_src" = "b" ]; then
            FORM_src=""
            outloop=1
            eval uci_set firewall "\$$1" dest "$FORM_src"
         elif [ "$FORM_src" = "C" -o "$FORM_src" = "c" ]; then
            exit 0
         else
            echo "$FORM_src is Invalidate input, Please try again"
   
            outloop=0
         fi
      else
        if [ "$FORM_src" = "A" -o "$FORM_src" = "a" ]; then
           FORM_src=lan
            outloop=1
            eval uci_set firewall "\$$1" dest "$FORM_src"
         elif [ "$FORM_src" = "B" -o "$FORM_src" = "b" ]; then
            FORM_src=wan
            outloop=1
            eval uci_set firewall "\$$1" dest "$FORM_src"
         elif [ "$FORM_src" = "C" -o "$FORM_src" = "c" ]; then
            FORM_src=""
            outloop=1
            eval uci_set firewall "\$$1" dest "$FORM_src"
         elif [ "$FORM_src" = "D" -o "$FORM_src" = "d" ]; then
            exit 0
         else
            echo "$FORM_src is Invalidate input, Please try again"
            echo ""
            outloop=0
         fi
      fi

   done
}

uci_load firewall
uci_load network

count=0

for zone in $forwarding_cfgs; do
   count=`expr $count + 1`

   eval zone_$count=$zone
done

echo ""
echo "Add firewall forwarding configuration:"
echo -e "\tA) add   \tB) Exit"

loop_2=0
while [ "$loop_2" = "0" ]; do

   console_read add

   if [ "$add" = "B" -o "$add" = "b" ]; then
      exit 0
   fi

   if [ "$add" = "A" -o "$add" = "a" ]; then
      uci_add firewall forwarding ""; add_forward_cfg="$CONFIG_SECTION"
      display add_forward_cfg   
      echo ""
      echo "forwarding configuration adding is done !"
      exit 0
   else
      echo ""
      echo "Invalidate Input. please try again"
      loop_2=0
   fi
done

