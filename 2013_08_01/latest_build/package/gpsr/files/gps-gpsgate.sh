#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"

uci_load gpsd
conf="gpsd_conf"
config_get FORM_status $conf status
if [ "$FORM_status" = "1" ]; then



uci_load "gpsgatetr"
conf="gpsgatetr_conf"

if empty "$FORM_submit"; then

    conf="gpsgatetr_conf"

else

 	FORM_TRACKER_Status="$FORM_TRACKER_Status_gate"
 	FORM_SERVER_Mode="$FORM_SERVER_Mode_gate"
 	FORM_TCP_ALIVE_Mode="$FORM_TCP_ALIVE_Mode_gate"
 	FORM_TCP_ALIVE_Interval="$FORM_TCP_ALIVE_Interval_gate"
 	FORM_SERVER_PhoneFilt_En="$FORM_SERVER_PhoneFilt_En_gate"
 	FORM_SERVER_Phone1="$FORM_SERVER_Phone1_gate"
 	FORM_SERVER_Phone2="$FORM_SERVER_Phone2_gate"
 	FORM_SERVER_Phone3="$FORM_SERVER_Phone3_gate"
 	FORM_Motion_Set="$FORM_Motion_Set_gate"
 	FORM_Motion_Distance="$FORM_Motion_Distance_gate"
 	FORM_SERVER_Address="$FORM_SERVER_Address_gate"
 	FORM_SERVER_Port="$FORM_SERVER_Port_gate"
 	FORM_SERVER_IOStatus="$FORM_SERVER_IOStatus_gate"
 	FORM_Last_Position="$FORM_Last_Position_gate"
 	FORM_SERVER_Interval="$FORM_SERVER_Interval_gate"
 	FORM_SERVER_Conn_StartTr_Status="$FORM_SERVER_Conn_StartTr_Status_gate"


validate <<EOF
int|$FORM_TCP_ALIVE_Interval_gate|@TR<<TCP ALIVE Interval>>||$FORM_TCP_ALIVE_Interval
int|$FORM_Motion_Distance_gate|@TR<<Motion Distance>>||$FORM_Motion_Distance
int|$FORM_SERVER_Port_gate|@TR<<SERVER Port>>||$FORM_SERVER_Port
int|$FORM_SERVER_Interval_gate|@TR<<SERVER Interval>>||$FORM_SERVER_Interval
EOF


       [ "$?" = "0" ] && {

            uci_set "gpsgatetr" "$conf" "TRACKER_Status" "$FORM_TRACKER_Status"
            uci_set "gpsgatetr" "$conf" "SERVER_Mode" "$FORM_SERVER_Mode"
            uci_set "gpsgatetr" "$conf" "TCP_ALIVE_Mode" "$FORM_TCP_ALIVE_Mode"
            uci_set "gpsgatetr" "$conf" "TCP_ALIVE_Interval" "$FORM_TCP_ALIVE_Interval"
            uci_set "gpsgatetr" "$conf" "SERVER_PhoneFilt_En" "$FORM_SERVER_PhoneFilt_En"
            uci_set "gpsgatetr" "$conf" "SERVER_Phone1" "$FORM_SERVER_Phone1"
            uci_set "gpsgatetr" "$conf" "SERVER_Phone2" "$FORM_SERVER_Phone2"
            uci_set "gpsgatetr" "$conf" "SERVER_Phone3" "$FORM_SERVER_Phone3"
            uci_set "gpsgatetr" "$conf" "Motion_Set" "$FORM_Motion_Set"
            uci_set "gpsgatetr" "$conf" "Motion_Distance" "$FORM_Motion_Distance"
            uci_set "gpsgatetr" "$conf" "SERVER_Address" "$FORM_SERVER_Address"
            uci_set "gpsgatetr" "$conf" "SERVER_Port" "$FORM_SERVER_Port"
            uci_set "gpsgatetr" "$conf" "SERVER_Interval" "$FORM_SERVER_Interval"
            uci_set "gpsgatetr" "$conf" "SERVER_IOStatus" "$FORM_SERVER_IOStatus"
            uci_set "gpsgatetr" "$conf" "Last_Position" "$FORM_Last_Position"

         }
fi



uci_load "gpsgatetr"
       config_get FORM_TRACKER_Status $conf TRACKER_Status
       config_get FORM_SERVER_Mode $conf SERVER_Mode
       config_get FORM_TCP_ALIVE_Mode $conf TCP_ALIVE_Mode
       config_get FORM_TCP_ALIVE_Interval $conf TCP_ALIVE_Interval
       config_get FORM_SERVER_PhoneFilt_En $conf SERVER_PhoneFilt_En
       config_get FORM_SERVER_Phone1 $conf SERVER_Phone1
       config_get FORM_SERVER_Phone2 $conf SERVER_Phone2
       config_get FORM_SERVER_Phone3 $conf SERVER_Phone3
       config_get FORM_Motion_Set $conf Motion_Set
       config_get FORM_Motion_Distance $conf Motion_Distance
       config_get FORM_SERVER_Address $conf SERVER_Address
       config_get FORM_SERVER_Port $conf SERVER_Port
       config_get FORM_SERVER_Interval $conf SERVER_Interval
       config_get FORM_SERVER_IOStatus $conf SERVER_IOStatus
       config_get FORM_Last_Position $conf Last_Position

       config_get FORM_SERVER_Setup_Status $conf SERVER_Setup_Status
       config_get FORM_SERVER_Conn_Phone $conf SERVER_Conn_Phone
       config_get FORM_SERVER_Conn_TCP_Status $conf SERVER_Conn_TCP_Status
       config_get FORM_SERVER_Conn_Domain $conf SERVER_Conn_Domain
       config_get FORM_SERVER_Conn_Port $conf SERVER_Conn_Port
       config_get FORM_SERVER_Conn_STime $conf SERVER_Conn_STime
       config_get FORM_SERVER_Conn_TCPTimeFilter $conf SERVER_Conn_TCPTimeFilter
       config_get FORM_SERVER_Conn_SMSTimeFilter $conf SERVER_Conn_SMSTimeFilter
       config_get FORM_SERVER_Conn_MotionSet $conf SERVER_Conn_MotionSet
       config_get FORM_TRACKER_IMEI $conf TRACKER_IMEI
       config_get FORM_TRACKER_Type $conf TRACKER_Type
       config_get FORM_TRACKER_Brand $conf TRACKER_Brand
       config_get FORM_TRACKER_Version $conf TRACKER_Version
       config_get FORM_SERVER_Conn_StartTr_Status $conf SERVER_Conn_StartTr_Status

    if [ "B" = "$FORM_SERVER_Setup_Status" ]; then
        conn_str="Setup Enabled"
    else
        conn_str="Not Setup"
    fi
    if [ "B" = "$FORM_SERVER_Conn_StartTr_Status" ]; then
        FORM_SERVER_Conn_StartTr_Status_str="Start Enabled"
    else
        FORM_SERVER_Conn_StartTr_Status_str="Stopped"
    fi


    if [ "1" = "$FORM_SERVER_Conn_MotionSet" ]; then
        FORM_SERVER_Conn_MotionSet_str="Enabled"
    else
        FORM_SERVER_Conn_MotionSet_str="Disabled"
    fi
    if [ "B" = "$FORM_SERVER_Conn_TCP_Status" ]; then
        FORM_SERVER_Conn_TCP_Status_str="Linked"
    else
        FORM_SERVER_Conn_TCP_Status_str="Not Available"
    fi

#field|@TR<<&nbsp;&nbsp;AAA>>|_gpsr_field|hidden
#text|_gpsr|$FORM_|@TR<<>>
#field|@TR<<&nbsp;&nbsp;AAA>>|_gpsr_field|hidden
#select|_gpsr|$FORM_
#option|A|@TR<<Disable>>
#option|D|@TR<<GpsGate Protocol UDP Report>>
config_options=""

if [ "B" = "$FORM_TRACKER_Status" ]; then
config_options="
start_form| @TR<<Server Connecting Infomation>>
field|@TR<< <strong>Setup Status:<strong> >>|SERVER_Setup_Status_gate_field|1
string| $conn_str
"

if [ "B" = "$FORM_SERVER_Setup_Status" ]; then
config_options=$config_options"
field|@TR<<&nbsp;&nbsp;Tracker Status:>>|SERVER_Conn_StartTr_Status_gate_field|1
string| $FORM_SERVER_Conn_StartTr_Status_str
field|@TR<<&nbsp;&nbsp;TCP Status:>>|SERVER_Setup_Status_gate_field|1
string| $FORM_SERVER_Conn_TCP_Status_str
field|@TR<<&nbsp;&nbsp;Server Phone:>>|SERVER_Setup_Status_gate_field|1
string| $FORM_SERVER_Conn_Phone
field|@TR<<&nbsp;&nbsp;TCP Server:>>|SERVER_Setup_Status_gate_field|1
string| $FORM_SERVER_Conn_Domain
field|@TR<<&nbsp;&nbsp;TCP Port:>>|SERVER_Setup_Status_gate_field|1
string| $FORM_SERVER_Conn_Port
field|@TR<<&nbsp;&nbsp;TCP Time Filter(s):>>|SERVER_Setup_Status_gate_field|1
string| $FORM_SERVER_Conn_TCPTimeFilter
field|@TR<<&nbsp;&nbsp;SMS Time Filter(s):>>|SERVER_Setup_Status_gate_field|1
string| $FORM_SERVER_Conn_SMSTimeFilter
field|@TR<<&nbsp;&nbsp;Motion Enable:>>|SERVER_Setup_Status_gate_field|1
string| $FORM_SERVER_Conn_MotionSet_str
field|@TR<<&nbsp;&nbsp;Setup Time:>>|SERVER_Setup_Status_gate_field|1
string| $FORM_SERVER_Conn_STime
"
fi

config_options=$config_options"
end_form
"
fi

#option|A|@TR<<Send Position Message>>
#radio|TRACKER_Status_gate|$FORM_TRACKER_Status|A|@TR<<Disable>>
#radio|TRACKER_Status_gate|$FORM_TRACKER_Status|B|@TR<<Tracker Mode>>
#radio|TRACKER_Status_gate|$FORM_TRACKER_Status|C|@TR<<TCP Send Mode>>

config_options=$config_options"
start_form| @TR<<Tracker Device Setting>>
field|@TR<< <strong>Mode Set<strong> >>|TRACKER_Status_gate_field|1
select|TRACKER_Status_gate|$FORM_TRACKER_Status
option|A|@TR<<Disable>>
option|B|@TR<<Enable Tracker Mode>>
option|C|@TR<<Enable TCP Send Mode>>

field|@TR<<&nbsp;&nbsp;Server Address/IP>>|SERVER_Address_gate_field|hidden
text|SERVER_Address_gate|$FORM_SERVER_Address
field|@TR<<&nbsp;&nbsp;Server Port>>|SERVER_Port_gate_field|hidden
text|SERVER_Port_gate|$FORM_SERVER_Port
field|@TR<<&nbsp;&nbsp;Server Interval>>|SERVER_Interval_gate_field|hidden
text|SERVER_Interval_gate|$FORM_SERVER_Interval|@TR<<(s)>>

field|@TR<<&nbsp;&nbsp;Server Command Channel>>|SERVER_Mode_gate_field|hidden
select|SERVER_Mode_gate|$FORM_SERVER_Mode
option|A|@TR<<TCP and SMS>>
option|B|@TR<<TCP Only>>
option|C|@TR<<SMS Only>>

field|@TR<<&nbsp;&nbsp;TCP Alive Mode>>|TCP_ALIVE_Mode_gate_field|hidden
select|TCP_ALIVE_Mode_gate|$FORM_TCP_ALIVE_Mode
option|B|@TR<<_Ping Command>>

field|@TR<<&nbsp;&nbsp;Alive Time Interval>>|TCP_ALIVE_Interval_gate_field|hidden
text|TCP_ALIVE_Interval_gate|$FORM_TCP_ALIVE_Interval|@TR<<(s)>>

field|@TR<<&nbsp;&nbsp;Setup Phone Filter>>|SERVER_PhoneFilt_En_gate_field|hidden
select|SERVER_PhoneFilt_En_gate|$FORM_SERVER_PhoneFilt_En
option|A|@TR<<Disable: Accept All>>
option|B|@TR<<Enable Filter>>

field|@TR<<&nbsp;&nbsp;Accept Phone No.1>>|SERVER_Phone1_gate_field|hidden
text|SERVER_Phone1_gate|$FORM_SERVER_Phone1
field|@TR<<&nbsp;&nbsp;Accept Phone No.2>>|SERVER_Phone2_gate_field|hidden
text|SERVER_Phone2_gate|$FORM_SERVER_Phone2
field|@TR<<&nbsp;&nbsp;Accept Phone No.3>>|SERVER_Phone3_gate_field|hidden
text|SERVER_Phone3_gate|$FORM_SERVER_Phone3

field|@TR<<&nbsp;&nbsp;Motion Trigger>>|Motion_Set_gate_field|hidden
select|Motion_Set_gate|$FORM_Motion_Set
option|0|@TR<<Disable>>
option|1|@TR<<Enable Motion Trigger>>

field|@TR<<&nbsp;&nbsp;Motion Distance>>|Motion_Distance_gate_field|hidden
text|Motion_Distance_gate|$FORM_Motion_Distance|@TR<<(m)>>

field|@TR<<&nbsp;&nbsp;Send IO Status>>|SERVER_IOStatus_gate_field|hidden
select|SERVER_IOStatus_gate|$FORM_SERVER_IOStatus
option|A|@TR<<Disable>>
option|B|@TR<<Send Input Status>>
option|C|@TR<<Send Ouput Status>>
option|D|@TR<<Send Input&Output Status>>

field|@TR<<&nbsp;&nbsp;When GPS Invalid, Sending Data>>|Last_Position_gate_field|hidden
select|Last_Position_gate|$FORM_Last_Position
option|A|@TR<<Not Use Last Valid Position >>
option|B|@TR<<Use Last Valid Position>>


end_form
"

append forms "$config_options" "$N"


######################################################################################
# set JavaScript
javascript_forms="
    v_en_B = isset('TRACKER_Status_gate','B');
    v_en_C = isset('TRACKER_Status_gate','C');
    if(v_en_B||v_en_C)v_en=1;
    else v_en=0;
    v_smso = isset('SERVER_Mode_gate','C');
    v_phone = isset('SERVER_PhoneFilt_En_gate','B');
    v_motion = isset('Motion_Set_gate','1');

    set_visible('SERVER_Address_gate_field', v_en_C);
    set_visible('SERVER_Port_gate_field', v_en_C);
    set_visible('SERVER_Interval_gate_field', v_en_C);

    set_visible('SERVER_Mode_gate_field', v_en_B);
    if(v_smso==1)v_sms=0;
    else v_sms=1;
    if(v_sms&&v_en_B)v_sms=1;
    else v_sms=0;
    set_visible('TCP_ALIVE_Mode_gate_field', v_sms);
    set_visible('TCP_ALIVE_Interval_gate_field', v_sms);

    set_visible('SERVER_PhoneFilt_En_gate_field', v_en_B);
    if((v_en_B)&&(v_phone))v_phone=1;
    else v_phone=0;
    set_visible('SERVER_Phone1_gate_field', v_phone);
    set_visible('SERVER_Phone2_gate_field', v_phone);
    set_visible('SERVER_Phone3_gate_field', v_phone);

    set_visible('Motion_Set_gate_field', v_en_B);
    v_motion=0;
    if((v_en_B)&&(v_motion))v_motion=1;
    if(v_en_C)v_motion=1;
    set_visible('Motion_Distance_gate_field', v_motion);

    set_visible('SERVER_IOStatus_gate_field', v_en);
    set_visible('Last_Position_gate_field', v_en);

"

append js "$javascript_forms" "$N"


#####################################################################
header "GPS" "GpsGate" "@TR<<GpsGate TrackerOne Connection>>" ' onload="modechange()" ' "$SCRIPT_NAME"
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

#####################################################################
else  
#####################################################################
header "GPS" "GpsGate" "@TR<<GpsGate Tracker Settings>>"  "$SCRIPT_NAME"
#####################################################################
cat <<EOF
Sorry, GPS service is disable.
<br>Please config GPS Settings firstly.
EOF

fi




footer ?>
<!--
##WEBIF:name:GPS:400:GpsGate
-->
