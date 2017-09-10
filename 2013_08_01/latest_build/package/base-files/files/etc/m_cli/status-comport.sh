#!/bin/sh
. /usr/lib/webif/webif.sh
###################################################################
# status-comport.sh
#
# Description:
#  Display com port status. 
#
# Major revisions:
#

uci_load comport

show_port(){
if [ "$1" = "com1" ]; then
   port="com1"
   config_get FORM_COM1_Port_Status $port COM1_Port_Status
   config_get FORM_COM1_Data_Baud_Rate $port COM1_Data_Baud_Rate
   config_get FORM_COM1_IP_Protocol $port COM1_IP_Protocol

   com1_str="irq:15"
# here tx and rx look from socket side
#   com1_rx_b=`cat /proc/tty/driver/serial |grep $com1_str|awk {'print $5'}|awk -F ":" {'print $2'}`
#   com1_tx_b=`cat /proc/tty/driver/serial |grep $com1_str|awk {'print $6'}|awk -F ":" {'print $2'}`
   if [ -r "/var/run/com1_packet" ]; then
   com1_connect_status=`cat /var/run/com1_packet | head -n 20 |tail -n 1`
   com1_rx_b=`cat /var/run/com1_packet | head -n 1 |tail -n 1 `
   com1_rx_p=`cat /var/run/com1_packet | head -n 2 |tail -n 1`
   com1_rx_e=`cat /var/run/com1_packet | head -n 3 |tail -n 1`
   com1_rx_d=`cat /var/run/com1_packet | head -n 4 |tail -n 1`
   com1_rx_f=`cat /var/run/com1_packet | head -n 5 |tail -n 1`
   com1_rx_fr=`cat /var/run/com1_packet | head -n 6 |tail -n 1`
   com1_rx_c=`cat /var/run/com1_packet | head -n 7 |tail -n 1`
   com1_rx_m=`cat /var/run/com1_packet | head -n 8 |tail -n 1`

   com1_tx_b=`cat /var/run/com1_packet | head -n 9 |tail -n 1`
   com1_tx_p=`cat /var/run/com1_packet | head -n 10 |tail -n 1`
   com1_tx_e=`cat /var/run/com1_packet | head -n 11 |tail -n 1`
   com1_tx_d=`cat /var/run/com1_packet | head -n 12 |tail -n 1`
   com1_tx_f=`cat /var/run/com1_packet | head -n 13 |tail -n 1`
   com1_tx_co=`cat /var/run/com1_packet | head -n 14 |tail -n 1`
   com1_tx_ca=`cat /var/run/com1_packet | head -n 15 |tail -n 1`
   com1_tx_c=`cat /var/run/com1_packet | head -n 16 |tail -n 1`

   fi

   if [ "$com1_connect_status" = "B" ];then
      com1_connect_status="Active"      
   else  
      com1_connect_status="Not Active"      
   fi

   if [ "$FORM_COM1_Port_Status" = "B" ];then
      FORM_COM1_Port_Status="Enable"      
   else  
      FORM_COM1_Port_Status="Disable"      
   fi

   case "$FORM_COM1_IP_Protocol" in
   "A")
      FORM_COM1_IP_Protocol="TCP Client"
      ;;      
   "B")
      FORM_COM1_IP_Protocol="TCP Server"
      ;;      
   "C")
      FORM_COM1_IP_Protocol="TCP Client/Server"
      ;;      
   "D")
      FORM_COM1_IP_Protocol="UDP Point to Point"
      ;;      
   "E")
      FORM_COM1_IP_Protocol="UDP Point to Multipoint(P)"
      ;;      
   "F")
      FORM_COM1_IP_Protocol="UDP Point to Multipoint(MP)"
      ;;      
   "G")
      FORM_COM1_IP_Protocol="UDP Multipoint to Multipoint"
      ;;      
   "H")
      FORM_COM1_IP_Protocol="SMTP Client"
      ;;      
   "I")
      FORM_COM1_IP_Protocol="C12.22"
      ;;      
   esac

   echo ""
   echo "Interfaces Status | Com1 Port Connection Status"
   echo -e "\tCom1 Port status    \t$FORM_COM1_Port_Status"
   echo -e "\tCom1 Connect As     \t$FORM_COM1_IP_Protocol"
   echo -e "\tCom1 Connect Status \t$com1_connect_status"
   echo "Com1 Port Received Packet Statistics"
   echo -e "\tReceive bytes     \t$com1_rx_b"
   echo -e "\tReceive packets   \t$com1_rx_p"
   echo -e "\tReceive errors    \t$com1_rx_e"
   echo -e "\tDrop packets      \t$com1_rx_d"
   echo -e "\tReceive fifo      \t$com1_rx_f"
   echo -e "\tReceive frame     \t$com1_rx_fr"
   echo -e "\tCompressed        \t$com1_rx_c"
   echo -e "\tReceive multicast \t$com1_rx_m"
   echo "Com1 port Transmitted Packet Statistics"
   echo -e "\tTransmit bytes    \t$com1_tx_b"
   echo -e "\tTransmit packets  \t$com1_tx_p"
   echo -e "\tTransmit errors   \t$com1_tx_e"
   echo -e "\tDrop packets      \t$com1_tx_d"
   echo -e "\tTransmit fifo     \t$com1_tx_f"
   echo -e "\tCollisions        \t$com1_tx_co"
   echo -e "\tTransmit carrier  \t$com1_tx_ca"
   echo -e "\tTransmit compress \t$com1_tx_c"

fi

}
show_port com1
