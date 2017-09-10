#!/bin/sh
#led management script for wifi card 
. /etc/version

if [ "$HARDWARE" = "v1.0.0" ]; then
   echo "THIS is NanoPCI board"

elif [ "$HARDWARE" = "5.0.0" -o "$HARDWARE" = "3.0.0" ]; then
   echo "THIS is IPMesh rev5 board or SVIP board"

   [ -z "$#" ] && {
	echo "requires phy id"
	exit 0
   }
      
   led_on()
   {
      lights=$@
      for led in $lights; do
	echo 1 > /sys/class/leds/$led/brightness
      done
   }
   
   led_off()
   {
      lights=$@
      for led in $lights; do
	echo 0 > /sys/class/leds/$led/brightness
      done
   }
   
   led_flash()
   {
      lights=$@
      for led in $lights; do
         led_status=`cat /sys/class/leds/$led/brightness`
         if [ "$led_status" = "0" ]; then
            led_on $led 
         else
            led_off $led 
         fi
      done
   }

   led_scan()
   {
      phys=$@
      led_on ${phys}_rssi1 
      led_off ${phys}_rssi2 ${phys}_rssi3
      usleep 250000
      led_on ${phys}_rssi2
      led_off ${phys}_rssi1 ${phys}_rssi3
      usleep 250000
      led_on ${phys}_rssi3
      led_off ${phys}_rssi1 ${phys}_rssi2
      usleep 250000
      led_off ${phys}_rssi3
   }

   phy=$1

   while [ 1 ]; do
      max_level=""
      netdevs=$(ls /sys/class/ieee80211/$phy/device/net)
      for dev in $netdevs; do
	level=`iw $dev station dump |grep signal|awk '{print $2}'`
        if [ "$level" != "" ]; then
        	mmlevel=""
        	for mlevel in $level; do
        	   [ "$mmlevel" = "" ] && {
        	   	eval mmlevel=$mlevel
        	   } || {
        	   	[ "$mlevel" -gt "$mmlevel" ] && {
        	   		eval mmlevel=$mlevel
        	   	}
        	   }
        	done
        
        	level=$mmlevel
        	[ "level" = "" ] || {
        	   if [ "$max_level" = "" ]; then
        	      eval max_level=$level
                   else
        	      [ "$level" -ge "$max_level" ] && {
        	         eval max_level=$level
        	      }
                   fi
        	}
        fi
      done
          
      if [ "$max_level" = "" ]; then
         led_scan $phy
      else
   
         if [ "$max_level" -le "-106" ];then
            led_off ${phy}_rssi1 ${phy}_rssi2 ${phy}_rssi3
         elif [ "$max_level" -gt "-106" -a "$max_level" -le "-102" ];then
            led_off  ${phy}_rssi2 ${phy}_rssi3   
            led_flash ${phy}_rssi1
         elif [ "$max_level" -gt "-102" -a "$max_level" -le "-98" ];then
            led_off ${phy}_rssi2 ${phy}_rssi3
            led_on ${phy}_rssi1
         elif [ "$max_level" -gt "-98" -a "$max_level" -le "-94" ];then
            led_off ${phy}_rssi3
            led_flash ${phy}_rssi2
            led_on ${phy}_rssi1
         elif [ "$max_level" -gt "-94" -a "$max_level" -le "-90" ];then
            led_off ${phy}_rssi3
            led_on ${phy}_rssi2 ${phy}_rssi1
         elif [ "$max_level" -gt "-90" -a "$max_level" -le "-86" ];then
            led_flash ${phy}_rssi3  
            led_on ${phy}_rssi2 ${phy}_rssi1
         elif [ "$max_level" -gt "-86" ];then
            led_on ${phy}_rssi3 ${phy}_rssi2 ${phy}_rssi1 
         else
            echo " wrong level"
         fi
      fi

      sleep 1

   done
   
else
   echo "THIS is normal vip board "

   [ -z "$#" ] && {
	echo "requires phy id"
	exit 0
   }
      
   led_on()
   {
      lights=$@
      for led in $lights; do
	echo 1 > /sys/class/leds/$led/brightness
      done
   }
   
   led_off()
   {
      lights=$@
      for led in $lights; do
	echo 0 > /sys/class/leds/$led/brightness
      done
   }
   
   led_flash()
   {
      lights=$@
      for led in $lights; do
         led_status=`cat /sys/class/leds/$led/brightness`
         if [ "$led_status" = "0" ]; then
            led_on $led 
         else
            led_off $led 
         fi
      done
   }

   led_scan()
   {
      phys=$@
      led_on ${phys}_rssi1 
      led_off ${phys}_rssi2 ${phys}_rssi3 ${phys}_rssi4 ${phys}_rssi5 ${phys}_rssi6
      usleep 250000
      led_on ${phys}_rssi2
      led_off ${phys}_rssi1 ${phys}_rssi3 ${phys}_rssi4 ${phys}_rssi5 ${phys}_rssi6
      usleep 250000
      led_on ${phys}_rssi3
      led_off ${phys}_rssi1 ${phys}_rssi2 ${phys}_rssi4 ${phys}_rssi5 ${phys}_rssi6
      usleep 250000
      led_on ${phys}_rssi4
      led_off ${phys}_rssi1 ${phys}_rssi2 ${phys}_rssi3 ${phys}_rssi5 ${phys}_rssi6
      usleep 250000
      led_on ${phys}_rssi5
      led_off ${phys}_rssi1 ${phys}_rssi2 ${phys}_rssi3 ${phys}_rssi4 ${phys}_rssi6
      usleep 250000
      led_on ${phys}_rssi6
      led_off ${phys}_rssi1 ${phys}_rssi2 ${phys}_rssi3 ${phys}_rssi4 ${phys}_rssi5
      usleep 250000
      led_off ${phys}_rssi1 ${phys}_rssi2 ${phys}_rssi3 ${phys}_rssi4 ${phys}_rssi5 ${phys}_rssi6
   }

   phy=$1

   while [ 1 ]; do
      max_level=""
      netdevs=$(ls /sys/class/ieee80211/$phy/device/net)
      for dev in $netdevs; do
	level=`iw $dev station dump |grep signal|awk '{print $2}'`
        if [ "$level" != "" ]; then
        	mmlevel=""
        	for mlevel in $level; do
        	   [ "$mmlevel" = "" ] && {
        	   	eval mmlevel=$mlevel
        	   } || {
        	   	[ "$mlevel" -gt "$mmlevel" ] && {
        	   		eval mmlevel=$mlevel
        	   	}
        	   }
        	done
        
        	level=$mmlevel
        	[ "level" = "" ] || {
        	   if [ "$max_level" = "" ]; then
        	      eval max_level=$level
                   else
        	      [ "$level" -ge "$max_level" ] && {
        	         eval max_level=$level
        	      }
                   fi
        	}
        fi
      done
          
      if [ "$max_level" = "" ]; then
         led_scan $phy
      else
   
         if [ "$max_level" -le "-106" ];then
            led_off ${phy}_rssi1 ${phy}_rssi2 ${phy}_rssi3 ${phy}_rssi4 ${phy}_rssi5 ${phy}_rssi6
         elif [ "$max_level" -gt "-106" -a "$max_level" -le "-104" ];then
            led_off ${phy}_rssi2 ${phy}_rssi3 ${phy}_rssi4 ${phy}_rssi5 ${phy}_rssi6
            led_flash ${phy}_rssi1
         elif [ "$max_level" -gt "-104" -a "$max_level" -le "-102" ];then
            led_off  ${phy}_rssi2 ${phy}_rssi3 ${phy}_rssi4 ${phy}_rssi5 ${phy}_rssi6   
            led_on ${phy}_rssi1
         elif [ "$max_level" -gt "-102" -a "$max_level" -le "-100" ];then
            led_off ${phy}_rssi3 ${phy}_rssi4 ${phy}_rssi5 ${phy}_rssi6
            led_on ${phy}_rssi1
            led_flash ${phy}_rssi2
         elif [ "$max_level" -gt "-100" -a "$max_level" -le "-98" ];then
            led_off ${phy}_rssi3 ${phy}_rssi4 ${phy}_rssi5 ${phy}_rssi6
            led_on ${phy}_rssi1 ${phy}_rssi2
         elif [ "$max_level" -gt "-98" -a "$max_level" -le "-96" ];then
            led_off ${phy}_rssi4 ${phy}_rssi5 ${phy}_rssi6
            led_flash ${phy}_rssi3
            led_on ${phy}_rssi1 ${phy}_rssi2
         elif [ "$max_level" -gt "-96" -a "$max_level" -le "-94" ];then
            led_off ${phy}_rssi4 ${phy}_rssi5 ${phy}_rssi6
            led_on ${phy}_rssi1 ${phy}_rssi2 ${phy}_rssi3
         elif [ "$max_level" -gt "-94" -a "$max_level" -le "-92" ];then
            led_off ${phy}_rssi5 ${phy}_rssi6
            led_flash ${phy}_rssi4
            led_on ${phy}_rssi1 ${phy}_rssi2 ${phy}_rssi3
         elif [ "$max_level" -gt "-92" -a "$max_level" -le "-90" ];then
            led_off ${phy}_rssi5 ${phy}_rssi6 
            led_on ${phy}_rssi1 ${phy}_rssi2 ${phy}_rssi3 ${phy}_rssi4
         elif [ "$max_level" -gt "-90" -a "$max_level" -le "-88" ];then
            led_off ${phy}_rssi6
            led_flash ${phy}_rssi5  
            led_on ${phy}_rssi1 ${phy}_rssi2 ${phy}_rssi3 ${phy}_rssi4
         elif [ "$max_level" -gt "-88" -a "$max_level" -le "-86" ];then
            led_off ${phy}_rssi6  
            led_on ${phy}_rssi1 ${phy}_rssi2 ${phy}_rssi3 ${phy}_rssi4 ${phy}_rssi5 
         elif [ "$max_level" -gt "-86" -a "$max_level" -le "-84" ];then
            led_flash ${phy}_rssi6  
            led_on ${phy}_rssi1 ${phy}_rssi2 ${phy}_rssi3 ${phy}_rssi4 ${phy}_rssi5 
         elif [ "$max_level" -gt "-84" ];then
            led_on ${phy}_rssi1 ${phy}_rssi2 ${phy}_rssi3 ${phy}_rssi4 ${phy}_rssi5 ${phy}_rssi6 
         else
            echo " wrong level"
         fi
      fi

      sleep 1

   done
fi
exit 0
