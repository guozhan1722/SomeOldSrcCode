#!/bin/sh

#This is the common script for get or set uci value according to 
#the file name . you can simple create a soft link file using 
#using "s-network-lan-type.sh" format to define a action for the uci

. /usr/lib/webif/webif.sh
. /usr/lib/webif/functions.sh
. /etc/functions.sh

show_dns(){
   dns=""
   x=0
   if [ -s /etc/resolv.conf ]; then
      linecount=`cat /etc/resolv.conf 2>&1 | grep nameserver | wc -l | tr -d ' '` 
      while [ "$x" -lt "$linecount" ]
      do
         let x=x+1
         dns=`cat /etc/resolv.conf |grep nameserver| head -n $x | tail -n 1 | awk '{print $2}'`
         echo -e "DNS Servers : \t$dns"
      done
   else
      echo -e "DNS Servers : \t$dns"
   fi
}

#echo "command line is $0"
cmd=$(echo "$0" |awk 'FS="/" {print $4}')
#echo "cmd is $cmd"
action=$(echo "$cmd" | awk 'FS="-" {print $1}')
#echo "action is $action" 

first_col=$(echo "$cmd" | awk 'FS="-" {print $2}')
#echo "first_col is $first_col" 

second_l=$(echo "$cmd" | awk 'FS="-" {print $3}')
if [ "$second_l" = "wlan0" ];then
   second_col="@wifi-iface[0]"
else
   second_col=$second_l
fi

third_col=$(echo "$cmd" | awk 'FS="-" {print $4}')
#echo "third_col is $third_col"

value=$(echo "$cmd" | awk 'FS="-" {print $5}')
#echo "set the value is $value"

if [ "$action" = "set" ];then
#	echo "action is  setting"

      if [ "$second_col" = "com1" ];then
         if [ "`uci get comport.$second_col.COM1_Port_Status`" = "A" ];then
            if [ "$third_col" != "COM1_Port_Status" ]; then
               echo "Com1 port is disabled. Cannot set Com1"   
               exit 0	
            fi
         fi
      fi


       #for mesh setting
        if [ "$third_col" = "mesh_id" ] ;then
            uci $action $first_col.$second_col.mesh
        fi

	if [ "$value" != "" ]; then 	
		uci $action $first_col.$second_col.$third_col=$value
		if [ "$?" = "0" ]; then
			done=1
		else
			done=0
		fi
	else
		if [ "$1" != "" ];then
                     #check the validation of the value input by user
                     case "$third_col" in
                     
                     "ipaddr" \
                     |"netmask" \
                     |"dns" \
                     |"gateway" \
                     |"COM1_T_Client_Server_Addr" \
                     |"COM1_U_PtoP_Remote_Addr" \
                     |"COM1_UM_P_Multicast_Addr" \
                     |"COM1_UM_M_Remote_Addr" \
                     |"COM1_UM_M_Multicast_Addr" \
                     |"COM1_UMTOM_Multicast_Addr" \
                     |"COM1_UMTOM_Listen_Multicast_Addr")

                        if [ "$first_col" = "network" ];then 
                           if [ "`uci get $first_col.$second_col.proto`" = "dhcp" ];then
                              echo "cannot set this item for dhcp mode"
                              exit 1
                           fi
                        fi

                        validate "ip|$third_col|||$1" 
                        if [ "$?" = "0" ]; then 
                           uci $action $first_col.$second_col.$third_col=$1
                           if [ "$?" = "0" ]; then
                               done=1
                           else
                               done=0
                           fi
                         else
                           echo "input value is invalid "
                         fi
                     ;;

                      "COM1_Pre_Data_Delay" \
                     |"COM1_Post_Data_Delay" \
                     |"COM1_Character_Timeout" \
                     |"COM1_Max_Packet_Len" \
                     |"COM1_Modbus_Protect_Key" \
                     |"COM1_T_Client_Timeout" \
                     |"COM1_T_Server_Timeout" \
                     |"COM1_UM_P_TTL" \
                     |"COM1_UMTOM_Multicast_TTL")
                        validate "int|$third_col|||$1" 
                        if [ "$?" = "0" ]; then 
                           uci $action $first_col.$second_col.$third_col=$1
                           if [ "$?" = "0" ]; then
                               done=1
                           else
                               done=0
                           fi
                         else
                           echo "input value is invalid "
                         fi
                     ;;

                     "COM1_T_Client_Server_Port" \
                     |"COM1_T_Server_Listen_Port" \
                     |"COM1_U_PtoP_Remote_Port" \
                     |"COM1_U_PtoP_Listen_Port" \
                     |"COM1_UM_P_Multicast_Port" \
                     |"COM1_UM_P_Listen_Port" \
                     |"COM1_UM_M_Remote_Port" \
                     |"COM1_UM_M_Multicast_Port" \
                     |"COM1_UMTOM_Multicast_Port" \
                     |"COM1_UMTOM_Listen_Multicast_Port")
                        validate "port|$third_col|||$1" 
                        if [ "$?" = "0" ]; then 
                           uci $action $first_col.$second_col.$third_col=$1
                           if [ "$?" = "0" ]; then
                               done=1
                           else
                               done=0
                           fi
                         else
                           echo "input value is invalid "
                         fi
                     ;;

                     *)
                       uci $action $first_col.$second_col.$third_col=$1
                       done=1
                     ;;
                     esac
		else
			echo "no value can be set"
			done=0
		fi
	fi
else
	if [ "$action" = "get" ] ; then
#		echo "action is getting"
            
               case "$second_col" in 
               "com1")
                  case "$third_col" in 
                    "COM1_Port_Status" \
                  | "COM1_NoConnect_Data_Intake" \
                  | "COM1_MODBUS_Mode" \
                  | "COM1_Modbus_Protect_Status")
                  if [ "`uci get $first_col.$second_col.$third_col`" = "A" ];then
                        echo "$third_col is disabled"   
                        exit 0	
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "B" ];then
                        echo "$third_col is enabled"   
                        exit 0
                  else
                        echo "Invalid command"                             
                  	exit 1
                  fi
                  ;;

                  "COM1_Chanel_Mode")
                  if [ "`uci get $first_col.$second_col.$third_col`" = "A" ];then
                        echo "Com1 channel mode is RS232"   
                        exit 0	
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "B" ];then
                        echo "Com1 channel mode is RS485"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "C" ];then
                        echo "Com1 channel mode is RS422"   
                        exit 0
                  else
                        echo "Invalid command"                             
                  	exit 1
                  fi
                  ;;

                  "COM1_Data_Baud_Rate")
                  if [ "`uci get $first_col.$second_col.$third_col`" = "A" ];then
                        echo "Com1 Baudrate is 300"   
                        exit 0	
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "B" ];then
                        echo "Com1 Baudrate is 600"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "C" ];then
                        echo "Com1 Baudrate is 1200"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "D" ];then
                        echo "Com1 Baudrate is 2400"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "E" ];then
                        echo "Com1 Baudrate is 3600"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "F" ];then
                        echo "Com1 Baudrate is 4800"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "G" ];then
                        echo "Com1 Baudrate is 7200"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "H" ];then
                        echo "Com1 Baudrate is 9600"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "I" ];then
                        echo "Com1 Baudrate is 14400"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "J" ];then
                        echo "Com1 Baudrate is 19200"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "K" ];then
                        echo "Com1 Baudrate is 28800"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "L" ];then
                        echo "Com1 Baudrate is 38400"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "M" ];then
                        echo "Com1 Baudrate is 57600"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "N" ];then
                        echo "Com1 Baudrate is 115200"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "O" ];then
                        echo "Com1 Baudrate is 230400"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "P" ];then
                        echo "Com1 Baudrate is 460800"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "Q" ];then
                        echo "Com1 Baudrate is 921600"   
                        exit 0
                  else
                        echo "Invalid command"                             
                  	exit 1
                  fi
                  ;;

                  "COM1_Data_Format")
                  if [ "`uci get $first_col.$second_col.$third_col`" = "A" ];then
                        echo "$third_col is 8N1"   
                        exit 0	
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "B" ];then
                        echo "$third_col is 8N2"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "C" ];then
                        echo "$third_col is 8E1"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "D" ];then
                        echo "$third_col is 8O1"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "E" ];then
                        echo "$third_col is 7N1"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "F" ];then
                        echo "$third_col is 7N2"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "G" ];then
                        echo "$third_col is 7E1"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "H" ];then
                        echo "$third_col is 7O1"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "I" ];then
                        echo "$third_col is 7E2"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "J" ];then
                        echo "$third_col is 7O2"   
                        exit 0
                  else
                        echo "Invalid command"                             
                  	exit 1
                  fi
                  ;;

                  "COM1_Flow_Control")
                  if [ "`uci get $first_col.$second_col.$third_col`" = "A" ];then
                        echo "Flow Control is None"   
                        exit 0	
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "B" ];then
                        echo "Flow Control is Hardware"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "C" ];then
                        echo "Flow Control is CTS Framing"   
                        exit 0
                  else
                        echo "Invalid command"                             
                  	exit 1
                  fi
                  ;;

                  "COM1_Data_Mode")
                  if [ "`uci get $first_col.$second_col.$third_col`" = "A" ];then
                        echo "Data Mode is Seamless"   
                        exit 0	
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "B" ];then
                        echo "Data Mode is Transparent"   
                        exit 0
                  else
                        echo "Invalid command"                             
                  	exit 1
                  fi
                  ;;

                  "COM1_QoS")
                  if [ "`uci get $first_col.$second_col.$third_col`" = "A" ];then
                        echo "Priority is Normal"   
                        exit 0	
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "B" ];then
                        echo "Proority is Medium"   
                        exit 0
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "C" ];then
                        echo "Proority is High"   
                        exit 0
                  else
                        echo "Invalid command"                             
                  	exit 1
                  fi
                  ;;

                  "COM1_IP_Protocol")
                  if [ "`uci get $first_col.$second_col.$third_col`" = "A" ];then
                        echo "IP Protocol is TCP Client"   
                        exit 0	
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "B" ];then
                        echo "IP Protocol is TCP Server"   
                        exit 0	
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "C" ];then
                        echo "IP Protocol is TCP Client_Server"   
                        exit 0	
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "D" ];then
                        echo "IP Protocol is UDP Point to Point"   
                        exit 0	
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "E" ];then
                        echo "IP Protocol is UDP Point to Multipoint(P)"   
                        exit 0	
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "F" ];then
                        echo "IP Protocol is UDP Point to Multipoint(MP)"   
                        exit 0	
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "G" ];then
                        echo "IP Protocol is UDP Multipoint to Multipoint"   
                        exit 0	
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "H" ];then
                        echo "IP Protocol is SMTP Client"   
                        exit 0	
                  elif [ "`uci get $first_col.$second_col.$third_col`" = "I" ];then
                        echo "IP Protocol is C12.22"   
                        exit 0	
                  else
                        echo "Invalid command"                             
                  	exit 1
                  fi
                  ;;

                  *) ;;
                  esac
               ;;
               *)
               ;;
               esac

               case "$third_col" in
               
               "ipaddr")
                  ipaddr="`ifconfig "br-$second_col" |grep "inet addr"|awk '{print $2}'|awk -F ":" '{print $2}'`"
                  echo -e "IP address  : \t$ipaddr"
                  exit 0	
               ;;
               "netmask")
                  netmask="`ifconfig "br-$second_col" |grep "Mask"|awk '{print $4}'|awk -F ":" '{print $2}'`"
                  echo -e "net mask    : \t$netmask"	
                  exit 0
               ;;
               "dns")
                  show_dns
                  exit 0
               ;;
               
               "disabled")
                  if [ "`uci get $first_col.$second_col.disabled`" = "1" ];then
                     echo "$second_col power off"
                     exit 0
                  fi

                  if [ "`uci get $first_col.$second_col.disabled`" = "0" ];then
                     echo "$second_col power on"
                     exit 0
                  fi
               ;;

               *)
               ;;
               esac
                result=`uci $action $first_col.$second_col.$third_col 2>/dev/null`
                if [ "$result" = "" ];then     
                        echo "Invalid command"                             
                else                                                  
                        echo -e "$third_col : \t$result"   
                fi                                        
		done=0
	else
		echo " other action "
		done=0
	fi
fi

if [ "$done" = "1" ]; then

        uci commit $first_col
	case "$first_col" in
	"network")
           echo "setting network ... "
           /etc/init.d/network restart  >/dev/null 2>&1
           sleep 3
           killall dnsmasq 
            if [ -f /etc/rc.d/S??dnsmasq ]; then
               /etc/init.d/dnsmasq start & >/etc/null 2>&1
            echo ""
            fi
	    ;;
	"wireless")
            echo "setting wireless ... "
            wifi >/dev/null 2>&1
            ;;
	"comport")
            echo "setting comport ... "
            /etc/init.d/soipd1 stop >/dev/null 2>&1
            sleep 1
            /etc/init.d/soipd1 start >/dev/null 2>&1
            ;;
        esac

	echo "done !"
        exit 0
else
	echo "  "
	exit 1
fi
