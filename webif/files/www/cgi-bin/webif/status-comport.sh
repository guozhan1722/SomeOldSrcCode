#!/usr/bin/webif-page
<?
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
   com1_connect_status=`cat /var/run/com1_packet | head -n 20 |tail -n 1`
   com1_rx_b=`cat /var/run/com1_packet | head -n 1 |tail -n 1`
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

display_form <<EOF
   start_form|@TR<<Com1 Port Connection Status>> 
   field|@TR<<Com1 Port status>>
   string|$FORM_COM1_Port_Status

   field|@TR<<Com1 Connect As>>
   string|$FORM_COM1_IP_Protocol

   field|@TR<<Com1 Connect Status>>
   string|$com1_connect_status
   end_form

   start_form|@TR<<Com1 Port Received Packet Statistics>> 
   field|@TR<<Receive bytes>>
   string|$com1_rx_b

   field|@TR<<Receive packets>>
   string|$com1_rx_p

   field|@TR<<Receive errors>>
   string|$com1_rx_e

   field|@TR<<Drop packets>>
   string|$com1_rx_d

   field|@TR<<Receive fifo>>
   string|$com1_rx_f

   field|@TR<<Receive frame>>
   string|$com1_rx_fr
       
   field|@TR<<Compressed>>
   string|$com1_rx_c

   field|@TR<<Receive multicast>>
   string|$com1_rx_m

   end_form

   start_form|@TR<<Com1 port Transmitted Packet Statistics>> 
   field|@TR<<Transmit bytes>>
   string|$com1_tx_b

   field|@TR<<Transmit packets>>
   string|$com1_tx_p

   field|@TR<<Transmit errors>>
   string|$com1_tx_e

   field|@TR<<Drop packets>>
   string|$com1_tx_d

   field|@TR<<Transmit fifo>>
   string|$com1_tx_f

   field|@TR<<Collisions>>
   string|$com1_tx_co
       
   field|@TR<<Transmit carrier>>
   string|$com1_tx_ca

   field|@TR<<Transmit compress>>
   string|$com1_tx_c

   end_form

EOF
fi

}

[ -n "$FORM_interval" ] || FORM_interval=20

! empty "$FORM_refreshstop" && {
	FORM_interval=0
}

[ "$FORM_interval" -gt 0 ] >/dev/null 2>&1 && {
	meta_refresh="<meta http-equiv=\"refresh\" content=\"$FORM_interval;url=$SCRIPT_NAME?interval=$FORM_interval\" />"
}

header_inject_head=$(cat <<EOF
$meta_refresh

<script type="text/javascript">
<!--
function targetwindow(url) {
	var wasOpen  = false;
	var win = window.open(url);    
	return (typeof(win)=='object')?true:false;
}
-->
</script>

<style type="text/css">
<!--
#proctable table {
	width: 98%;
	margin-left: auto;
	margin-right: auto;
	text-align: left;
	font-size: 0.9em;
	border-style: none;
	border-spacing: 0;
}
#proctable td, th {
	padding-left: 0.2em;
	padding-right: 0.2em;
}
#proctable .number, .buttons {
	text-align: right;
}
#content .procwarn {
	position: relative;
	margin-left: 40%;
	text-align: right;
}
-->
</style>

EOF

)

header "Comport" "Status" "@TR<<status_comport_Running_Comports#Comport Status >>"

show_port com1

echo "<div class=\"settings\" style=\"float: right;\">"
echo "<form name=\"refresh\" method=\"post\" action=\"$SCRIPT_NAME\">"

[ "$FORM_interval" -gt 0 ] >/dev/null 2>&1 && {
	cat <<EOF
<input type="submit" value=" @TR<<status_comports_Stop_Refreshing#Stop Refreshing>> " name="refreshstop" />
@TR<<status_comports_Interval#Interval>>: $FORM_interval (@TR<<status_comports_in_seconds#in seconds>>)
EOF
} || {
	cat <<EOF
<input type="submit" value=" @TR<<status_comports_Auto_Refresh#Auto Refresh>> " name="refreshstart" />
@TR<<status_comports_Interval#Interval>>:

<select name="interval">
EOF
	for sec in $(seq 3 59); do
		[ "$sec" -eq 20 ] && {
			echo "<option value=\"$sec\" selected=\"selected\">$sec</option>"
		} || {
			echo "<option value=\"$sec\">$sec</option>"
		}
	done
	cat <<EOF
</select>
@TR<<status_comports_in_seconds#in seconds>>
EOF
}
echo "</form></div><br/>"

footer ?>
<!--
##WEBIF:name:Comport:100:Status
-->
