#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"
###################################################################
# COM1 configuration page
#
# Description:
#	Configures COM1 port settings.
#
#
# UCI variables referenced:
#   todo
# Configuration files referenced:
#   /etc/config/comport
#

uci_load comport
#ports=$(echo "$port" |uniq)
port="com1"

if empty "$FORM_submit"; then
       config_get FORM_COM1_Port_Status $port COM1_Port_Status
       config_get FORM_COM1_Chanel_Mode $port COM1_Chanel_Mode
       config_get FORM_COM1_Data_Baud_Rate $port COM1_Data_Baud_Rate
       config_get FORM_COM1_Data_Format $port COM1_Data_Format
       config_get FORM_COM1_Flow_Control $port COM1_Flow_Control

       config_get FORM_COM1_Pre_Data_Delay $port COM1_Pre_Data_Delay
       config_get FORM_COM1_Post_Data_Delay $port COM1_Post_Data_Delay
       config_get FORM_COM1_Data_Mode $port COM1_Data_Mode

       config_get FORM_COM1_Character_Timeout $port COM1_Character_Timeout
       config_get FORM_COM1_Max_Packet_Len $port COM1_Max_Packet_Len
       config_get FORM_COM1_QoS $port COM1_QoS

       config_get FORM_COM1_NoConnect_Data_Intake $port COM1_NoConnect_Data_Intake
       config_get FORM_COM1_MODBUS_Mode $port COM1_MODBUS_Mode
       config_get FORM_COM1_IP_Protocol $port COM1_IP_Protocol

       config_get FORM_COM1_T_Client_Server_Addr $port COM1_T_Client_Server_Addr
       config_get FORM_COM1_T_Client_Server_Port $port COM1_T_Client_Server_Port
       config_get FORM_COM1_T_Client_Timeout $port COM1_T_Client_Timeout

       config_get FORM_COM1_T_Server_Listen_Port $port COM1_T_Server_Listen_Port
       config_get FORM_COM1_T_Server_Timeout $port COM1_T_Server_Timeout

       config_get FORM_COM1_U_PtoP_Remote_Addr $port COM1_U_PtoP_Remote_Addr
       config_get FORM_COM1_U_PtoP_Remote_Port $port COM1_U_PtoP_Remote_Port
       config_get FORM_COM1_U_PtoP_Listen_Port $port COM1_U_PtoP_Listen_Port

       config_get FORM_COM1_UM_P_Multicast_Addr $port COM1_UM_P_Multicast_Addr
       config_get FORM_COM1_UM_P_Multicast_Port $port COM1_UM_P_Multicast_Port
       config_get FORM_COM1_UM_P_Listen_Port $port COM1_UM_P_Listen_Port
       config_get FORM_COM1_UM_P_TTL $port COM1_UM_P_TTL

       config_get FORM_COM1_UM_M_Remote_Addr $port COM1_UM_M_Remote_Addr
       config_get FORM_COM1_UM_M_Remote_Port $port COM1_UM_M_Remote_Port
       config_get FORM_COM1_UM_M_Multicast_Addr $port COM1_UM_M_Multicast_Addr
       config_get FORM_COM1_UM_M_Multicast_Port $port COM1_UM_M_Multicast_Port

       config_get FORM_COM1_UMtoM_Multicast_Addr $port COM1_UMtoM_Multicast_Addr
       config_get FORM_COM1_UMtoM_Multicast_Port $port COM1_UMtoM_Multicast_Port
       config_get FORM_COM1_UMtoM_Multicast_TTL $port COM1_UMtoM_Multicast_TTL

       config_get FORM_COM1_UMtoM_Listen_Multicast_Addr $port COM1_UMtoM_Listen_Multicast_Addr
       config_get FORM_COM1_UMtoM_Listen_Multicast_Port $port COM1_UMtoM_Listen_Multicast_Port

       config_get FORM_COM1_SMTP_Mail_Subject $port COM1_SMTP_Mail_Subject
       config_get FORM_COM1_SMTP_Server $port COM1_SMTP_Server
       config_get FORM_COM1_SMTP_Recipient $port COM1_SMTP_Recipient
       config_get FORM_COM1_SMTP_Buffer $port COM1_SMTP_Buffer
       config_get FORM_COM1_SMTP_Timeout $port COM1_SMTP_Timeout
       config_get FORM_COM1_SMTP_Transfer_Mode $port COM1_SMTP_Transfer_Mode

       config_get FORM_COM1_C1222_Reg_to_Net $port COM1_C1222_Reg_to_Net
       config_get FORM_COM1_C1222_Log_Net_Comm $port COM1_C1222_Log_Net_Comm
       config_get FORM_COM1_C1222_Diff_Socket $port COM1_C1222_Diff_Socket
       config_get FORM_COM1_C1222_Reassembly_Packet $port COM1_C1222_Reassembly_Packet
       config_get FORM_COM1_C1222_Host_IP $port COM1_C1222_Host_IP
       config_get FORM_COM1_C1222_Host_Port $port COM1_C1222_Host_Port
       config_get FORM_COM1_C1222_Local_Port $port COM1_C1222_Local_Port
       
else
       eval FORM_COM1_Port_Status="\$FORM_${port}_COM1_Port_Status"
       eval FORM_COM1_Chanel_Mode="\$FORM_${port}_COM1_Chanel_Mode"
       eval FORM_COM1_Data_Baud_Rate="\$FORM_${port}_COM1_Data_Baud_Rate"
       eval FORM_COM1_Data_Format="\$FORM_${port}_COM1_Data_Format"
       eval FORM_COM1_Flow_Control="\$FORM_${port}_COM1_Flow_Control"

       eval FORM_COM1_Pre_Data_Delay="\$FORM_${port}_COM1_Pre_Data_Delay"
       eval FORM_COM1_Post_Data_Delay="\$FORM_${port}_COM1_Post_Data_Delay"
       eval FORM_COM1_Data_Mode="\$FORM_${port}_COM1_Data_Mode"
       eval FORM_COM1_Character_Timeout="\$FORM_${port}_COM1_Character_Timeout"
       eval FORM_COM1_Max_Packet_Len="\$FORM_${port}_COM1_Max_Packet_Len"
       eval FORM_COM1_QoS="\$FORM_${port}_COM1_QoS"

       eval FORM_COM1_NoConnect_Data_Intake="\$FORM_${port}_COM1_NoConnect_Data_Intake"
       eval FORM_COM1_MODBUS_Mode="\$FORM_${port}_COM1_MODBUS_Mode"
       eval FORM_COM1_IP_Protocol="\$FORM_${port}_COM1_IP_Protocol"

       eval FORM_COM1_T_Client_Server_Addr="\$FORM_${port}_COM1_T_Client_Server_Addr"
       eval FORM_COM1_T_Client_Server_Port="\$FORM_${port}_COM1_T_Client_Server_Port"
       eval FORM_COM1_T_Client_Timeout="\$FORM_${port}_COM1_T_Client_Timeout"
       eval FORM_COM1_T_Server_Listen_Port="\$FORM_${port}_COM1_T_Server_Listen_Port"
       eval FORM_COM1_T_Server_Timeout="\$FORM_${port}_COM1_T_Server_Timeout"

       eval FORM_COM1_U_PtoP_Remote_Addr="\$FORM_${port}_COM1_U_PtoP_Remote_Addr"
       eval FORM_COM1_U_PtoP_Remote_Port="\$FORM_${port}_COM1_U_PtoP_Remote_Port"
       eval FORM_COM1_U_PtoP_Listen_Port="\$FORM_${port}_COM1_U_PtoP_Listen_Port"

       eval FORM_COM1_UM_P_Multicast_Addr="\$FORM_${port}_COM1_UM_P_Multicast_Addr"
       eval FORM_COM1_UM_P_Multicast_Port="\$FORM_${port}_COM1_UM_P_Multicast_Port"
       eval FORM_COM1_UM_P_Listen_Port="\$FORM_${port}_COM1_UM_P_Listen_Port"
       eval FORM_COM1_UM_P_TTL="\$FORM_${port}_COM1_UM_P_TTL"

       eval FORM_COM1_UM_M_Remote_Addr="\$FORM_${port}_COM1_UM_M_Remote_Addr"
       eval FORM_COM1_UM_M_Remote_Port="\$FORM_${port}_COM1_UM_M_Remote_Port"
       eval FORM_COM1_UM_M_Multicast_Addr="\$FORM_${port}_COM1_UM_M_Multicast_Addr"
       eval FORM_COM1_UM_M_Multicast_Port="\$FORM_${port}_COM1_UM_M_Multicast_Port"

       eval FORM_COM1_UMtoM_Multicast_Addr="\$FORM_${port}_COM1_UMtoM_Multicast_Addr"
       eval FORM_COM1_UMtoM_Multicast_Port="\$FORM_${port}_COM1_UMtoM_Multicast_Port"
       eval FORM_COM1_UMtoM_Multicast_TTL="\$FORM_${port}_COM1_UMtoM_Multicast_TTL"

       eval FORM_COM1_UMtoM_Listen_Multicast_Addr="\$FORM_${port}_COM1_UMtoM_Listen_Multicast_Addr"
       eval FORM_COM1_UMtoM_Listen_Multicast_Port="\$FORM_${port}_COM1_UMtoM_Listen_Multicast_Port"
       eval FORM_COM1_SMTP_Mail_Subject="\$FORM_${port}_COM1_SMTP_Mail_Subject"
       eval FORM_COM1_SMTP_Server="\$FORM_${port}_COM1_SMTP_Server"
       eval FORM_COM1_SMTP_Recipient="\$FORM_${port}_COM1_SMTP_Recipient"
       eval FORM_COM1_SMTP_Buffer="\$FORM_${port}_COM1_SMTP_Buffer"
       eval FORM_COM1_SMTP_Timeout="\$FORM_${port}_COM1_SMTP_Timeout"
       eval FORM_COM1_SMTP_Transfer_Mode="\$FORM_${port}_COM1_SMTP_Transfer_Mode"

       eval FORM_COM1_C1222_Reg_to_Net="\$FORM_${port}_COM1_C1222_Reg_to_Net"
       eval FORM_COM1_C1222_Log_Net_Comm="\$FORM_${port}_COM1_C1222_Log_Net_Comm"
       eval FORM_COM1_C1222_Diff_Socket="\$FORM_${port}_COM1_C1222_Diff_Socket"
       eval FORM_COM1_C1222_Reassembly_Packet="\$FORM_${port}_COM1_C1222_Reassembly_Packet"
       eval FORM_COM1_C1222_Host_IP="\$FORM_${port}_COM1_C1222_Host_IP"
       eval FORM_COM1_C1222_Host_Port="\$FORM_${port}_COM1_C1222_Host_Port"
       eval FORM_COM1_C1222_Local_Port="\$FORM_${port}_COM1_C1222_Local_Port"

       validate <<EOF
int|FORM_${port}_COM1_Pre_Data_Delay|$port @TR<<Pre-Data Delay>>||$FORM_COM1_Pre_Data_Delay
int|FORM_${port}_COM1_Post_Data_Delay|$port @TR<<Post-Data Delay>>||$FORM_COM1_Post_Data_Delay
int|FORM_${port}_COM1_Character_Timeout|$port @TR<<Character Timeout>>||$FORM_COM1_Character_Timeout
int|FORM_${port}_COM1_Max_Packet_Len|$port @TR<<Maximum Packet Size>>||$FORM_COM1_Max_Packet_Len
 
ip|FORM_${port}_COM1_T_Client_Server_Addr|$port @TR<<Remote Server IP Address>>||$FORM_COM1_T_Client_Server_Addr
int|FORM_${port}_COM1_T_Client_Server_Port|$port @TR<<Remote Server port>>||$FORM_COM1_T_Client_Server_Port
int|FORM_${port}_COM1_T_Client_Timeout|$port @TR<<Outgoing Connection Timeout>>||$FORM_COM1_T_Client_Timeout
int|FORM_${port}_COM1_T_Server_Listen_Port|$port @TR<<Local Listening port>>||$FORM_COM1_T_Server_Listen_Port
int|FORM_${port}_COM1_T_Server_Timeout|$port @TR<<Incoming Connection Timeout>>||$FORM_COM1_T_Server_Timeout
 
ip|FORM_${port}_COM1_U_PtoP_Remote_Addr|$port @TR<<Remote IP Address>>||$FORM_COM1_U_PtoP_Remote_Addr
int|FORM_${port}_COM1_U_PtoP_Remote_Port|$port @TR<<Remote port>>||$FORM_COM1_U_PtoP_Remote_Port
int|FORM_${port}_COM1_U_PtoP_Listen_Port|$port @TR<<Listening port>>||$FORM_COM1_U_PtoP_Listen_Port

ip|FORM_${port}_COM1_UM_P_Multicast_Addr|$port @TR<<Multicast IP Address>>||$FORM_COM1_UM_P_Multicast_Addr
int|FORM_${port}_COM1_UM_P_Multicast_Port|$port @TR<<Multicast port>>||$FORM_COM1_UM_P_Multicast_Port
int|FORM_${port}_COM1_UM_P_Listen_Port|$port @TR<<Listening port>>||$FORM_COM1_UM_P_Listen_Port
int|FORM_${port}_COM1_UM_P_TTL|$port @TR<<Time To Live>>||$FORM_COM1_UM_P_TTL

ip|FORM_${port}_COM1_UM_M_Remote_Addr|$port @TR<<Remote IP Address>>||$FORM_COM1_UM_M_Remote_Addr
int|FORM_${port}_COM1_UM_M_Remote_Port|$port @TR<<Remot port>>||$FORM_COM1_UM_M_Remote_Port
ip|FORM_${port}_COM1_UM_M_Multicast_Addr|$port @TR<<Multicast IP Address>>||$FORM_COM1_UM_M_Multicast_Addr
int|FORM_${port}_COM1_UM_M_Multicast_Port|$port @TR<<Multicast Port>>||$FORM_COM1_UM_M_Multicast_Port

ip|FORM_${port}_COM1_UMtoM_Multicast_Addr|$port @TR<<Multicast IP Address>>||$FORM_COM1_UMtoM_Multicast_Addr
int|FORM_${port}_COM1_UMtoM_Multicast_Port|$port @TR<<Multicast port>>||$FORM_COM1_UMtoM_Multicast_Port
int|FORM_${port}_COM1_UMtoM_Multicast_TTL|$port @TR<<Time To Live>>||$FORM_COM1_UMtoM_Multicast_TTL
ip|FORM_${port}_COM1_UMtoM_Listen_Multicast_Addr|$port @TR<<Listen Multicast IP Address>>||$FORM_COM1_UMtoM_Listen_Multicast_Addr
int|FORM_${port}_COM1_UMtoM_Listen_Multicast_Port|$port @TR<<Listen Multicast Port>>||$FORM_COM1_UMtoM_Listen_Multicast_Port

int|FORM_${port}_COM1_SMTP_Buffer|$port @TR<<Message Max Size>>||$FORM_COM1_SMTP_Buffer
int|FORM_${port}_COM1_SMTP_Timeout|$port @TR<<TimeOut>>||$FORM_COM1_SMTP_Timeout

ip|FORM_${port}_COM1_C1222_Host_IP|$port @TR<<Host Server IP>>||$FORM_COM1_C1222_Host_IP
int|FORM_${port}_COM1_C1222_Host_Port|$port @TR<<Host Server Port>>||$FORM_COM1_C1222_Host_Port
int|FORM_${port}_COM1_C1222_Local_Port|$port @TR<<Local Server Port>>||$FORM_COM1_C1222_Local_Port

EOF

       [ "$?" = "0" ] && {
         uci_set "comport" "$port" "COM1_Port_Status" "$FORM_COM1_Port_Status"
         uci_set "comport" "$port" "COM1_Chanel_Mode" "$FORM_COM1_Chanel_Mode"
         uci_set "comport" "$port" "COM1_Data_Baud_Rate" "$FORM_COM1_Data_Baud_Rate"
         uci_set "comport" "$port" "COM1_Data_Format" "$FORM_COM1_Data_Format"
         uci_set "comport" "$port" "COM1_Flow_Control" "$FORM_COM1_Flow_Control"
         uci_set "comport" "$port" "COM1_Pre_Data_Delay" "$FORM_COM1_Pre_Data_Delay"
         uci_set "comport" "$port" "COM1_Post_Data_Delay" "$FORM_COM1_Post_Data_Delay"
         uci_set "comport" "$port" "COM1_Data_Mode" "$FORM_COM1_Data_Mode"
         uci_set "comport" "$port" "COM1_Character_Timeout" "$FORM_COM1_Character_Timeout"
         uci_set "comport" "$port" "COM1_Max_Packet_Len" "$FORM_COM1_Max_Packet_Len"
         uci_set "comport" "$port" "COM1_NoConnect_Data_Intake" "$FORM_COM1_NoConnect_Data_Intake"
         uci_set "comport" "$port" "COM1_QoS" "$FORM_COM1_QoS"
         uci_set "comport" "$port" "COM1_MODBUS_Mode" "$FORM_COM1_MODBUS_Mode"
         uci_set "comport" "$port" "COM1_IP_Protocol" "$FORM_COM1_IP_Protocol"

         case "$FORM_COM1_IP_Protocol" in
            "A")
               uci_set "comport" "$port" "COM1_T_Client_Server_Addr" "$FORM_COM1_T_Client_Server_Addr"
               uci_set "comport" "$port" "COM1_T_Client_Server_Port" "$FORM_COM1_T_Client_Server_Port"
               uci_set "comport" "$port" "COM1_T_Client_Timeout" "$FORM_COM1_T_Client_Timeout"
               ;;
            "B")
               uci_set "comport" "$port" "COM1_T_Server_Listen_Port" "$FORM_COM1_T_Server_Listen_Port"
               uci_set "comport" "$port" "COM1_T_Server_Timeout" "$FORM_COM1_T_Server_Timeout"
               ;;
            "C")
               uci_set "comport" "$port" "COM1_T_Client_Server_Addr" "$FORM_COM1_T_Client_Server_Addr"
               uci_set "comport" "$port" "COM1_T_Client_Server_Port" "$FORM_COM1_T_Client_Server_Port"
               uci_set "comport" "$port" "COM1_T_Client_Timeout" "$FORM_COM1_T_Client_Timeout"
               uci_set "comport" "$port" "COM1_T_Server_Listen_Port" "$FORM_COM1_T_Server_Listen_Port"
               ;;
            "D")
               uci_set "comport" "$port" "COM1_U_PtoP_Remote_Addr" "$FORM_COM1_U_PtoP_Remote_Addr"
               uci_set "comport" "$port" "COM1_U_PtoP_Remote_Port" "$FORM_COM1_U_PtoP_Remote_Port"
               uci_set "comport" "$port" "COM1_U_PtoP_Listen_Port" "$FORM_COM1_U_PtoP_Listen_Port"
               ;;
            "E")
               uci_set "comport" "$port" "COM1_UM_P_Multicast_Addr" "$FORM_COM1_UM_P_Multicast_Addr"
               uci_set "comport" "$port" "COM1_UM_P_Multicast_Port" "$FORM_COM1_UM_P_Multicast_Port"
               uci_set "comport" "$port" "COM1_UM_P_Listen_Port" "$FORM_COM1_UM_P_Listen_Port"
               uci_set "comport" "$port" "COM1_UM_P_TTL" "$FORM_COM1_UM_P_TTL"
               ;;
            "F")
               uci_set "comport" "$port" "COM1_UM_M_Remote_Addr" "$FORM_COM1_UM_M_Remote_Addr"
               uci_set "comport" "$port" "COM1_UM_M_Remote_Port" "$FORM_COM1_UM_M_Remote_Port"
               uci_set "comport" "$port" "COM1_UM_M_Multicast_Addr" "$FORM_COM1_UM_M_Multicast_Addr"
               uci_set "comport" "$port" "COM1_UM_M_Multicast_Port" "$FORM_COM1_UM_M_Multicast_Port"
               ;;
            "G")
               uci_set "comport" "$port" "COM1_UMtoM_Multicast_Addr" "$FORM_COM1_UMtoM_Multicast_Addr"
               uci_set "comport" "$port" "COM1_UMtoM_Multicast_Port" "$FORM_COM1_UMtoM_Multicast_Port"
               uci_set "comport" "$port" "COM1_UMtoM_Multicast_TTL" "$FORM_COM1_UMtoM_Multicast_TTL"
               uci_set "comport" "$port" "COM1_UMtoM_Listen_Multicast_Addr" "$FORM_COM1_UMtoM_Listen_Multicast_Addr"
               uci_set "comport" "$port" "COM1_UMtoM_Listen_Multicast_Port" "$FORM_COM1_UMtoM_Listen_Multicast_Port"
               ;;
            "H")
               uci_set "comport" "$port" "COM1_SMTP_Mail_Subject" "$FORM_COM1_SMTP_Mail_Subject"
               uci_set "comport" "$port" "COM1_SMTP_Server" "$FORM_COM1_SMTP_Server"
               uci_set "comport" "$port" "COM1_SMTP_Recipient" "$FORM_COM1_SMTP_Recipient"
               uci_set "comport" "$port" "COM1_SMTP_Buffer" "$FORM_COM1_SMTP_Buffer"
               uci_set "comport" "$port" "COM1_SMTP_Timeout" "$FORM_COM1_SMTP_Timeout"
               uci_set "comport" "$port" "COM1_SMTP_Transfer_Mode" "$FORM_COM1_SMTP_Transfer_Mode"
               ;;
            
            "I")
               uci_set "comport" "$port" "COM1_C1222_Reg_to_Net" "$FORM_COM1_C1222_Reg_to_Net"
               uci_set "comport" "$port" "COM1_C1222_Log_Net_Comm" "$FORM_COM1_C1222_Log_Net_Comm"
               uci_set "comport" "$port" "COM1_C1222_Diff_Socket" "$FORM_COM1_C1222_Diff_Socket"
               uci_set "comport" "$port" "COM1_C1222_Reassembly_Packet" "$FORM_COM1_C1222_Reassembly_Packet"
               uci_set "comport" "$port" "COM1_C1222_Host_IP" "$FORM_COM1_C1222_Host_IP"
               uci_set "comport" "$port" "COM1_C1222_Host_Port" "$FORM_COM1_C1222_Host_Port"
               uci_set "comport" "$port" "COM1_C1222_Local_Port" "$FORM_COM1_C1222_Local_Port"
               ;;
         esac
     }

fi
   
comport_options="start_form|$port @TR<<Configuration>>

field|@TR<<Com1 Port status>>
select|${port}_COM1_Port_Status|$FORM_COM1_Port_Status
option|B|@TR<<Enable>>
option|A|@TR<<Disable>>

field|@TR<<Channel Mode>>|${port}_COM1_Chanel_Mode_field|hidden
select|${port}_COM1_Chanel_Mode|$FORM_COM1_Chanel_Mode
option|A|@TR<<RS232>>
option|B|@TR<<RS485>>
option|C|@TR<<RS422>>
#helpitem|Channel Mode
#helptext|Helptext Channel Mode#Determines which serial interface shall be used to connect to external devices.

field|@TR<<Data Baud Rate>>|${port}_COM1_Data_Baud_Rate_field|hidden
select|${port}_COM1_Data_Baud_Rate|$FORM_COM1_Data_Baud_Rate
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

field|@TR<<Data Format>>|${port}_COM1_Data_Format_field|hidden
select|${port}_COM1_Data_Format|$FORM_COM1_Data_Format
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

field|@TR<<Flow Control>>|${port}_COM1_Flow_Control_field|hidden
select|${port}_COM1_Flow_Control|$FORM_COM1_Flow_Control
option|A|@TR<<none>>
option|B|@TR<<Hardware>>
option|C|@TR<<CTS Framing>>

field|@TR<<Pre-Data Delay (ms)>>|${port}_COM1_Pre_Data_Delay_field|hidden
text|${port}_COM1_Pre_Data_Delay|$FORM_COM1_Pre_Data_Delay

field|@TR<<Post-Data Delay (ms)>>|${port}_COM1_Post_Data_Delay_field|hidden
text|${port}_COM1_Post_Data_Delay|$FORM_COM1_Post_Data_Delay

field|@TR<<Data Mode>>|${port}_COM1_Data_Mode_field|hidden
radio|${port}_COM1_Data_Mode|$FORM_COM1_Data_Mode|A|@TR<<Seamless>>
radio|${port}_COM1_Data_Mode|$FORM_COM1_Data_Mode|B|@TR<<Transparent>>

field|@TR<<Character Timeout>>|${port}_COM1_Character_Timeout_field|hidden
text|${port}_COM1_Character_Timeout|$FORM_COM1_Character_Timeout

field|@TR<<Maximum Packet Size>>|${port}_COM1_Max_Packet_Len_field|hidden
text|${port}_COM1_Max_Packet_Len|$FORM_COM1_Max_Packet_Len

field|@TR<<Priority>>|${port}_COM1_QoS_field|hidden
radio|${port}_COM1_QoS|$FORM_COM1_QoS|A|@TR<<Normal>>
radio|${port}_COM1_QoS|$FORM_COM1_QoS|B|@TR<<Medium>>
radio|${port}_COM1_QoS|$FORM_COM1_QoS|C|@TR<<High>>

field|@TR<<No-Connection Data>>|${port}_COM1_NoConnect_Data_Intake_field|hidden
radio|${port}_COM1_NoConnect_Data_Intake|$FORM_COM1_NoConnect_Data_Intake|A|@TR<<Disable>>
radio|${port}_COM1_NoConnect_Data_Intake|$FORM_COM1_NoConnect_Data_Intake|B|@TR<<Enable>>

field|@TR<<MODBUS TCP Status>>|${port}_COM1_MODBUS_Mode_field|hidden
radio|${port}_COM1_MODBUS_Mode|$FORM_COM1_MODBUS_Mode|A|@TR<<Disable>>
radio|${port}_COM1_MODBUS_Mode|$FORM_COM1_MODBUS_Mode|B|@TR<<Enable>>

field|@TR<<IP Protocol Config>>|${port}_COM1_IP_Protocol_field|hidden
select|${port}_COM1_IP_Protocol|$FORM_COM1_IP_Protocol
option|A|@TR<<TCP Client>>
option|B|@TR<<TCP Server>>
option|C|@TR<<TCP Client/Server>>
option|D|@TR<<UDP Point to Point>>
option|E|@TR<<UDP Point to Multipoint(P)>>
option|F|@TR<<UDP Point to Multipoint(MP)>>
option|G|@TR<<UDP Multipoint to Multipoint>>
option|H|@TR<<SMTP Client>>
option|I|@TR<<C12.22>>
end_form

start_form|@TR<<TCP Configuration>>|${port}_T_Client_Server|hidden
field|@TR<<Remote Server IP Address>>|field_${port}_COM1_T_Client_Server_Addr|hidden
text|${port}_COM1_T_Client_Server_Addr|$FORM_COM1_T_Client_Server_Addr
field|@TR<<Remote Server port>>|field_${port}_COM1_T_Client_Server_Port|hidden
text|${port}_COM1_T_Client_Server_Port|$FORM_COM1_T_Client_Server_Port
field|@TR<<Outgoing Connection Timeout>>|field_${port}_COM1_T_Client_Timeout|hidden
text|${port}_COM1_T_Client_Timeout|$FORM_COM1_T_Client_Timeout
field|@TR<<Local Listening port>>|field_${port}_COM1_T_Server_Listen_Port|hidden
text|${port}_COM1_T_Server_Listen_Port|$FORM_COM1_T_Server_Listen_Port
field|@TR<<Incoming Connection Timeout>>|field_${port}_COM1_T_Server_Timeout|hidden
text|${port}_COM1_T_Server_Timeout|$FORM_COM1_T_Server_Timeout
end_form

start_form|@TR<<UDP Configurate>>|${port}_U_PtoP|hidden
field|@TR<<Remote IP Address>>|field_${port}_COM1_U_PtoP_Remote_Addr|hidden
text|${port}_COM1_U_PtoP_Remote_Addr|$FORM_COM1_U_PtoP_Remote_Addr
field|@TR<<Remote port>>|field_${port}_COM1_U_PtoP_Remote_Port|hidden
text|${port}_COM1_U_PtoP_Remote_Port|$FORM_COM1_U_PtoP_Remote_Port
field|@TR<<Listening port>>|field_${port}_COM1_U_PtoP_Listen_Port|hidden
text|${port}_COM1_U_PtoP_Listen_Port|$FORM_COM1_U_PtoP_Listen_Port
end_form

start_form|@TR<<UDP Configurate>>|${port}_UM_P_Multicast|hidden
field|@TR<<Multicast IP Address>>|field_${port}_COM1_UM_P_Multicast_Addr|hidden
text|${port}_COM1_UM_P_Multicast_Addr|$FORM_COM1_UM_P_Multicast_Addr
field|@TR<<Multicast port>>|field_${port}_COM1_UM_P_Multicast_Port|hidden
text|${port}_COM1_UM_P_Multicast_Port|$FORM_COM1_UM_P_Multicast_Port
field|@TR<<Listening port>>|field_${port}_COM1_UM_P_Listen_Port|hidden
text|${port}_COM1_UM_P_Listen_Port|$FORM_COM1_UM_P_Listen_Port
field|@TR<<Time To Live>>|field_${port}_COM1_UM_P_TTL|hidden
text|${port}_COM1_UM_P_TTL|$FORM_COM1_UM_P_TTL
end_form

start_form|@TR<<UDP Configurate>>|${port}_UM_M_Remote|hidden
field|@TR<<Remote IP Address>>|field_${port}_COM1_UM_M_Remote_Addr|hidden
text|${port}_COM1_UM_M_Remote_Addr|$FORM_COM1_UM_M_Remote_Addr
field|@TR<<Remot port>>|field_${port}_COM1_UM_M_Remote_Port|hidden
text|${port}_COM1_UM_M_Remote_Port|$FORM_COM1_UM_M_Remote_Port
field|@TR<<Multicast IP Address>>|field_${port}_COM1_UM_M_Multicast_Addr|hidden
text|${port}_COM1_UM_M_Multicast_Addr|$FORM_COM1_UM_M_Multicast_Addr
field|@TR<<Multicast Port>>|field_${port}_COM1_UM_M_Multicast_Port|hidden
text|${port}_COM1_UM_M_Multicast_Port|$FORM_COM1_UM_M_Multicast_Port
end_form

start_form|@TR<<UDP Configurate>>|${port}_UMtoM_Multicast|hidden
field|@TR<<Multicast IP Address>>|field_${port}_COM1_UMtoM_Multicast_Addr|hidden
text|${port}_COM1_UMtoM_Multicast_Addr|$FORM_COM1_UMtoM_Multicast_Addr
field|@TR<<Multicast port>>|field_${port}_COM1_UMtoM_Multicast_Port|hidden
text|${port}_COM1_UMtoM_Multicast_Port|$FORM_COM1_UMtoM_Multicast_Port
field|@TR<<Time To Live>>|field_${port}_COM1_UMtoM_Multicast_TTL|hidden
text|${port}_COM1_UMtoM_Multicast_TTL|$FORM_COM1_UMtoM_Multicast_TTL
field|@TR<<Listen Multicast IP Address>>|field_${port}_COM1_UMtoM_Listen_Multicast_Addr|hidden
text|${port}_COM1_UMtoM_Listen_Multicast_Addr|$FORM_COM1_UMtoM_Listen_Multicast_Addr
field|@TR<<Listen Multicast Port>>|field_${port}_COM1_UMtoM_Listen_Multicast_Port|hidden
text|${port}_COM1_UMtoM_Listen_Multicast_Port|$FORM_COM1_UMtoM_Listen_Multicast_Port
end_form

start_form|@TR<<SMTP Configuration>>|${port}_SMTP|hidden
field|@TR<<Mail Subject>>|field_${port}_COM1_SMTP_Mail_Subject|hidden
text|${port}_COM1_SMTP_Mail_Subject|$FORM_COM1_SMTP_Mail_Subject
field|@TR<<Mail Server (IP/Name)>>|field_${port}_COM1_SMTP_Server|hidden
text|${port}_COM1_SMTP_Server|$FORM_COM1_SMTP_Server
field|@TR<<Mail Recipient>>|field_${port}_COM1_SMTP_Recipient|hidden
text|${port}_COM1_SMTP_Recipient|$FORM_COM1_SMTP_Recipient
field|@TR<<Message Max Size>>|field_${port}_COM1_SMTP_Buffer|hidden
text|${port}_COM1_SMTP_Buffer|$FORM_COM1_SMTP_Buffer
field|@TR<<TimeOut(s)>>|field_${port}_COM1_SMTP_Timeout|hidden
text|${port}_COM1_SMTP_Timeout|$FORM_COM1_SMTP_Timeout
field|@TR<<Transfer Mode>>|field_${port}_COM1_SMTP_Transfer_Mode|hidden
text|${port}_COM1_SMTP_Transfer_Mode|$FORM_COM1_SMTP_Transfer_Mode
end_form

start_form|@TR<<C12.22 Configuration>>|${port}_C1222|hidden
field|@TR<<Register to Network>>|field_${port}_COM1_C1222_Reg_to_Net|hidden
radio|${port}_COM1_C1222_Reg_to_Net|$FORM_COM1_C1222_Reg_to_Net|A|@TR<<Disable>>
radio|${port}_COM1_C1222_Reg_to_Net|$FORM_COM1_C1222_Reg_to_Net|B|@TR<<Enable>>
field|@TR<<Log Network Communication>>|field_${port}_COM1_C1222_Log_Net_Comm|hidden
radio|${port}_COM1_C1222_Log_Net_Comm|$FORM_COM1_C1222_Log_Net_Comm|A|@TR<<Disable>>
radio|${port}_COM1_C1222_Log_Net_Comm|$FORM_COM1_C1222_Log_Net_Comm|B|@TR<<Enable>>
field|@TR<<Using Different Socket>>|field_${port}_COM1_C1222_Diff_Socket|hidden
radio|${port}_COM1_C1222_Diff_Socket|$FORM_COM1_C1222_Diff_Socket|A|@TR<<Disable>>
radio|${port}_COM1_C1222_Diff_Socket|$FORM_COM1_C1222_Diff_Socket|B|@TR<<Enable>>
field|@TR<<Reassembly Packet>>|field_${port}_COM1_C1222_Reassembly_Packet|hidden
radio|${port}_COM1_C1222_Reassembly_Packet|$FORM_COM1_C1222_Reassembly_Packet|A|@TR<<Disable>>
radio|${port}_COM1_C1222_Reassembly_Packet|$FORM_COM1_C1222_Reassembly_Packet|B|@TR<<Enable>>
field|@TR<<Host Server IP>>|field_${port}_COM1_C1222_Host_IP|hidden
text|${port}_COM1_C1222_Host_IP|$FORM_COM1_C1222_Host_IP
field|@TR<<Host Server Port>>|field_${port}_COM1_C1222_Host_Port|hidden
text|${port}_COM1_C1222_Host_Port|$FORM_COM1_C1222_Host_Port
field|@TR<<Local Server Port>>|field_${port}_COM1_C1222_Local_Port|hidden
text|${port}_COM1_C1222_Local_Port|$FORM_COM1_C1222_Local_Port
end_form"


append forms "$comport_options" "$N"

###################################################################
# set JavaScript
javascript_forms="
    if (isset('${port}_COM1_Port_Status','A')) 
    {
        set_visible('${port}_COM1_Chanel_Mode_field', 0);
        set_visible('${port}_COM1_Data_Baud_Rate_field', 0);
        set_visible('${port}_COM1_Data_Format_field', 0);
        set_visible('${port}_COM1_Flow_Control_field', 0);
        set_visible('${port}_COM1_Pre_Data_Delay_field', 0);
        set_visible('${port}_COM1_Post_Data_Delay_field', 0);
        set_visible('${port}_COM1_Data_Mode_field', 0);
        set_visible('${port}_COM1_Character_Timeout_field', 0);
        set_visible('${port}_COM1_Max_Packet_Len_field', 0);
        set_visible('${port}_COM1_QoS_field', 0);
        set_visible('${port}_COM1_NoConnect_Data_Intake_field', 0);
        set_visible('${port}_COM1_MODBUS_Mode_field', 0);
        set_visible('${port}_COM1_IP_Protocol_field', 0);
        set_visible('${port}_T_Client_Server', 0);
        set_visible('${port}_U_PtoP', 0);
        set_visible('${port}_UM_P_Multicast', 0);
        set_visible('${port}_UM_M_Remote', 0);
        set_visible('${port}_UMtoM_Multicast', 0);
        set_visible('${port}_SMTP', 0);
        set_visible('${port}_C1222', 0);
    }
    else
    {
        set_visible('${port}_COM1_Chanel_Mode_field', 1);
        set_visible('${port}_COM1_Data_Baud_Rate_field', 1);
        set_visible('${port}_COM1_Data_Format_field', 1);
        set_visible('${port}_COM1_Flow_Control_field', 1);
        set_visible('${port}_COM1_Pre_Data_Delay_field', 1);
        set_visible('${port}_COM1_Post_Data_Delay_field', 1);
        set_visible('${port}_COM1_Data_Mode_field', 1);
        set_visible('${port}_COM1_Character_Timeout_field', 1);
        set_visible('${port}_COM1_Max_Packet_Len_field', 1);
        set_visible('${port}_COM1_QoS_field', 1);
        set_visible('${port}_COM1_NoConnect_Data_Intake_field', 1);
        set_visible('${port}_COM1_MODBUS_Mode_field', 1);
        set_visible('${port}_COM1_IP_Protocol_field', 1);

       v = (isset('${port}_COM1_IP_Protocol', 'A') || isset('${port}_COM1_IP_Protocol', 'C') || isset('${port}_COM1_IP_Protocol', 'B'));
       set_visible('${port}_T_Client_Server', v);

       v = (isset('${port}_COM1_IP_Protocol', 'A') || isset('${port}_COM1_IP_Protocol', 'C'));
       set_visible('field_${port}_COM1_T_Client_Server_Addr', v);
       set_visible('field_${port}_COM1_T_Client_Server_Port', v);
       set_visible('field_${port}_COM1_T_Client_Timeout', v);

       v = (isset('${port}_COM1_IP_Protocol', 'B') || isset('${port}_COM1_IP_Protocol', 'C'));
       set_visible('field_${port}_COM1_T_Server_Listen_Port', v);

       v = (isset('${port}_COM1_IP_Protocol', 'B')); 
       set_visible('field_${port}_COM1_T_Server_Timeout', v);

       v = (isset('${port}_COM1_IP_Protocol', 'D'));
       set_visible('${port}_U_PtoP', v);
       set_visible('field_${port}_COM1_U_PtoP_Remote_Addr', v);
       set_visible('field_${port}_COM1_U_PtoP_Remote_Port', v);
       set_visible('field_${port}_COM1_U_PtoP_Listen_Port', v);

       v = (isset('${port}_COM1_IP_Protocol', 'E'));
       set_visible('${port}_UM_P_Multicast', v);
       set_visible('field_${port}_COM1_UM_P_Multicast_Addr', v);
       set_visible('field_${port}_COM1_UM_P_Multicast_Port', v);
       set_visible('field_${port}_COM1_UM_P_Listen_Port', v);
       set_visible('field_${port}_COM1_UM_P_TTL', v);



       v = (isset('${port}_COM1_IP_Protocol', 'F'));
       set_visible('${port}_UM_M_Remote', v);
       set_visible('field_${port}_COM1_UM_M_Remote_Addr', v);
       set_visible('field_${port}_COM1_UM_M_Remote_Port', v);
       set_visible('field_${port}_COM1_UM_M_Multicast_Addr', v);
       set_visible('field_${port}_COM1_UM_M_Multicast_Port', v);

       v = (isset('${port}_COM1_IP_Protocol', 'G'));
       set_visible('${port}_UMtoM_Multicast', v);
       set_visible('field_${port}_COM1_UMtoM_Multicast_Addr', v);
       set_visible('field_${port}_COM1_UMtoM_Multicast_Port', v);
       set_visible('field_${port}_COM1_UMtoM_Multicast_TTL', v);
       set_visible('field_${port}_COM1_UMtoM_Listen_Multicast_Addr', v);
       set_visible('field_${port}_COM1_UMtoM_Listen_Multicast_Port', v);

       v = (isset('${port}_COM1_IP_Protocol', 'H'));
       set_visible('${port}_SMTP', v);
       set_visible('field_${port}_COM1_SMTP_Mail_Subject', v);
       set_visible('field_${port}_COM1_SMTP_Server', v);
       set_visible('field_${port}_COM1_SMTP_Recipient', v);
       set_visible('field_${port}_COM1_SMTP_Buffer', v);
       set_visible('field_${port}_COM1_SMTP_Timeout', v);
       set_visible('field_${port}_COM1_SMTP_Transfer_Mode', v);
       
       

       v = (isset('${port}_COM1_IP_Protocol', 'I'));
       set_visible('${port}_C1222', v);
       set_visible('field_${port}_COM1_C1222_Reg_to_Net', v);
       set_visible('field_${port}_COM1_C1222_Log_Net_Comm', v);
       set_visible('field_${port}_COM1_C1222_Diff_Socket', v);
       set_visible('field_${port}_COM1_C1222_Reassembly_Packet', v);
       set_visible('field_${port}_COM1_C1222_Host_IP', v);
       set_visible('field_${port}_COM1_C1222_Host_Port', v);
       set_visible('field_${port}_COM1_C1222_Local_Port', v);
    } "

append js "$javascript_forms" "$N"

header "Comport" "Com1" "@TR<<Comport Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"


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
#<!--
###WEBIF:name:Comport:101:Com1
#-->
