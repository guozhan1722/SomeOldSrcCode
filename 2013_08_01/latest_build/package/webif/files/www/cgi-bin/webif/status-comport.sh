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
uci_load comport2

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
   com1_connect_num=`cat /var/run/com1_packet | head -n 19 |tail -n 1`
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

comport="com2"


header "Comport" "Status" "@TR<<status_comport_Running_Comports#Comport Status >>"

for port in $comport; do

    if [ "$port" = "com1" ];then
       config_get FORM_COM12_Port_Status $port COM1_Port_Status
       config_get FORM_COM12_Data_Baud_Rate $port COM1_Data_Baud_Rate
       config_get FORM_COM12_IP_Protocol $port COM1_IP_Protocol
       com12_connect_status=`cat /var/run/com1_packet | head -n 20 |tail -n 1`
       com12_connect_num=`cat /var/run/com1_packet | head -n 19 |tail -n 1`
       com12_rx_b=`cat /var/run/com1_packet | head -n 1 |tail -n 1`
       com12_rx_p=`cat /var/run/com1_packet | head -n 2 |tail -n 1`
       com12_tx_b=`cat /var/run/com1_packet | head -n 9 |tail -n 1`
       com12_tx_p=`cat /var/run/com1_packet | head -n 10 |tail -n 1`

    elif [ "$port" = "com2" ];then
       config_get FORM_COM12_Port_Status $port COM2_Port_Status
       config_get FORM_COM12_Data_Baud_Rate $port COM2_Data_Baud_Rate
       config_get FORM_COM12_IP_Protocol $port COM2_IP_Protocol
       com12_connect_status=`cat /var/run/com2_packet | head -n 20 |tail -n 1`
       com12_connect_num=`cat /var/run/com2_packet | head -n 19 |tail -n 1`
       com12_rx_b=`cat /var/run/com2_packet | head -n 1 |tail -n 1`
       com12_rx_p=`cat /var/run/com2_packet | head -n 2 |tail -n 1`
       com12_tx_b=`cat /var/run/com2_packet | head -n 9 |tail -n 1`
       com12_tx_p=`cat /var/run/com2_packet | head -n 10 |tail -n 1`
    else
       config_get FORM_COM12_Port_Status $port COM1_Port_Status
       config_get FORM_COM12_Data_Baud_Rate $port COM1_Data_Baud_Rate
       config_get FORM_COM12_IP_Protocol $port COM1_IP_Protocol
       com12_connect_status=`cat /var/run/com1_packet | head -n 20 |tail -n 1`
       com12_rx_b=`cat /var/run/com1_packet | head -n 1 |tail -n 1`
       com12_rx_p=`cat /var/run/com1_packet | head -n 2 |tail -n 1`
       com12_tx_b=`cat /var/run/com1_packet | head -n 9 |tail -n 1`
       com12_tx_p=`cat /var/run/com1_packet | head -n 10 |tail -n 1`
    fi

    if [ "$com12_connect_status" = "B" ];then
      com12_connect_status="Active"      
   else  
      com12_connect_status="Not Active"      
   fi

   if [ "$FORM_COM12_Port_Status" = "B" ];then
      FORM_COM12_Port_Status="Enable"      
   else  
      FORM_COM12_Port_Status="Disable"      
   fi


   case "$FORM_COM12_Data_Baud_Rate" in
    "A")
        FORM_COM12_Data_Baud_Rate="300"
        ;;
    "B")
        FORM_COM12_Data_Baud_Rate="600"
        ;;
    "C")
        FORM_COM12_Data_Baud_Rate="1200"
        ;;
    "D")
        FORM_COM12_Data_Baud_Rate="2400"
        ;;
    "E")
        FORM_COM12_Data_Baud_Rate="3600"
        ;;
    "F")
        FORM_COM12_Data_Baud_Rate="4800"
        ;;
    "G")
        FORM_COM12_Data_Baud_Rate="7200"
        ;;
    "H")
        FORM_COM12_Data_Baud_Rate="9600"
        ;;
    "I")
        FORM_COM12_Data_Baud_Rate="14400"
        ;;
    "J")
        FORM_COM12_Data_Baud_Rate="19200"
        ;;

    "K")
        FORM_COM12_Data_Baud_Rate="28800"
        ;;
    "L")
        FORM_COM12_Data_Baud_Rate="38400"
        ;;
    "M")
        FORM_COM12_Data_Baud_Rate="57600"
        ;;
    "N")
        FORM_COM12_Data_Baud_Rate="115200"
        ;;
    "O")
        FORM_COM12_Data_Baud_Rate="230400"
        ;;
    "P")
        FORM_COM12_Data_Baud_Rate="460800"
        ;;
    "Q")
        FORM_COM12_Data_Baud_Rate="921600"
        ;;
    esac    



   case "$FORM_COM12_IP_Protocol" in
   "A")
      FORM_COM12_IP_Protocol="TCP Client"
      ;;      
   "B")
      FORM_COM12_IP_Protocol="TCP Server"
      ;;      
   "C")
      FORM_COM12_IP_Protocol="TCP Client/Server"
      ;;      
   "D")
      FORM_COM12_IP_Protocol="UDP Point to Point"
      ;;      
   "E")
      FORM_COM12_IP_Protocol="UDP Point to Multipoint(P)"
      ;;      
   "F")
      FORM_COM12_IP_Protocol="UDP Point to Multipoint(MP)"
      ;;      
   "G")
      FORM_COM12_IP_Protocol="UDP Multipoint to Multipoint"
      ;;      
   "H")
      FORM_COM12_IP_Protocol="SMTP Client"
      ;;      
   "I")
      FORM_COM12_IP_Protocol="PPP"
      ;;
   "J")
      FORM_COM12_IP_Protocol="SMS Transparent Mode"
      ;;
   "K")
      FORM_COM12_IP_Protocol="SMS AT Mode"
      ;;      
   esac

cat <<EOF
<div class="settings">
<table style="width: 95%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3><strong>Port Status</strong></h3></th>
</table>

EOF
if [ "$FORM_COM12_Port_Status" = "Disable" ]; then
cat <<EOF
    Port is disabled
EOF
else
cat <<EOF
<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
        <th><h3>General Status</h3></th>
</tr>
<tr>
	<td width="10%">Port Status</td>
	<td width="10%">Baud Rate</td>
	<td width="10%">Connect As</td>
	<td width="10%">Connect Status</td>
</tr>
EOF
if [ "$com12_connect_status" = "Active" ];then
cat <<EOF
<tr class="odd">
	<td width="10%">$FORM_COM12_Port_Status</td>
	<td width="10%">$FORM_COM12_Data_Baud_Rate</td>
	<td width="10%">$FORM_COM12_IP_Protocol</td>
	<td width="10%">$com12_connect_status (${com12_connect_num})</td>
</tr>
EOF
else
cat <<EOF
<tr class="odd">
	<td width="10%">$FORM_COM12_Port_Status</td>
	<td width="10%">$FORM_COM12_Data_Baud_Rate</td>
	<td width="10%">$FORM_COM12_IP_Protocol</td>
	<td width="10%">$com12_connect_status</td>
</tr>
EOF
fi

cat <<EOF
</table>

<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">

<tr>
        <th><h3><strong>Traffic Status</strong></h3></th>
</tr>

<tr>
	<td width="10%">Receive bytes</td>
	<td width="10%">Receive packets</td>
	<td width="10%">Transmit bytes</td>
	<td width="10%">Transmit packets</td>

</tr>

<tr class="odd">
	<td width="10%">$com12_rx_b</th>
	<td width="10%">$com12_rx_p</th>
	<td width="10%">$com12_tx_b</th>
	<td width="10%">$com12_tx_p</th>
</tr>
</table>
EOF
fi
cat <<EOF
</div>
EOF
done

#show_port com1

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

footer_nosubmit ?>
<!--
##WEBIF:name:Comport:100:Status
-->
