#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"
###################################################################
# COM2 configuration page
#
# Description:
#	Configures COM2 port settings.
#
#
# UCI variables referenced:
#   todo
# Configuration files referenced:
#   /etc/config/comport2
#

uci_load comport2
#ports=$(echo "$port" |uniq)
port="com2"


if empty "$FORM_submit"; then
       config_get FORM_COM2_Port_Status             $port COM2_Port_Status
       config_get FORM_COM2_Chanel_Mode             $port COM2_Chanel_Mode
       config_get FORM_COM2_Data_Baud_Rate          $port COM2_Data_Baud_Rate
       config_get FORM_COM2_Data_Format             $port COM2_Data_Format
       config_get FORM_COM2_Flow_Control            $port COM2_Flow_Control
       config_get FORM_COM2_Pre_Data_Delay          $port COM2_Pre_Data_Delay
       config_get FORM_COM2_Post_Data_Delay         $port COM2_Post_Data_Delay
       config_get FORM_COM2_Data_Mode               $port COM2_Data_Mode
       config_get FORM_COM2_Character_Timeout       $port COM2_Character_Timeout
       config_get FORM_COM2_Max_Packet_Len          $port COM2_Max_Packet_Len
       config_get FORM_COM2_QoS                     $port COM2_QoS
       config_get FORM_COM2_NoConnect_Data_Intake   $port COM2_NoConnect_Data_Intake
       config_get FORM_COM2_Modbus_Mode             $port COM2_Modbus_Mode
       config_get FORM_COM2_IP_Protocol             $port COM2_IP_Protocol

       config_get FORM_COM2_T_Client_Server_Addr    $port COM2_T_Client_Server_Addr
       config_get FORM_COM2_T_Client_Server_Port    $port COM2_T_Client_Server_Port
       config_get FORM_COM2_T_Client_Timeout        $port COM2_T_Client_Timeout

       config_get FORM_COM2_T_Server_Listen_Port $port COM2_T_Server_Listen_Port
       config_get FORM_COM2_T_Server_Timeout $port COM2_T_Server_Timeout

       config_get FORM_COM2_U_PtoP_Remote_Addr $port COM2_U_PtoP_Remote_Addr
       config_get FORM_COM2_U_PtoP_Remote_Port $port COM2_U_PtoP_Remote_Port
       config_get FORM_COM2_U_PtoP_Listen_Port $port COM2_U_PtoP_Listen_Port

       config_get FORM_COM2_UM_P_Multicast_Addr $port COM2_UM_P_Multicast_Addr
       config_get FORM_COM2_UM_P_Multicast_Port $port COM2_UM_P_Multicast_Port
       config_get FORM_COM2_UM_P_Listen_Port $port COM2_UM_P_Listen_Port
       config_get FORM_COM2_UM_P_TTL $port COM2_UM_P_TTL

       config_get FORM_COM2_UM_M_Remote_Addr $port COM2_UM_M_Remote_Addr
       config_get FORM_COM2_UM_M_Remote_Port $port COM2_UM_M_Remote_Port
       config_get FORM_COM2_UM_M_Multicast_Addr $port COM2_UM_M_Multicast_Addr
       config_get FORM_COM2_UM_M_Multicast_Port $port COM2_UM_M_Multicast_Port

       config_get FORM_COM2_UMtoM_Multicast_Addr $port COM2_UMtoM_Multicast_Addr
       config_get FORM_COM2_UMtoM_Multicast_Port $port COM2_UMtoM_Multicast_Port
       config_get FORM_COM2_UMtoM_Multicast_TTL $port COM2_UMtoM_Multicast_TTL

       config_get FORM_COM2_UMtoM_Listen_Multicast_Addr $port COM2_UMtoM_Listen_Multicast_Addr
       config_get FORM_COM2_UMtoM_Listen_Multicast_Port $port COM2_UMtoM_Listen_Multicast_Port

       config_get FORM_COM2_SMTP_Mail_Subject $port COM2_SMTP_Mail_Subject
       config_get FORM_COM2_SMTP_Server $port COM2_SMTP_Server
       config_get FORM_COM2_SMTP_Username $port COM2_SMTP_Username
       config_get FORM_COM2_SMTP_Password $port COM2_SMTP_Password
       config_get FORM_COM2_SMTP_Recipient $port COM2_SMTP_Recipient
       config_get FORM_COM2_SMTP_Buffer $port COM2_SMTP_Buffer
       config_get FORM_COM2_SMTP_Timeout $port COM2_SMTP_Timeout
       config_get FORM_COM2_SMTP_Transfer_Mode $port COM2_SMTP_Transfer_Mode

       config_get FORM_COM2_PPP_Local_IP $port COM2_PPP_Local_IP
       config_get FORM_COM2_PPP_Host_IP $port COM2_PPP_Host_IP
       config_get FORM_COM2_PPP_Idle_Timeout $port COM2_PPP_Idle_Timeout

       config_get FORM_COM2_SMS_Message_Max_Size $port COM2_SMS_Message_Max_Size
       config_get FORM_COM2_SMS_Reply_Timeout $port COM2_SMS_Reply_Timeout
       config_get FORM_COM2_SMS_Access_Control $port COM2_SMS_Access_Control
       config_get FORM_COM2_SMS_Read_SMS_Control $port COM2_SMS_Read_SMS_Control
       config_get FORM_COM2_SMS_Phone_Number_1 $port COM2_SMS_Phone_Number_1
       config_get FORM_COM2_SMS_Phone_Number_2 $port COM2_SMS_Phone_Number_2
       config_get FORM_COM2_SMS_Phone_Number_3 $port COM2_SMS_Phone_Number_3
       config_get FORM_COM2_SMS_Phone_Number_4 $port COM2_SMS_Phone_Number_4
       config_get FORM_COM2_SMS_Phone_Number_5 $port COM2_SMS_Phone_Number_5
else
       eval FORM_COM2_Port_Status="\$FORM_${port}_COM2_Port_Status"
       eval FORM_COM2_Chanel_Mode="\$FORM_${port}_COM2_Chanel_Mode"
       eval FORM_COM2_Data_Baud_Rate="\$FORM_${port}_COM2_Data_Baud_Rate"
       eval FORM_COM2_Data_Format="\$FORM_${port}_COM2_Data_Format"
       eval FORM_COM2_Flow_Control="\$FORM_${port}_COM2_Flow_Control"

       eval FORM_COM2_Pre_Data_Delay="\$FORM_${port}_COM2_Pre_Data_Delay"
       eval FORM_COM2_Post_Data_Delay="\$FORM_${port}_COM2_Post_Data_Delay"
       eval FORM_COM2_Data_Mode="\$FORM_${port}_COM2_Data_Mode"
       eval FORM_COM2_Character_Timeout="\$FORM_${port}_COM2_Character_Timeout"
       eval FORM_COM2_Max_Packet_Len="\$FORM_${port}_COM2_Max_Packet_Len"
       eval FORM_COM2_QoS="\$FORM_${port}_COM2_QoS"

       eval FORM_COM2_NoConnect_Data_Intake="\$FORM_${port}_COM2_NoConnect_Data_Intake"
       eval FORM_COM2_Modbus_Mode="\$FORM_${port}_COM2_Modbus_Mode"
       eval FORM_COM2_IP_Protocol="\$FORM_${port}_COM2_IP_Protocol"

       eval FORM_COM2_T_Client_Server_Addr="\$FORM_${port}_COM2_T_Client_Server_Addr"
       eval FORM_COM2_T_Client_Server_Port="\$FORM_${port}_COM2_T_Client_Server_Port"
       eval FORM_COM2_T_Client_Timeout="\$FORM_${port}_COM2_T_Client_Timeout"
       eval FORM_COM2_T_Server_Listen_Port="\$FORM_${port}_COM2_T_Server_Listen_Port"
       eval FORM_COM2_T_Server_Timeout="\$FORM_${port}_COM2_T_Server_Timeout"

       eval FORM_COM2_U_PtoP_Remote_Addr="\$FORM_${port}_COM2_U_PtoP_Remote_Addr"
       eval FORM_COM2_U_PtoP_Remote_Port="\$FORM_${port}_COM2_U_PtoP_Remote_Port"
       eval FORM_COM2_U_PtoP_Listen_Port="\$FORM_${port}_COM2_U_PtoP_Listen_Port"

       eval FORM_COM2_UM_P_Multicast_Addr="\$FORM_${port}_COM2_UM_P_Multicast_Addr"
       eval FORM_COM2_UM_P_Multicast_Port="\$FORM_${port}_COM2_UM_P_Multicast_Port"
       eval FORM_COM2_UM_P_Listen_Port="\$FORM_${port}_COM2_UM_P_Listen_Port"
       eval FORM_COM2_UM_P_TTL="\$FORM_${port}_COM2_UM_P_TTL"

       eval FORM_COM2_UM_M_Remote_Addr="\$FORM_${port}_COM2_UM_M_Remote_Addr"
       eval FORM_COM2_UM_M_Remote_Port="\$FORM_${port}_COM2_UM_M_Remote_Port"
       eval FORM_COM2_UM_M_Multicast_Addr="\$FORM_${port}_COM2_UM_M_Multicast_Addr"
       eval FORM_COM2_UM_M_Multicast_Port="\$FORM_${port}_COM2_UM_M_Multicast_Port"

       eval FORM_COM2_UMtoM_Multicast_Addr="\$FORM_${port}_COM2_UMtoM_Multicast_Addr"
       eval FORM_COM2_UMtoM_Multicast_Port="\$FORM_${port}_COM2_UMtoM_Multicast_Port"
       eval FORM_COM2_UMtoM_Multicast_TTL="\$FORM_${port}_COM2_UMtoM_Multicast_TTL"

       eval FORM_COM2_UMtoM_Listen_Multicast_Addr="\$FORM_${port}_COM2_UMtoM_Listen_Multicast_Addr"
       eval FORM_COM2_UMtoM_Listen_Multicast_Port="\$FORM_${port}_COM2_UMtoM_Listen_Multicast_Port"

       eval FORM_COM2_SMTP_Mail_Subject="\$FORM_${port}_COM2_SMTP_Mail_Subject"
       eval FORM_COM2_SMTP_Server="\$FORM_${port}_COM2_SMTP_Server"
       eval FORM_COM2_SMTP_Username="\$FORM_${port}_COM2_SMTP_Username"
       eval FORM_COM2_SMTP_Password="\$FORM_${port}_COM2_SMTP_Password"
       eval FORM_COM2_SMTP_Recipient="\$FORM_${port}_COM2_SMTP_Recipient"
       eval FORM_COM2_SMTP_Buffer="\$FORM_${port}_COM2_SMTP_Buffer"
       eval FORM_COM2_SMTP_Timeout="\$FORM_${port}_COM2_SMTP_Timeout"
       eval FORM_COM2_SMTP_Transfer_Mode="\$FORM_${port}_COM2_SMTP_Transfer_Mode"

       eval FORM_COM2_PPP_Local_IP="\$FORM_${port}_COM2_PPP_Local_IP"
       eval FORM_COM2_PPP_Host_IP="\$FORM_${port}_COM2_PPP_Host_IP"
       eval FORM_COM2_PPP_Idle_Timeout="\$FORM_${port}_COM2_PPP_Idle_Timeout"



       eval FORM_COM2_SMS_Message_Max_Size="\$FORM_${port}_COM2_SMS_Message_Max_Size"
       eval FORM_COM2_SMS_Reply_Timeout="\$FORM_${port}_COM2_SMS_Reply_Timeout"
       eval FORM_COM2_SMS_Access_Control="\$FORM_${port}_COM2_SMS_Access_Control"
       eval FORM_COM2_SMS_Read_SMS_Control="\$FORM_${port}_COM2_SMS_Read_SMS_Control"
       eval FORM_COM2_SMS_Phone_Number_1="\$FORM_${port}_COM2_SMS_Phone_Number_1"
       eval FORM_COM2_SMS_Phone_Number_2="\$FORM_${port}_COM2_SMS_Phone_Number_2"
       eval FORM_COM2_SMS_Phone_Number_3="\$FORM_${port}_COM2_SMS_Phone_Number_3"
       eval FORM_COM2_SMS_Phone_Number_4="\$FORM_${port}_COM2_SMS_Phone_Number_4"
       eval FORM_COM2_SMS_Phone_Number_5="\$FORM_${port}_COM2_SMS_Phone_Number_5"



       validate <<EOF
int|FORM_${port}_COM2_Pre_Data_Delay|$port @TR<<Pre-Data Delay>>||$FORM_COM2_Pre_Data_Delay
int|FORM_${port}_COM2_Post_Data_Delay|$port @TR<<Post-Data Delay>>||$FORM_COM2_Post_Data_Delay
int|FORM_${port}_COM2_Character_Timeout|$port @TR<<Character Timeout>>||$FORM_COM2_Character_Timeout
int|FORM_${port}_COM2_Max_Packet_Len|$port @TR<<Maximum Packet Size>>||$FORM_COM2_Max_Packet_Len
int|FORM_${port}_COM2_Modbus_Protect_Key|$port @TR<<TCP MODBUS Protect Key>>||$FORM_COM2_Modbus_Protect_Key
 
ip|FORM_${port}_COM2_T_Client_Server_Addr|$port @TR<<Remote Server IP Address>>||$FORM_COM2_T_Client_Server_Addr
int|FORM_${port}_COM2_T_Client_Server_Port|$port @TR<<Remote Server port>>||$FORM_COM2_T_Client_Server_Port
int|FORM_${port}_COM2_T_Client_Timeout|$port @TR<<Outgoing Connection Timeout>>||$FORM_COM2_T_Client_Timeout
int|FORM_${port}_COM2_T_Server_Listen_Port|$port @TR<<Local Listening port>>||$FORM_COM2_T_Server_Listen_Port
int|FORM_${port}_COM2_T_Server_Timeout|$port @TR<<Incoming Connection Timeout>>||$FORM_COM2_T_Server_Timeout
 
ip|FORM_${port}_COM2_U_PtoP_Remote_Addr|$port @TR<<Remote IP Address>>||$FORM_COM2_U_PtoP_Remote_Addr
int|FORM_${port}_COM2_U_PtoP_Remote_Port|$port @TR<<Remote port>>||$FORM_COM2_U_PtoP_Remote_Port
int|FORM_${port}_COM2_U_PtoP_Listen_Port|$port @TR<<Listening port>>||$FORM_COM2_U_PtoP_Listen_Port

ip|FORM_${port}_COM2_UM_P_Multicast_Addr|$port @TR<<Multicast IP Address>>||$FORM_COM2_UM_P_Multicast_Addr
int|FORM_${port}_COM2_UM_P_Multicast_Port|$port @TR<<Multicast port>>||$FORM_COM2_UM_P_Multicast_Port
int|FORM_${port}_COM2_UM_P_Listen_Port|$port @TR<<Listening port>>||$FORM_COM2_UM_P_Listen_Port
int|FORM_${port}_COM2_UM_P_TTL|$port @TR<<Time To Live>>||$FORM_COM2_UM_P_TTL

ip|FORM_${port}_COM2_UM_M_Remote_Addr|$port @TR<<Remote IP Address>>||$FORM_COM2_UM_M_Remote_Addr
int|FORM_${port}_COM2_UM_M_Remote_Port|$port @TR<<Remot port>>||$FORM_COM2_UM_M_Remote_Port
ip|FORM_${port}_COM2_UM_M_Multicast_Addr|$port @TR<<Multicast IP Address>>||$FORM_COM2_UM_M_Multicast_Addr
int|FORM_${port}_COM2_UM_M_Multicast_Port|$port @TR<<Multicast Port>>||$FORM_COM2_UM_M_Multicast_Port

ip|FORM_${port}_COM2_UMtoM_Multicast_Addr|$port @TR<<Multicast IP Address>>||$FORM_COM2_UMtoM_Multicast_Addr
int|FORM_${port}_COM2_UMtoM_Multicast_Port|$port @TR<<Multicast port>>||$FORM_COM2_UMtoM_Multicast_Port
int|FORM_${port}_COM2_UMtoM_Multicast_TTL|$port @TR<<Time To Live>>||$FORM_COM2_UMtoM_Multicast_TTL
ip|FORM_${port}_COM2_UMtoM_Listen_Multicast_Addr|$port @TR<<Listen Multicast IP Address>>||$FORM_COM2_UMtoM_Listen_Multicast_Addr
int|FORM_${port}_COM2_UMtoM_Listen_Multicast_Port|$port @TR<<Listen Multicast Port>>||$FORM_COM2_UMtoM_Listen_Multicast_Port

int|FORM_${port}_COM2_SMTP_Buffer|$port @TR<<Message Max Size>>||$FORM_COM2_SMTP_Buffer
int|FORM_${port}_COM2_SMTP_Timeout|$port @TR<<TimeOut>>||$FORM_COM2_SMTP_Timeout
EOF

       [ "$?" = "0" ] && {

         uci_set "comport2" "$port" "COM2_Port_Status" "$FORM_COM2_Port_Status"
         uci_set "comport2" "$port" "COM2_Chanel_Mode" "$FORM_COM2_Chanel_Mode"
         uci_set "comport2" "$port" "COM2_Data_Baud_Rate" "$FORM_COM2_Data_Baud_Rate"
         uci_set "comport2" "$port" "COM2_Data_Format" "$FORM_COM2_Data_Format"
         uci_set "comport2" "$port" "COM2_Flow_Control" "$FORM_COM2_Flow_Control"
         uci_set "comport2" "$port" "COM2_Pre_Data_Delay" "$FORM_COM2_Pre_Data_Delay"
         uci_set "comport2" "$port" "COM2_Post_Data_Delay" "$FORM_COM2_Post_Data_Delay"
         uci_set "comport2" "$port" "COM2_Data_Mode" "$FORM_COM2_Data_Mode"
         uci_set "comport2" "$port" "COM2_Character_Timeout" "$FORM_COM2_Character_Timeout"
         uci_set "comport2" "$port" "COM2_Max_Packet_Len" "$FORM_COM2_Max_Packet_Len"
         uci_set "comport2" "$port" "COM2_NoConnect_Data_Intake" "$FORM_COM2_NoConnect_Data_Intake"
         uci_set "comport2" "$port" "COM2_QoS" "$FORM_COM2_QoS"
         uci_set "comport2" "$port" "COM2_Modbus_Mode" "$FORM_COM2_Modbus_Mode"
         uci_set "comport2" "$port" "COM2_IP_Protocol" "$FORM_COM2_IP_Protocol"

         case "$FORM_COM2_IP_Protocol" in
            "A")
               uci_set "comport2" "$port" "COM2_T_Client_Server_Addr" "$FORM_COM2_T_Client_Server_Addr"
               uci_set "comport2" "$port" "COM2_T_Client_Server_Port" "$FORM_COM2_T_Client_Server_Port"
               uci_set "comport2" "$port" "COM2_T_Client_Timeout" "$FORM_COM2_T_Client_Timeout"
               ;;
            "B")
               uci_set "comport2" "$port" "COM2_T_Server_Listen_Port" "$FORM_COM2_T_Server_Listen_Port"
               uci_set "comport2" "$port" "COM2_T_Server_Timeout" "$FORM_COM2_T_Server_Timeout"
               ;;
            "C")
               uci_set "comport2" "$port" "COM2_T_Client_Server_Addr" "$FORM_COM2_T_Client_Server_Addr"
               uci_set "comport2" "$port" "COM2_T_Client_Server_Port" "$FORM_COM2_T_Client_Server_Port"
               uci_set "comport2" "$port" "COM2_T_Client_Timeout" "$FORM_COM2_T_Client_Timeout"
               uci_set "comport2" "$port" "COM2_T_Server_Listen_Port" "$FORM_COM2_T_Server_Listen_Port"
               ;;
            "D")
               uci_set "comport2" "$port" "COM2_U_PtoP_Remote_Addr" "$FORM_COM2_U_PtoP_Remote_Addr"
               uci_set "comport2" "$port" "COM2_U_PtoP_Remote_Port" "$FORM_COM2_U_PtoP_Remote_Port"
               uci_set "comport2" "$port" "COM2_U_PtoP_Listen_Port" "$FORM_COM2_U_PtoP_Listen_Port"
               ;;
            "E")
               uci_set "comport2" "$port" "COM2_UM_P_Multicast_Addr" "$FORM_COM2_UM_P_Multicast_Addr"
               uci_set "comport2" "$port" "COM2_UM_P_Multicast_Port" "$FORM_COM2_UM_P_Multicast_Port"
               uci_set "comport2" "$port" "COM2_UM_P_Listen_Port" "$FORM_COM2_UM_P_Listen_Port"
               uci_set "comport2" "$port" "COM2_UM_P_TTL" "$FORM_COM2_UM_P_TTL"
               ;;
            "F")
               uci_set "comport2" "$port" "COM2_UM_M_Remote_Addr" "$FORM_COM2_UM_M_Remote_Addr"
               uci_set "comport2" "$port" "COM2_UM_M_Remote_Port" "$FORM_COM2_UM_M_Remote_Port"
               uci_set "comport2" "$port" "COM2_UM_M_Multicast_Addr" "$FORM_COM2_UM_M_Multicast_Addr"
               uci_set "comport2" "$port" "COM2_UM_M_Multicast_Port" "$FORM_COM2_UM_M_Multicast_Port"
               ;;
            "G")
               uci_set "comport2" "$port" "COM2_UMtoM_Multicast_Addr" "$FORM_COM2_UMtoM_Multicast_Addr"
               uci_set "comport2" "$port" "COM2_UMtoM_Multicast_Port" "$FORM_COM2_UMtoM_Multicast_Port"
               uci_set "comport2" "$port" "COM2_UMtoM_Multicast_TTL" "$FORM_COM2_UMtoM_Multicast_TTL"
               uci_set "comport2" "$port" "COM2_UMtoM_Listen_Multicast_Addr" "$FORM_COM2_UMtoM_Listen_Multicast_Addr"
               uci_set "comport2" "$port" "COM2_UMtoM_Listen_Multicast_Port" "$FORM_COM2_UMtoM_Listen_Multicast_Port"
               ;;
            "H")
               uci_set "comport2" "$port" "COM2_SMTP_Mail_Subject" "$FORM_COM2_SMTP_Mail_Subject"
               uci_set "comport2" "$port" "COM2_SMTP_Server" "$FORM_COM2_SMTP_Server"
               uci_set "comport2" "$port" "COM2_SMTP_Username" "$FORM_COM2_SMTP_Username"
               uci_set "comport2" "$port" "COM2_SMTP_Password" "$FORM_COM2_SMTP_Password"
               uci_set "comport2" "$port" "COM2_SMTP_Recipient" "$FORM_COM2_SMTP_Recipient"
               uci_set "comport2" "$port" "COM2_SMTP_Buffer" "$FORM_COM2_SMTP_Buffer"
               uci_set "comport2" "$port" "COM2_SMTP_Timeout" "$FORM_COM2_SMTP_Timeout"
               uci_set "comport2" "$port" "COM2_SMTP_Transfer_Mode" "$FORM_COM2_SMTP_Transfer_Mode"
               ;;
            "I")
               uci_set "comport2" "$port" "COM2_PPP_Local_IP" "$FORM_COM2_PPP_Local_IP"
               uci_set "comport2" "$port" "COM2_PPP_Host_IP" "$FORM_COM2_PPP_Host_IP"
               uci_set "comport2" "$port" "COM2_PPP_Idle_Timeout" "$FORM_COM2_PPP_Idle_Timeout"
               ;;
            "J")
               uci_set "comport2" "$port" "COM2_SMS_Message_Max_Size" "$FORM_COM2_SMS_Message_Max_Size"
               uci_set "comport2" "$port" "COM2_SMS_Reply_Timeout" "$FORM_COM2_SMS_Reply_Timeout"
               uci_set "comport2" "$port" "COM2_SMS_Access_Control" "$FORM_COM2_SMS_Access_Control"
               uci_set "comport2" "$port" "COM2_SMS_Read_SMS_Control" "$FORM_COM2_SMS_Read_SMS_Control"
               uci_set "comport2" "$port" "COM2_SMS_Phone_Number_1" "$FORM_COM2_SMS_Phone_Number_1"
               uci_set "comport2" "$port" "COM2_SMS_Phone_Number_2" "$FORM_COM2_SMS_Phone_Number_2"
               uci_set "comport2" "$port" "COM2_SMS_Phone_Number_3" "$FORM_COM2_SMS_Phone_Number_3"
               uci_set "comport2" "$port" "COM2_SMS_Phone_Number_4" "$FORM_COM2_SMS_Phone_Number_4"
               uci_set "comport2" "$port" "COM2_SMS_Phone_Number_5" "$FORM_COM2_SMS_Phone_Number_5"
               ;;
         esac
     }

fi
   
   
uci_load "gpsd"
       config_get GPS_status "gpsd_conf" status
       config_get GPS_GPSD_TCP_Port "gpsd_conf" GPSD_TCP_Port
       config_get GPS_GPS_Source "gpsd_conf" GPS_Source 
       config_get GPS_GPS_Source_STD "gpsd_conf" Standalone_GPS_Source 
        if [ "$GPS_GPS_Source" ]; then
    	   GPS_STD="NONEED"
        else
	   GPS_GPS_Source="$GPS_GPS_Source_STD"
        fi
        if [ "$GPS_status" = "1" ]; then
            GPS_status_info="Connected.  Port:${GPS_GPSD_TCP_Port}"
        else
            GPS_status_info="Disabled. <a href=gps-settings.sh>Enable gps here firstly.</a>"
        fi
   
   
comport_options="start_form|Comport @TR<<Configuration>>

field|@TR<<Com Port status>>
select|${port}_COM2_Port_Status|$FORM_COM2_Port_Status
option|B|@TR<<Enable>>
option|A|@TR<<Disable>>

field|@TR<<Channel Mode>>|${port}_COM2_Chanel_Mode_field|hidden
select|${port}_COM2_Chanel_Mode|$FORM_COM2_Chanel_Mode
option|A|@TR<<RS232>>
option|B|@TR<<RS485>>
option|C|@TR<<RS422>>
#helpitem|Channel Mode
#helptext|Helptext Channel Mode#Determines which serial interface shall be used to connect to external devices.

field|@TR<<Data Baud Rate>>|${port}_COM2_Data_Baud_Rate_field|hidden
select|${port}_COM2_Data_Baud_Rate|$FORM_COM2_Data_Baud_Rate
option|A|@TR<<300>>
option|B|@TR<<600>>
option|C|@TR<<1200>>
option|D|@TR<<2400>>
option|E|@TR<<3600>>
option|F|@TR<<4800>>
option|G|@TR<<7200>>
option|H|@TR<<9600>>
option|I|@TR<<14400>>
option|J|@TR<<19200>>
option|K|@TR<<28800>>
option|L|@TR<<38400>>
option|M|@TR<<57600>>
option|N|@TR<<115200>>
option|O|@TR<<230400>>
option|P|@TR<<460800>>
option|Q|@TR<<921600>>

field|@TR<<Data Format>>|${port}_COM2_Data_Format_field|hidden
select|${port}_COM2_Data_Format|$FORM_COM2_Data_Format
option|A|@TR<<8N1>>
option|B|@TR<<8N2>>
option|C|@TR<<8E1>>
option|D|@TR<<8O1>>
option|E|@TR<<7N1>>
option|F|@TR<<7N2>>
option|G|@TR<<7E1>>
option|H|@TR<<7O1>>
option|I|@TR<<7E2>>
option|J|@TR<<7O2>>

field|@TR<<Flow Control>>|${port}_COM2_Flow_Control_field|hidden
select|${port}_COM2_Flow_Control|$FORM_COM2_Flow_Control
option|A|@TR<<none>>
option|C|@TR<<CTS Framing>>

field|@TR<<Pre-Data Delay (ms)>>|${port}_COM2_Pre_Data_Delay_field|hidden
text|${port}_COM2_Pre_Data_Delay|$FORM_COM2_Pre_Data_Delay

field|@TR<<Post-Data Delay (ms)>>|${port}_COM2_Post_Data_Delay_field|hidden
text|${port}_COM2_Post_Data_Delay|$FORM_COM2_Post_Data_Delay

field|@TR<<Data Mode>>|${port}_COM2_Data_Mode_field|hidden
radio|${port}_COM2_Data_Mode|$FORM_COM2_Data_Mode|A|@TR<<Seamless>>
radio|${port}_COM2_Data_Mode|$FORM_COM2_Data_Mode|B|@TR<<Transparent>>

field|@TR<<Character Timeout>>|${port}_COM2_Character_Timeout_field|hidden
text|${port}_COM2_Character_Timeout|$FORM_COM2_Character_Timeout

field|@TR<<Maximum Packet Size>>|${port}_COM2_Max_Packet_Len_field|hidden
text|${port}_COM2_Max_Packet_Len|$FORM_COM2_Max_Packet_Len

field|@TR<<Priority>>|${port}_COM2_QoS_field|hidden
radio|${port}_COM2_QoS|$FORM_COM2_QoS|A|@TR<<Normal>>
radio|${port}_COM2_QoS|$FORM_COM2_QoS|B|@TR<<Medium>>
radio|${port}_COM2_QoS|$FORM_COM2_QoS|C|@TR<<High>>

field|@TR<<No-Connection Data>>|${port}_COM2_NoConnect_Data_Intake_field|hidden
radio|${port}_COM2_NoConnect_Data_Intake|$FORM_COM2_NoConnect_Data_Intake|A|@TR<<Disable>>
radio|${port}_COM2_NoConnect_Data_Intake|$FORM_COM2_NoConnect_Data_Intake|B|@TR<<Enable>>

field|@TR<<TCP MODBUS Status>>|${port}_COM2_Modbus_Mode_field|hidden
radio|${port}_COM2_Modbus_Mode|$FORM_COM2_Modbus_Mode|A|@TR<<Disable>>
radio|${port}_COM2_Modbus_Mode|$FORM_COM2_Modbus_Mode|B|@TR<<Enable>>

field|@TR<<IP Protocol Config>>|${port}_COM2_IP_Protocol_field|hidden
select|${port}_COM2_IP_Protocol|$FORM_COM2_IP_Protocol
option|A|@TR<<TCP Client>>
option|B|@TR<<TCP Server>>
option|C|@TR<<TCP Client/Server>>
option|D|@TR<<UDP Point to Point>>
option|E|@TR<<UDP Point to Multipoint(P)>>
option|F|@TR<<UDP Point to Multipoint(MP)>>
option|G|@TR<<UDP Multipoint to Multipoint>>
option|H|@TR<<SMTP Client>>
option|J|@TR<<SMS Transparent Mode>>
option|L|@TR<<GPS Transparent Mode>>
end_form

start_form|@TR<<TCP Configuration>>|${port}_T_Client_Server|hidden
field|@TR<<Remote Server IP Address>>|field_${port}_COM2_T_Client_Server_Addr|hidden
text|${port}_COM2_T_Client_Server_Addr|$FORM_COM2_T_Client_Server_Addr
field|@TR<<Remote Server port>>|field_${port}_COM2_T_Client_Server_Port|hidden
text|${port}_COM2_T_Client_Server_Port|$FORM_COM2_T_Client_Server_Port
field|@TR<<Outgoing Connection Timeout>>|field_${port}_COM2_T_Client_Timeout|hidden
text|${port}_COM2_T_Client_Timeout|$FORM_COM2_T_Client_Timeout
field|@TR<<Local Listening port>>|field_${port}_COM2_T_Server_Listen_Port|hidden
text|${port}_COM2_T_Server_Listen_Port|$FORM_COM2_T_Server_Listen_Port
field|@TR<<Incoming Connection Timeout>>|field_${port}_COM2_T_Server_Timeout|hidden
text|${port}_COM2_T_Server_Timeout|$FORM_COM2_T_Server_Timeout
end_form

start_form|@TR<<UDP Configurate>>|${port}_U_PtoP|hidden
field|@TR<<Remote IP Address>>|field_${port}_COM2_U_PtoP_Remote_Addr|hidden
text|${port}_COM2_U_PtoP_Remote_Addr|$FORM_COM2_U_PtoP_Remote_Addr
field|@TR<<Remote port>>|field_${port}_COM2_U_PtoP_Remote_Port|hidden
text|${port}_COM2_U_PtoP_Remote_Port|$FORM_COM2_U_PtoP_Remote_Port
field|@TR<<Listening port>>|field_${port}_COM2_U_PtoP_Listen_Port|hidden
text|${port}_COM2_U_PtoP_Listen_Port|$FORM_COM2_U_PtoP_Listen_Port
end_form

start_form|@TR<<UDP Configurate>>|${port}_UM_P_Multicast|hidden
field|@TR<<Multicast IP Address>>|field_${port}_COM2_UM_P_Multicast_Addr|hidden
text|${port}_COM2_UM_P_Multicast_Addr|$FORM_COM2_UM_P_Multicast_Addr
field|@TR<<Multicast port>>|field_${port}_COM2_UM_P_Multicast_Port|hidden
text|${port}_COM2_UM_P_Multicast_Port|$FORM_COM2_UM_P_Multicast_Port
field|@TR<<Listening port>>|field_${port}_COM2_UM_P_Listen_Port|hidden
text|${port}_COM2_UM_P_Listen_Port|$FORM_COM2_UM_P_Listen_Port
field|@TR<<Time To Live>>|field_${port}_COM2_UM_P_TTL|hidden
text|${port}_COM2_UM_P_TTL|$FORM_COM2_UM_P_TTL
end_form

start_form|@TR<<UDP Configurate>>|${port}_UM_M_Remote|hidden
field|@TR<<Remote IP Address>>|field_${port}_COM2_UM_M_Remote_Addr|hidden
text|${port}_COM2_UM_M_Remote_Addr|$FORM_COM2_UM_M_Remote_Addr
field|@TR<<Remot port>>|field_${port}_COM2_UM_M_Remote_Port|hidden
text|${port}_COM2_UM_M_Remote_Port|$FORM_COM2_UM_M_Remote_Port
field|@TR<<Multicast IP Address>>|field_${port}_COM2_UM_M_Multicast_Addr|hidden
text|${port}_COM2_UM_M_Multicast_Addr|$FORM_COM2_UM_M_Multicast_Addr
field|@TR<<Multicast Port>>|field_${port}_COM2_UM_M_Multicast_Port|hidden
text|${port}_COM2_UM_M_Multicast_Port|$FORM_COM2_UM_M_Multicast_Port
end_form

start_form|@TR<<UDP Configurate>>|${port}_UMtoM_Multicast|hidden
field|@TR<<Multicast IP Address>>|field_${port}_COM2_UMtoM_Multicast_Addr|hidden
text|${port}_COM2_UMtoM_Multicast_Addr|$FORM_COM2_UMtoM_Multicast_Addr
field|@TR<<Multicast port>>|field_${port}_COM2_UMtoM_Multicast_Port|hidden
text|${port}_COM2_UMtoM_Multicast_Port|$FORM_COM2_UMtoM_Multicast_Port
field|@TR<<Time To Live>>|field_${port}_COM2_UMtoM_Multicast_TTL|hidden
text|${port}_COM2_UMtoM_Multicast_TTL|$FORM_COM2_UMtoM_Multicast_TTL
field|@TR<<Listen Multicast IP Address>>|field_${port}_COM2_UMtoM_Listen_Multicast_Addr|hidden
text|${port}_COM2_UMtoM_Listen_Multicast_Addr|$FORM_COM2_UMtoM_Listen_Multicast_Addr
field|@TR<<Listen Multicast Port>>|field_${port}_COM2_UMtoM_Listen_Multicast_Port|hidden
text|${port}_COM2_UMtoM_Listen_Multicast_Port|$FORM_COM2_UMtoM_Listen_Multicast_Port
end_form


start_form|@TR<<SMTP Configuration>>|${port}_SMTP|hidden
field|@TR<<Mail Subject>>|field_${port}_COM2_SMTP_Mail_Subject|hidden
text|${port}_COM2_SMTP_Mail_Subject|$FORM_COM2_SMTP_Mail_Subject
field|@TR<<Mail Server (IP/Name)>>|field_${port}_COM2_SMTP_Server|hidden
text|${port}_COM2_SMTP_Server|$FORM_COM2_SMTP_Server
field|@TR<<Username>>|field_${port}_COM2_SMTP_Username|hidden
text|${port}_COM2_SMTP_Username|$FORM_COM2_SMTP_Username
field|@TR<<Password>>|field_${port}_COM2_SMTP_Password|hidden
text|${port}_COM2_SMTP_Password|$FORM_COM2_SMTP_Password
field|@TR<<Mail Recipient>>|field_${port}_COM2_SMTP_Recipient|hidden
text|${port}_COM2_SMTP_Recipient|$FORM_COM2_SMTP_Recipient
field|@TR<<Message Max Size>>|field_${port}_COM2_SMTP_Buffer|hidden
text|${port}_COM2_SMTP_Buffer|$FORM_COM2_SMTP_Buffer
field|@TR<<TimeOut(s)>>|field_${port}_COM2_SMTP_Timeout|hidden
text|${port}_COM2_SMTP_Timeout|$FORM_COM2_SMTP_Timeout
field|@TR<<Transfer Mode>>|field_${port}_COM2_SMTP_Transfer_Mode|hidden
select|${port}_COM2_SMTP_Transfer_Mode|$FORM_COM2_SMTP_Transfer_Mode
option|A|@TR<<Text>>
option|B|@TR<<Attached File>>
option|C|@TR<<Hex Code>>
end_form

start_form|@TR<<PPP Configurate>>|${port}_PPP|hidden
field|@TR<<PPP Local IP>>|field_${port}_COM2_PPP_Local_IP|hidden
text|${port}_COM2_PPP_Local_IP|$FORM_COM2_PPP_Local_IP
field|@TR<<PPP Host IP>>|field_${port}_COM2_PPP_Host_IP|hidden
text|${port}_COM2_PPP_Host_IP|$FORM_COM2_PPP_Host_IP
field|@TR<<PPP Idle Timeout(s)>>|field_${port}_COM2_PPP_Idle_Timeout|hidden
text|${port}_COM2_PPP_Idle_Timeout|$FORM_COM2_PPP_Idle_Timeout
end_form


start_form|@TR<<SMS Configuration>>|${port}_SMS|hidden

field|@TR<<Message Max Size>>|field_${port}_COM2_SMS_Message_Max_Size|hidden
text|${port}_COM2_SMS_Message_Max_Size|$FORM_COM2_SMS_Message_Max_Size
string|@TR<<[1...160]>>

field|@TR<<Reply Timeout(s)>>|field_${port}_COM2_SMS_Reply_Timeout|hidden
text|${port}_COM2_SMS_Reply_Timeout|$FORM_COM2_SMS_Reply_Timeout
string|@TR<<[1...65535] default: 10>>

field|@TR<<Access Control>>|field_${port}_COM2_SMS_Access_Control|hidden
select|${port}_COM2_SMS_Access_Control|$FORM_COM2_SMS_Access_Control
option|A|@TR<<Anonymous>>
option|B|@TR<<Control Phone List>>

field|@TR<<Read SMS Control>>|field_${port}_COM2_SMS_Read_SMS_Control|hidden
select|${port}_COM2_SMS_Read_SMS_Control|$FORM_COM2_SMS_Read_SMS_Control
option|A|@TR<<Keep in SIM Card>>
option|B|@TR<<Delete>>
end_form


start_form|@TR<<SMS Access Control Phone List>>|${port}_SMS_Access_Control_Phone_List|hidden
string|@TR<<Example: +1403xxxxxxx>>

field|@TR<<Phone Number 1>>|field_${port}_COM2_SMS_Phone_Number_1|hidden
text|${port}_COM2_SMS_Phone_Number_1|$FORM_COM2_SMS_Phone_Number_1

field|@TR<<Phone Number 2>>|field_${port}_COM2_SMS_Phone_Number_2|hidden
text|${port}_COM2_SMS_Phone_Number_2|$FORM_COM2_SMS_Phone_Number_2

field|@TR<<Phone Number 3>>|field_${port}_COM2_SMS_Phone_Number_3|hidden
text|${port}_COM2_SMS_Phone_Number_3|$FORM_COM2_SMS_Phone_Number_3

field|@TR<<Phone Number 4>>|field_${port}_COM2_SMS_Phone_Number_4|hidden
text|${port}_COM2_SMS_Phone_Number_4|$FORM_COM2_SMS_Phone_Number_4

field|@TR<<Phone Number 5>>|field_${port}_COM2_SMS_Phone_Number_5|hidden
text|${port}_COM2_SMS_Phone_Number_5|$FORM_COM2_SMS_Phone_Number_5

end_form

start_form|@TR<<GPS Transparent Mode>>|${port}_GPS|hidden
field|@TR<<GPS Connecting Status:>>|field_${port}_COM2_GPS_status|hidden
string| @TR<< $GPS_status_info >>
end_form

"

append forms "$comport_options" "$N"






###################################################################
# set JavaScript
javascript_forms="
    if (isset('${port}_COM2_Port_Status','A')) 
    {
        set_visible('${port}_COM2_Chanel_Mode_field', 0);
        set_visible('${port}_COM2_Data_Baud_Rate_field', 0);
        set_visible('${port}_COM2_Data_Format_field', 0);
        set_visible('${port}_COM2_Flow_Control_field', 0);
        set_visible('${port}_COM2_Pre_Data_Delay_field', 0);
        set_visible('${port}_COM2_Post_Data_Delay_field', 0);
        set_visible('${port}_COM2_Data_Mode_field', 0);
        set_visible('${port}_COM2_Character_Timeout_field', 0);
        set_visible('${port}_COM2_Max_Packet_Len_field', 0);
        set_visible('${port}_COM2_QoS_field', 0);
        set_visible('${port}_COM2_NoConnect_Data_Intake_field', 0);
        set_visible('${port}_COM2_Modbus_Mode_field', 0);
        set_visible('${port}_COM2_IP_Protocol_field', 0);
        set_visible('${port}_T_Client_Server', 0);
        set_visible('${port}_U_PtoP', 0);
        set_visible('${port}_UM_P_Multicast', 0);
        set_visible('${port}_UM_M_Remote', 0);
        set_visible('${port}_UMtoM_Multicast', 0);
        set_visible('${port}_SMTP', 0);
        set_visible('${port}_PPP', 0);
        set_visible('${port}_SMS', 0);
        set_visible('${port}_SMS_Access_Control_Phone_List', 0);
    }
    else
    {
        set_visible('${port}_COM2_Chanel_Mode_field', 1);
        set_visible('${port}_COM2_Data_Baud_Rate_field', 1);
        set_visible('${port}_COM2_Data_Format_field', 1);
        set_visible('${port}_COM2_Flow_Control_field', 1);
        set_visible('${port}_COM2_Pre_Data_Delay_field', 1);
        set_visible('${port}_COM2_Post_Data_Delay_field', 1);
        set_visible('${port}_COM2_Data_Mode_field', 1);
        set_visible('${port}_COM2_Character_Timeout_field', 1);
        set_visible('${port}_COM2_Max_Packet_Len_field', 1);
        set_visible('${port}_COM2_QoS_field', 1);
        set_visible('${port}_COM2_NoConnect_Data_Intake_field', 1);
        set_visible('${port}_COM2_Modbus_Mode_field', 1);
        set_visible('${port}_COM2_IP_Protocol_field', 1);

       v = (isset('${port}_COM2_IP_Protocol', 'A') || isset('${port}_COM2_IP_Protocol', 'C') || isset('${port}_COM2_IP_Protocol', 'B'));
       set_visible('${port}_T_Client_Server', v);

       v = (isset('${port}_COM2_IP_Protocol', 'A') || isset('${port}_COM2_IP_Protocol', 'C'));
       set_visible('field_${port}_COM2_T_Client_Server_Addr', v);
       set_visible('field_${port}_COM2_T_Client_Server_Port', v);
       set_visible('field_${port}_COM2_T_Client_Timeout', v);

       v = (isset('${port}_COM2_IP_Protocol', 'B') || isset('${port}_COM2_IP_Protocol', 'C'));
       set_visible('field_${port}_COM2_T_Server_Listen_Port', v);

       v = (isset('${port}_COM2_IP_Protocol', 'B')); 
       set_visible('field_${port}_COM2_T_Server_Timeout', v);

       v = (isset('${port}_COM2_IP_Protocol', 'D'));
       set_visible('${port}_U_PtoP', v);
       set_visible('field_${port}_COM2_U_PtoP_Remote_Addr', v);
       set_visible('field_${port}_COM2_U_PtoP_Remote_Port', v);
       set_visible('field_${port}_COM2_U_PtoP_Listen_Port', v);

       v = (isset('${port}_COM2_IP_Protocol', 'E'));
       set_visible('${port}_UM_P_Multicast', v);
       set_visible('field_${port}_COM2_UM_P_Multicast_Addr', v);
       set_visible('field_${port}_COM2_UM_P_Multicast_Port', v);
       set_visible('field_${port}_COM2_UM_P_Listen_Port', v);
       set_visible('field_${port}_COM2_UM_P_TTL', v);



       v = (isset('${port}_COM2_IP_Protocol', 'F'));
       set_visible('${port}_UM_M_Remote', v);
       set_visible('field_${port}_COM2_UM_M_Remote_Addr', v);
       set_visible('field_${port}_COM2_UM_M_Remote_Port', v);
       set_visible('field_${port}_COM2_UM_M_Multicast_Addr', v);
       set_visible('field_${port}_COM2_UM_M_Multicast_Port', v);

       v = (isset('${port}_COM2_IP_Protocol', 'G'));
       set_visible('${port}_UMtoM_Multicast', v);
       set_visible('field_${port}_COM2_UMtoM_Multicast_Addr', v);
       set_visible('field_${port}_COM2_UMtoM_Multicast_Port', v);
       set_visible('field_${port}_COM2_UMtoM_Multicast_TTL', v);
       set_visible('field_${port}_COM2_UMtoM_Listen_Multicast_Addr', v);
       set_visible('field_${port}_COM2_UMtoM_Listen_Multicast_Port', v);

       v = (isset('${port}_COM2_IP_Protocol', 'H'));
       set_visible('${port}_SMTP', v);
       set_visible('field_${port}_COM2_SMTP_Mail_Subject', v);
       set_visible('field_${port}_COM2_SMTP_Server', v);
       set_visible('field_${port}_COM2_SMTP_Username', v);
       set_visible('field_${port}_COM2_SMTP_Password', v);
       set_visible('field_${port}_COM2_SMTP_Recipient', v);
       set_visible('field_${port}_COM2_SMTP_Buffer', v);
       set_visible('field_${port}_COM2_SMTP_Timeout', v);
       set_visible('field_${port}_COM2_SMTP_Transfer_Mode', v);

       v = (isset('${port}_COM2_IP_Protocol', 'I'));
       set_visible('${port}_PPP', v);
       set_visible('field_${port}_COM2_PPP_Local_IP', v);
       set_visible('field_${port}_COM2_PPP_Host_IP', v);
       set_visible('field_${port}_COM2_PPP_Idle_Timeout', v);

       v = (isset('${port}_COM2_IP_Protocol', 'J'));
       set_visible('${port}_SMS', v);
       set_visible('field_${port}_COM2_SMS_Message_Max_Size', v);
       set_visible('field_${port}_COM2_SMS_Reply_Timeout', v);
       set_visible('field_${port}_COM2_SMS_Access_Control', v);
       set_visible('field_${port}_COM2_SMS_Read_SMS_Control', v);
       set_visible('${port}_SMS_Access_Control_Phone_List', v);
       set_visible('field_${port}_COM2_SMS_Phone_Number_1', v);
       set_visible('field_${port}_COM2_SMS_Phone_Number_2', v);
       set_visible('field_${port}_COM2_SMS_Phone_Number_3', v);
       set_visible('field_${port}_COM2_SMS_Phone_Number_4', v);
       set_visible('field_${port}_COM2_SMS_Phone_Number_5', v);

       v = (isset('${port}_COM2_IP_Protocol', 'L'));
       set_visible('${port}_GPS', v);
       set_visible('field_${port}_COM2_GPS_status', v);

    } "

append js "$javascript_forms" "$N"

header "Comport" "Com2" "@TR<<Comport Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"


#####################################################################
# modechange script
#

cat <<EOF
<script type="text/javascript" src="/webif.js"></script>
<script type="text/javascript">
<!--
function modechange()
{
	var v;
	$js

	hide('save');
	show('save');
}
-->
</script>

EOF

display_form <<EOF
onchange|modechange
$validate_error
$forms
EOF

footer ?>
<!--
##WEBIF:name:Comport:201:Settings
-->
