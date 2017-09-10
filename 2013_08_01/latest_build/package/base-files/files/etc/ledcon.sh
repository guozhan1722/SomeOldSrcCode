#!/bin/sh
#led management script for wifi card 

. /etc/functions.sh
. /etc/version

if [ "$HARDWARE" = "v2.0.0" ]; then

        echo "THIS is VIP4G board "
        
        start_rssi_led_flashing() {
                    echo heartbeat > /sys/class/leds/RSSIMIN/trigger
                    echo heartbeat > /sys/class/leds/RSSIMID/trigger
                    echo heartbeat > /sys/class/leds/RSSIMAX/trigger
        } 
          
        led_on()
        {
            lights=$@
            for led in $lights; do
    	        echo 0 > /sys/class/leds/$led/brightness
            done
        }
       
        led_off()
        {
            lights=$@
            for led in $lights; do
    	        echo 1 > /sys/class/leds/$led/brightness
            done
        }
       
        led_flash()
        {
            lights=$@
            for led in $lights; do
                led_status=`cat /sys/class/leds/$led/brightness`
                if [ "$led_status" = "1" ]; then
                    led_on $led 
                else
                    led_off $led 
                fi
            done
        }
    
        led_scan()
        {            
            led_on RSSIMIN 
            led_off RSSIMID RSSIMAX 
            usleep 250000
            led_on RSSIMID
            led_off RSSIMIN RSSIMAX 
            usleep 250000
            led_on RSSIMAX
            led_off RSSIMIN RSSIMID 
            usleep 250000 
            led_off RSSIMIN RSSIMID RSSIMAX
            usleep 500000
        }    
    
        led_upgrade()
        {
            led_off RSSIMIN RSSIMID RSSIMAX
            while [ 1 ]; do
                #echo "led_upgrade"
                led_on RSSIMIN 
                led_off RSSIMID RSSIMAX 
                usleep 500000
                led_on RSSIMIN RSSIMID
                led_off RSSIMAX 
                usleep 500000
                led_on RSSIMIN RSSIMID RSSIMAX             
                usleep 500000             
                led_off RSSIMIN RSSIMID RSSIMAX
                sleep 1
            done
        }
    
        led_reset_flash()
        {
            led_off RSSIMIN RSSIMID RSSIMAX
            x=1
            while [ $x -le 20 ]; do
                #echo "led_upgrade"            
                led_flash RSSIMIN RSSIMID RSSIMAX
                usleep 300000
                x=$(( $x + 1 ))
            done
        } 

        vip4g_show_wifi_rssi()
        {
            #echo "VIP4G board WIFI RSSI indicating..."
            max_level=""
            netdevs=$(ls /sys/class/ieee80211/phy0/device/net)
            for dev in $netdevs; do
              level=`iw $dev station dump |grep signal:|awk '{print $2}'`
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
            #echo "WiFi RSSI max_level=$max_level"
            
            
            if [ "$max_level" = "" ]; then
               led_scan
            else                
            
               if [ "$max_level" -lt "-106" ];then
                  led_off RSSIMIN RSSIMID RSSIMAX   
               elif [ "$max_level" -gt "-106" -a "$max_level" -lt "-102" ];then
                  led_off RSSIMID RSSIMAX   
                  led_flash RSSIMIN
               elif [ "$max_level" -gt "-102" -a "$max_level" -lt "-98" ];then
                  led_off RSSIMID RSSIMAX   
                  led_on RSSIMIN
               elif [ "$max_level" -gt "-98" -a "$max_level" -lt "-94" ];then
                  led_off RSSIMAX
                  led_flash RSSIMID   
                  led_on RSSIMIN
               elif [ "$max_level" -gt "-94" -a "$max_level" -lt "-90" ];then
                  led_off RSSIMAX    
                  led_on RSSIMIN RSSIMID
               elif [ "$max_level" -gt "-90" -a "$max_level" -lt "-86" ];then
                  led_flash RSSIMAX   
                  led_on RSSIMIN RSSIMID
               elif [ "$max_level" -gt "-86" ];then
                  led_on RSSIMIN RSSIMID RSSIMAX 
               else
                  echo " wrong max_level"
               fi
               sleep 1 
            fi

        } #endof func vip4g_show_wifi_rssi 
       
        if [ $1 = "upgrade" ]; then
                echo "upgrading VIP4G firmware..."
                led_upgrade         
        elif [ $1 = "reset" ]; then
                echo "reset VIP4G board..."
                led_reset_flash   
        elif [ $1 = "lte_rssi" ]; then
                echo "VIP4G board LTE RSSI indicating..."
                while [ 1 ]; do
                    config_load lte
                    config_get lte_disabled connect disabled "0"
                    if [ "$lte_disabled" = "0" ]; then
                    
                        connect_stat="_DISCONNECTED"
                        connect_stat=$(cat /var/run/VIP4G_status | grep "_CONNECTED")
                        havesim=$(cat /sys/class/button/SIM_STAT/status)
                        
                        if [ -n "$connect_stat" ]; then
                        
                             max_level=$(cat /var/run/VIP4G_status | grep "rssi=" | awk -F ":" '{print $2+0}')
                             # echo "orig rssi=$rssi max_level=$max_level"
                             if [ "$max_level" -gt "113" ];then
                                #led_off RSSIMIN RSSIMID RSSIMAX
                                led_scan 
                             elif [ "$max_level" -gt "110" -a "$max_level" -le "115" ];then
                                led_off RSSIMID RSSIMAX 
                                led_flash RSSIMIN
                             elif [ "$max_level" -gt "105" -a "$max_level" -le "110" ];then
                                led_off  RSSIMID RSSIMAX    
                                led_on RSSIMIN
                             elif [ "$max_level" -gt "100" -a "$max_level" -le "105" ];then
                                led_off RSSIMAX 
                                led_on RSSIMIN
                                led_flash RSSIMID
                             elif [ "$max_level" -gt "95" -a "$max_level" -le "100" ];then
                                led_off RSSIMAX 
                                led_on RSSIMIN RSSIMID
                             elif [ "$max_level" -gt "90" -a "$max_level" -le "95" ];then
                                led_flash RSSIMAX
                                led_on RSSIMIN RSSIMID
                             elif [ "$max_level" -le "90" ];then
                                led_on RSSIMIN RSSIMID RSSIMAX  
                             else
                                #echo " wrong rssi level"
                                led_scan
                             fi                                                 
                             
                        elif [ "$havesim" = "1" ]; then       
                              #logger "*** sim card pulled out from slot when system running ***"                           
                              led_flash RSSIMIN RSSIMID RSSIMAX
                        else
                             #echo "RSSI scan: lte.status.connect_status=$connect_stat"
                             led_scan
                        fi
                    else
                        #echo "off all led if carrier disabled"
                        led_off RSSIMIN RSSIMID RSSIMAX
                        #vip4g_show_wifi_rssi
                    fi    
                    sleep 1
                done
        fi
    
    exit 0

else  #other than VIP4G hardware
    
    led_on()
    {
       lights=$@
       for led in $lights; do
          led_status=`mpci -gpio get $led`
          if [ "$led_status" != "GPIO$led is HIGH." ]; then
             mpci -gpio set $led 1
          fi
       done
    }
    
    led_off()
    {
       lights=$@
       for led in $lights; do
          led_status=`mpci -gpio get $led`
          if [ "$led_status" = "GPIO$led is HIGH." ]; then
             mpci -gpio set $led 0
          fi
       done
    }
    
    led_flash()
    {
       lights=$@
       for led in $lights; do
          led_status=`mpci -gpio get $led`
          if [ "$led_status" = "GPIO$led is HIGH." ]; then
             mpci -gpio set $led 0
          else
             mpci -gpio set $led 1
          fi
       done
    }
    
    led_scan()
    {
       sleep 1
       led_on 4
       led_off 5 6
       sleep 1
       led_on 5
       led_off 4 6
       sleep 1
       led_on 6
       led_off 4 5
    }
    
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
          led_scan
       else
    
          if [ "$max_level" -lt "-106" ];then
             led_off 4 5 6   
          elif [ "$max_level" -gt "-106" -a "$max_level" -lt "-102" ];then
             led_off 5 6   
             led_flash 4
          elif [ "$max_level" -gt "-102" -a "$max_level" -lt "-98" ];then
             led_off 5 6   
             led_on 4
          elif [ "$max_level" -gt "-98" -a "$max_level" -lt "-94" ];then
             led_off 6
             led_flash 5   
             led_on 4
          elif [ "$max_level" -gt "-94" -a "$max_level" -lt "-90" ];then
             led_off 6   
             led_on 4 5
          elif [ "$max_level" -gt "-90" -a "$max_level" -lt "-86" ];then
             led_flash 6   
             led_on 4 5
          elif [ "$max_level" -gt "-86" ];then
             led_on 4 5 6
          else
             echo " wrong max_level"
          fi
          sleep 1 
       fi
    done
    exit 0
    
fi
