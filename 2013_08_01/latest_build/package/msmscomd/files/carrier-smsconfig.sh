#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"

#uci_load msmscomd
#uci_load salertd


if empty "$FORM_submit"; then

    conf="msms_conf"

else

 	FORM_System_SMS_Commad="$FORM_System_SMS_Commad_msms"
 	FORM_SMS_CMD_Filter_Set="$FORM_SMS_CMD_Filter_Set_msms"
 	FORM_SMS_CMD_Filter_Phone1="$FORM_SMS_CMD_Filter_Phone1_msms"
 	FORM_SMS_CMD_Filter_Phone2="$FORM_SMS_CMD_Filter_Phone2_msms"
 	FORM_SMS_CMD_Filter_Phone3="$FORM_SMS_CMD_Filter_Phone3_msms"
 	FORM_SMS_CMD_Filter_Phone4="$FORM_SMS_CMD_Filter_Phone4_msms"
 	FORM_SMS_CMD_Filter_Phone5="$FORM_SMS_CMD_Filter_Phone5_msms"
 	FORM_SMS_CMD_Filter_Phone6="$FORM_SMS_CMD_Filter_Phone6_msms"
 	FORM_SAL_Enable="$FORM_SAL_Enable_msms"
 	FORM_SAL_Interval="$FORM_SAL_Interval_msms"
 	FORM_SAL_Phone1="$FORM_SAL_Phone1_msms"
 	FORM_SAL_Phone2="$FORM_SAL_Phone2_msms"
 	FORM_SAL_Phone3="$FORM_SAL_Phone3_msms"
 	FORM_SAL_Phone4="$FORM_SAL_Phone4_msms"
 	FORM_SAL_Phone5="$FORM_SAL_Phone5_msms"
 	FORM_SAL_Phone6="$FORM_SAL_Phone6_msms"
 	FORM_RSSI_CHECK="$FORM_RSSI_CHECK_msms"
 	FORM_RSSI_LOW="$FORM_RSSI_LOW_msms"
# 	FORM_CORE_TEMPERATURE_CHECK="$FORM_CORE_TEMPERATURE_CHECK_msms"
# 	FORM_CTEMP_HIGH="$FORM_CTEMP_HIGH_msms"
# 	FORM_CTEMP_LOW="$FORM_CTEMP_LOW_msms"
 	FORM_ROAMING_CHECK="$FORM_ROAMING_CHECK_msms"
 	FORM_ROAMING_STATUS="$FORM_ROAMING_STATUS_msms"
 	FORM_IO_CHECK="$FORM_IO_CHECK_msms"
 	FORM_ETH_CHECK="$FORM_ETH_CHECK_msms"
 	FORM_ETH_LINK_STATUS="$FORM_ETH_LINK_STATUS_msms"

validate <<EOF
int|FORM_SAL_Interval_msms|@TR<<Alert Interval>>||$FORM_SAL_Interval
EOF

       [ "$?" = "0" ] && {

            uci_set "msmscomd" "msms_conf" "System_SMS_Commad" "$FORM_System_SMS_Commad"
            uci_set "msmscomd" "msms_conf" "SMS_CMD_Filter_Set" "$FORM_SMS_CMD_Filter_Set"
            uci_set "msmscomd" "msms_conf" "SMS_CMD_Filter_Phone1" "$FORM_SMS_CMD_Filter_Phone1"
            uci_set "msmscomd" "msms_conf" "SMS_CMD_Filter_Phone2" "$FORM_SMS_CMD_Filter_Phone2"
            uci_set "msmscomd" "msms_conf" "SMS_CMD_Filter_Phone3" "$FORM_SMS_CMD_Filter_Phone3"
            uci_set "msmscomd" "msms_conf" "SMS_CMD_Filter_Phone4" "$FORM_SMS_CMD_Filter_Phone4"
            uci_set "msmscomd" "msms_conf" "SMS_CMD_Filter_Phone5" "$FORM_SMS_CMD_Filter_Phone5"
            uci_set "msmscomd" "msms_conf" "SMS_CMD_Filter_Phone6" "$FORM_SMS_CMD_Filter_Phone6"
            uci_set "salertd" "salert_conf" "SAL_Enable" "$FORM_SAL_Enable"
            uci_set "salertd" "salert_conf" "SAL_Interval" "$FORM_SAL_Interval"
            uci_set "salertd" "salert_conf" "SAL_Phone1" "$FORM_SAL_Phone1"
            uci_set "salertd" "salert_conf" "SAL_Phone2" "$FORM_SAL_Phone2"
            uci_set "salertd" "salert_conf" "SAL_Phone3" "$FORM_SAL_Phone3"
            uci_set "salertd" "salert_conf" "SAL_Phone4" "$FORM_SAL_Phone4"
            uci_set "salertd" "salert_conf" "SAL_Phone5" "$FORM_SAL_Phone5"
            uci_set "salertd" "salert_conf" "SAL_Phone6" "$FORM_SAL_Phone6"
            uci_set "salertd" "salert_conf" "RSSI_CHECK" "$FORM_RSSI_CHECK"
            uci_set "salertd" "salert_conf" "RSSI_LOW" "$FORM_RSSI_LOW"
#            uci_set "salertd" "salert_conf" "CORE_TEMPERATURE_CHECK" "$FORM_CORE_TEMPERATURE_CHECK"
#            uci_set "salertd" "salert_conf" "CTEMP_HIGH" "$FORM_CTEMP_HIGH"
#            uci_set "salertd" "salert_conf" "CTEMP_LOW" "$FORM_CTEMP_LOW"
            uci_set "salertd" "salert_conf" "ROAMING_CHECK" "$FORM_ROAMING_CHECK"
            uci_set "salertd" "salert_conf" "ROAMING_STATUS" "$FORM_ROAMING_STATUS"
            uci_set "salertd" "salert_conf" "ETH_CHECK" "$FORM_ETH_CHECK"
            uci_set "salertd" "salert_conf" "ETH_LINK_STATUS" "$FORM_ETH_LINK_STATUS"
            uci_set "salertd" "salert_conf" "IO_CHECK" "$FORM_IO_CHECK"

         }
fi


uci_load "msmscomd"
       config_get FORM_System_SMS_Commad "msms_conf" System_SMS_Commad
       config_get FORM_SMS_CMD_Filter_Set "msms_conf" SMS_CMD_Filter_Set
       config_get FORM_SMS_CMD_Filter_Phone1 "msms_conf" SMS_CMD_Filter_Phone1
       config_get FORM_SMS_CMD_Filter_Phone2 "msms_conf" SMS_CMD_Filter_Phone2
       config_get FORM_SMS_CMD_Filter_Phone3 "msms_conf" SMS_CMD_Filter_Phone3
       config_get FORM_SMS_CMD_Filter_Phone4 "msms_conf" SMS_CMD_Filter_Phone4
       config_get FORM_SMS_CMD_Filter_Phone5 "msms_conf" SMS_CMD_Filter_Phone5
       config_get FORM_SMS_CMD_Filter_Phone6 "msms_conf" SMS_CMD_Filter_Phone6
       
uci_load "salertd"
       config_get FORM_SAL_Enable "salert_conf" SAL_Enable
       config_get FORM_SAL_Interval "salert_conf" SAL_Interval
       config_get FORM_SAL_Phone1 "salert_conf" SAL_Phone1
       config_get FORM_SAL_Phone2 "salert_conf" SAL_Phone2
       config_get FORM_SAL_Phone3 "salert_conf" SAL_Phone3
       config_get FORM_SAL_Phone4 "salert_conf" SAL_Phone4
       config_get FORM_SAL_Phone5 "salert_conf" SAL_Phone5
       config_get FORM_SAL_Phone6 "salert_conf" SAL_Phone6
       config_get FORM_RSSI_CHECK "salert_conf" RSSI_CHECK
       config_get FORM_RSSI_LOW "salert_conf" RSSI_LOW
#       config_get FORM_CORE_TEMPERATURE_CHECK "salert_conf" CORE_TEMPERATURE_CHECK
#       config_get FORM_CTEMP_HIGH "salert_conf" CTEMP_HIGH
#       config_get FORM_CTEMP_LOW "salert_conf" CTEMP_LOW
       config_get FORM_ROAMING_CHECK "salert_conf" ROAMING_CHECK
       config_get FORM_ROAMING_STATUS "salert_conf" ROAMING_STATUS
       config_get FORM_IO_CHECK "salert_conf" IO_CHECK
       config_get FORM_ETH_CHECK "salert_conf" ETH_CHECK
       config_get FORM_ETH_LINK_STATUS "salert_conf" ETH_LINK_STATUS

uci_load "comport2"
     config_get FORM_COM2_IP_Protocol "com2" COM2_IP_Protocol
     config_get FORM_COM2_Port_Status "com2" COM2_Port_Status
     str_config_cmd="Enable SMS Command"
    if [ "$FORM_COM2_Port_Status" = "B" ]; then
        if [ "$FORM_COM2_IP_Protocol" = "J" ]; then
            str_config_cmd="Enable when COM-SMS Mode Disabled"
        fi
        if [ "$FORM_COM2_IP_Protocol" = "K" ]; then
            str_config_cmd="Enable when SMS AT Mode Disabled"
        fi
    fi

#field|@TR<<&nbsp;&nbsp;AAA>>|_gpsr_field|hidden
#text|_gpsr|$FORM_|@TR<<>>
#field|@TR<<&nbsp;&nbsp;AAA>>|_gpsr_field|hidden
#select|_gpsr|$FORM_
#option|A|@TR<<Disable>>
#option|D|@TR<<GpsGate Protocol UDP Report>>

#field|@TR<<Carrier's Temperature Check>>|TEMP_CHECK_msms_field|hidden


config_options="
start_form| @TR<<System SMS Command:>>
field|@TR<< <strong>Status<strong> >>|System_SMS_Commad_msms_field|1
select|System_SMS_Commad_msms|$FORM_System_SMS_Commad
option|A|@TR<<Disable SMS Command>>
option|B|@TR<<$str_config_cmd>>

field|@TR<<Set Phone Filter>>|SMS_CMD_Filter_Set_msms_field|hidden
select|SMS_CMD_Filter_Set_msms|$FORM_SMS_CMD_Filter_Set
option|A|@TR<<Disable Phone Filter>>
option|B|@TR<<Enable Phone Filter>>

field|@TR<<Valid Phone Numbers:>>|SMS_CMD_Filter_msms_field|hidden

field|@TR<<&nbsp;&nbsp;Phone No.1>>|SMS_CMD_Filter_Phone1_msms_field|hidden
text|SMS_CMD_Filter_Phone1_msms|$FORM_SMS_CMD_Filter_Phone1
field|@TR<<&nbsp;&nbsp;Phone No.2>>|SMS_CMD_Filter_Phone2_msms_field|hidden
text|SMS_CMD_Filter_Phone2_msms|$FORM_SMS_CMD_Filter_Phone2
field|@TR<<&nbsp;&nbsp;Phone No.3>>|SMS_CMD_Filter_Phone3_msms_field|hidden
text|SMS_CMD_Filter_Phone3_msms|$FORM_SMS_CMD_Filter_Phone3
field|@TR<<&nbsp;&nbsp;Phone No.4>>|SMS_CMD_Filter_Phone4_msms_field|hidden
text|SMS_CMD_Filter_Phone4_msms|$FORM_SMS_CMD_Filter_Phone4
field|@TR<<&nbsp;&nbsp;Phone No.5>>|SMS_CMD_Filter_Phone5_msms_field|hidden
text|SMS_CMD_Filter_Phone5_msms|$FORM_SMS_CMD_Filter_Phone5
field|@TR<<&nbsp;&nbsp;Phone No.6>>|SMS_CMD_Filter_Phone6_msms_field|hidden
text|SMS_CMD_Filter_Phone6_msms|$FORM_SMS_CMD_Filter_Phone6

end_form

start_form| @TR<<System SMS Alert:>>
field|@TR<< <strong>Status<strong> >>|SAL_Enable_msms_field|1
select|SAL_Enable_msms|$FORM_SAL_Enable
option|A|@TR<<Disable SMS Alert>>
option|B|@TR<<Enable SMS Alert>>


field|@TR<<Received Phone Numbers:>>|phone_msms_field|hidden

field|@TR<<&nbsp;&nbsp;Phone No.1>>|SAL_Phone1_msms_field|hidden
text|SAL_Phone1_msms|$FORM_SAL_Phone1
field|@TR<<&nbsp;&nbsp;Phone No.2>>|SAL_Phone2_msms_field|hidden
text|SAL_Phone2_msms|$FORM_SAL_Phone2
field|@TR<<&nbsp;&nbsp;Phone No.3>>|SAL_Phone3_msms_field|hidden
text|SAL_Phone3_msms|$FORM_SAL_Phone3
field|@TR<<&nbsp;&nbsp;Phone No.4>>|SAL_Phone4_msms_field|hidden
text|SAL_Phone4_msms|$FORM_SAL_Phone4
field|@TR<<&nbsp;&nbsp;Phone No.5>>|SAL_Phone5_msms_field|hidden
text|SAL_Phone5_msms|$FORM_SAL_Phone5
field|@TR<<&nbsp;&nbsp;Phone No.6>>|SAL_Phone6_msms_field|hidden
text|SAL_Phone6_msms|$FORM_SAL_Phone6

field|@TR<<Alert Condition Settings:>>|CHECK_msms_field|hidden

field|@TR<<Time Interval(s)>>|SAL_Interval_msms_field|hidden
text|SAL_Interval_msms|$FORM_SAL_Interval|@TR<<[5~65535]>>

field|@TR<<RSSI Check>>|RSSI_CHECK_msms_field|hidden
select|RSSI_CHECK_msms|$FORM_RSSI_CHECK
option|A|@TR<<Disable RSSI Check>>
option|B|@TR<<Enable RSSI Check>>
field|@TR<<&nbsp;&nbsp;Low Threshold(dBm):>>|RSSI_LOW_msms_field|hidden
text|RSSI_LOW_msms|$FORM_RSSI_LOW|@TR<<default: -99>>

field|@TR<<Carrier Network>>|ROAMING_CHECK_msms_field|hidden
select|ROAMING_CHECK_msms|$FORM_ROAMING_CHECK
option|A|@TR<<Disable Roaming Check>>
option|B|@TR<<Enable Roaming Check>>
field|@TR<<&nbsp;&nbsp;Home/Roaming Status:>>|ROAMING_STATUS_msms_field|hidden
select|ROAMING_STATUS_msms|$FORM_ROAMING_STATUS
option|B|@TR<<Changed>>
option|C|@TR<<In Roaming>>
option|D|@TR<<Changed or In Roaming>>
option|E|@TR<<Changed to Roaming>>

field|@TR<<Ethernet>>|ETH_CHECK_msms_field|hidden
select|ETH_CHECK_msms|$FORM_ETH_CHECK
option|A|@TR<<Disable Ethernet Check>>
option|B|@TR<<Enable Ethernet Check>>
field|@TR<<&nbsp;&nbsp;Link Status:>>|ETH_LINK_STATUS_msms_field|hidden
select|ETH_LINK_STATUS_msms|$FORM_ETH_LINK_STATUS
option|B|@TR<<Changed>>
option|C|@TR<<In No-link>>
option|D|@TR<<Changed or In No-link>>
option|E|@TR<<Changed To No-link>>

field|@TR<<IO Status>>|IO_CHECK_msms_field|hidden
select|IO_CHECK_msms|$FORM_IO_CHECK
option|A|@TR<<Disable IO Check>>
option|B|@TR<<Enable: INPUT Changed>>
option|C|@TR<<Enable: OUTPUT Changed>>
option|D|@TR<<Enable: INPUT or OUTPUT Changed>>

field|@TR<<&nbsp;&nbsp;>>|SALERT_LINK_field|hidden
string|@TR<< <a href=carrier-sms.sh?views=alertrecord>View Alert SMS Record</a> >>

end_form
"

append forms "$config_options" "$N"


######################################################################################
# set JavaScript
javascript_forms="
    v_sal = isset('SAL_Enable_msms', 'B');
    v_rssi = isset('RSSI_CHECK_msms', 'B');
    v_roaming = isset('ROAMING_CHECK_msms', 'B');
    v_eth = isset('ETH_CHECK_msms', 'B');
    v_syscmd = isset('System_SMS_Commad_msms', 'B');
    v_filter = isset('SMS_CMD_Filter_Set_msms', 'B');

    set_visible('SMS_CMD_Filter_Set_msms_field', v_syscmd);
    set_visible('SMS_CMD_list_msms_field', v_syscmd);
    
    v_fildis=0;
    if((v_syscmd)&&(v_filter))v_fildis=1;

    set_visible('SMS_CMD_Filter_msms_field', v_fildis);
    set_visible('SMS_CMD_Filter_Phone1_msms_field', v_fildis);
    set_visible('SMS_CMD_Filter_Phone2_msms_field', v_fildis);
    set_visible('SMS_CMD_Filter_Phone3_msms_field', v_fildis);
    set_visible('SMS_CMD_Filter_Phone4_msms_field', v_fildis);
    set_visible('SMS_CMD_Filter_Phone5_msms_field', v_fildis);
    set_visible('SMS_CMD_Filter_Phone6_msms_field', v_fildis);

    set_visible('phone_msms_field', v_sal);
    set_visible('CHECK_msms_field', v_sal);
    set_visible('SAL_Phone1_msms_field', v_sal);
    set_visible('SAL_Phone2_msms_field', v_sal);
    set_visible('SAL_Phone3_msms_field', v_sal);
    set_visible('SAL_Phone4_msms_field', v_sal);
    set_visible('SAL_Phone5_msms_field', v_sal);
    set_visible('SAL_Phone6_msms_field', v_sal);

    set_visible('SAL_Interval_msms_field', v_sal);
    set_visible('RSSI_CHECK_msms_field', v_sal);
    set_visible('ROAMING_CHECK_msms_field', v_sal);
    set_visible('ETH_CHECK_msms_field', v_sal);
    set_visible('SALERT_LINK_field', v_sal);

    set_visible('IO_CHECK_msms_field', v_sal);

    v_dis=0;
    if((v_sal)&&(v_rssi))v_dis=1;
    set_visible('RSSI_LOW_msms_field', v_dis);

    v_dis=0;
    if((v_sal)&&(v_roaming))v_dis=1;
    set_visible('ROAMING_STATUS_msms_field', v_dis);

    v_dis=0;
    if((v_sal)&&(v_eth))v_dis=1;
    set_visible('ETH_LINK_STATUS_msms_field', v_dis);
    
"

append js "$javascript_forms" "$N"


#####################################################################
header "Carrier" "SMS Config" "@TR<<SMS Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"
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
##WEBIF:name:Carrier:600:SMS Config
-->
