#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"


config_cb() {
    local cfg_type="$1"
    local cfg_name="$2"

    case "$cfg_type" in
        s2stunnel)
            append s2s_cfgs "$cfg_name" "$N"
        ;;
        x2ctunnel)
            append x2c_cfgs "$cfg_name" "$N"
        ;;
        vca)
            append vca_cfgs "$cfg_name" "$N"
        ;;
    esac
}


if empty "$FORM_submit"; then
    uci_load adxl
    config_get FORM_ADXL_Remote_Reporting_Status    adxl_report_conf   ADXL_Remote_Reporting_Status
    config_get FORM_ADXL_Remote_Reporting_Mode      adxl_report_conf   ADXL_Remote_Reporting_Mode
    config_get FORM_ADXL_Remote_Reporting_Type      adxl_report_conf   ADXL_Remote_Reporting_Type
    config_get FORM_ADXL_Remote_Reporting_IP        adxl_report_conf   ADXL_Remote_Reporting_IP 
    config_get FORM_ADXL_Remote_Reporting_PORT      adxl_report_conf   ADXL_Remote_Reporting_PORT
    config_get FORM_ADXL_Remote_Reporting_Timer     adxl_report_conf   ADXL_Remote_Reporting_Timer
    config_get FORM_ADXL_Remote_Reporting_Message1     adxl_report_conf   ADXL_Remote_Reporting_Message1
    config_get FORM_ADXL_Remote_Reporting_Message2     adxl_report_conf   ADXL_Remote_Reporting_Message2
    config_get FORM_ADXL_Remote_Reporting_Message3     adxl_report_conf   ADXL_Remote_Reporting_Message3
    config_get FORM_ADXL_Remote_Reporting_Message4     adxl_report_conf   ADXL_Remote_Reporting_Message4


else

    FORM_ADXL_Remote_Reporting_IP="$FORM_ADXL_Remote_Reporting_IP"
    FORM_ADXL_Remote_Reporting_PORT="$FORM_ADXL_Remote_Reporting_PORT"
    FORM_ADXL_Remote_Reporting_Timer="$FORM_ADXL_Remote_Reporting_Timer"
    FORM_ADXL_Remote_Reporting_Status="$FORM_ADXL_Remote_Reporting_Status"
    FORM_ADXL_Remote_Reporting_Message1="$FORM_ADXL_Remote_Reporting_Message1"
    FORM_ADXL_Remote_Reporting_Message2="$FORM_ADXL_Remote_Reporting_Message2"
    FORM_ADXL_Remote_Reporting_Message3="$FORM_ADXL_Remote_Reporting_Message3"
    FORM_ADXL_Remote_Reporting_Message4="$FORM_ADXL_Remote_Reporting_Message4"


validate <<EOF
ip|FORM_ADXL_Remote_Reporting_IP|@TR<<Remote IP>>||$FORM_ADXL_Remote_Reporting_IP
int|FORM_ADXL_Remote_Reporting_PORT|@TR<<Remote Port>>||$FORM_ADXL_Remote_Reporting_PORT
int|FORM_ADXL_Remote_Reporting_Timer|@TR<<Intervals Timer>>||$FORM_ADXL_Remote_Reporting_Timer
EOF


       [ "$?" = "0" ] && {
            uci_set "adxl" "adxl_report_conf" "ADXL_Remote_Reporting_Status" "$FORM_ADXL_Remote_Reporting_Status"
            uci_set "adxl" "adxl_report_conf" "ADXL_Remote_Reporting_Mode" "$FORM_ADXL_Remote_Reporting_Mode"
            uci_set "adxl" "adxl_report_conf" "ADXL_Remote_Reporting_Type" "$FORM_ADXL_Remote_Reporting_Type"
            uci_set "adxl" "adxl_report_conf" "ADXL_Remote_Reporting_IP" "$FORM_ADXL_Remote_Reporting_IP"
            uci_set "adxl" "adxl_report_conf" "ADXL_Remote_Reporting_PORT" "$FORM_ADXL_Remote_Reporting_PORT"
            uci_set "adxl" "adxl_report_conf" "ADXL_Remote_Reporting_Timer" "$FORM_ADXL_Remote_Reporting_Timer"
            uci_set "adxl" "adxl_report_conf" "ADXL_Remote_Reporting_Message1" "$FORM_ADXL_Remote_Reporting_Message1"
            uci_set "adxl" "adxl_report_conf" "ADXL_Remote_Reporting_Message2" "$FORM_ADXL_Remote_Reporting_Message2"
            uci_set "adxl" "adxl_report_conf" "ADXL_Remote_Reporting_Message3" "$FORM_ADXL_Remote_Reporting_Message3"
            uci_set "adxl" "adxl_report_conf" "ADXL_Remote_Reporting_Message4" "$FORM_ADXL_Remote_Reporting_Message4"
       }
fi


##############################################################################################
# set Webpage

config_options="
start_form| @TR<<Report Configuration>>
field|@TR<< <strong>Accelerometer Report<strong> >>|ADXL_Remote_Reporting_Status_field|1
select|ADXL_Remote_Reporting_Status|$FORM_ADXL_Remote_Reporting_Status
option|0|@TR<<Disable>>
option|1|@TR<<Enable>>

field|@TR<<&nbsp;&nbsp;Report Trigger Mode>>|ADXL_Remote_Reporting_Mode_field|hidden
select|ADXL_Remote_Reporting_Mode|$FORM_ADXL_Remote_Reporting_Mode
option|A|@TR<<Event >>
option|B|@TR<<Timer >>
option|C|@TR<<Event OR Timer>>

field|@TR<<&nbsp;&nbsp;Interval Time(s)>>|ADXL_Remote_Reporting_Timer_field|hidden
text|ADXL_Remote_Reporting_Timer|$FORM_ADXL_Remote_Reporting_Timer|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Report Messsage >>|ADXL_Remote_Reporting_Message_field|hidden
checkbox|ADXL_Remote_Reporting_Message1|$FORM_ADXL_Remote_Reporting_Message1|1
string|All
checkbox|ADXL_Remote_Reporting_Message2|$FORM_ADXL_Remote_Reporting_Message2|1
string|Impact                                                                
checkbox|ADXL_Remote_Reporting_Message3|$FORM_ADXL_Remote_Reporting_Message3|1
string|Activity                                                              
checkbox|ADXL_Remote_Reporting_Message4|$FORM_ADXL_Remote_Reporting_Message4|1
string|Inactivity


field|@TR<<&nbsp;&nbsp;Report Format Type>>|ADXL_Remote_Reporting_Type_field|hidden
select|ADXL_Remote_Reporting_Type|$FORM_ADXL_Remote_Reporting_Type
option|A|@TR<<Text>>
option|B|@TR<<TAIP>>

field|@TR<<&nbsp;&nbsp;Remote IP>>|ADXL_Remote_Reporting_IP_field|hidden
text|ADXL_Remote_Reporting_IP|$FORM_ADXL_Remote_Reporting_IP|@TR<<0.0.0.0>>

field|@TR<<&nbsp;&nbsp;Remote PORT>>|ADXL_Remote_Reporting_PORT_field|hidden
text|ADXL_Remote_Reporting_PORT|$FORM_ADXL_Remote_Reporting_PORT|@TR<<[0 ~ 65535]>>


end_form"

append forms "$config_options" "$N"


######################################################################################
# set JavaScript
javascript_forms="
    v01 = isset('ADXL_Remote_Reporting_Status', '0');
    set_visible('ADXL_Remote_Reporting_Type_field', !v01);
    set_visible('ADXL_Remote_Reporting_Mode_field', !v01);
    set_visible('ADXL_Remote_Reporting_IP_field', !v01);
    set_visible('ADXL_Remote_Reporting_PORT_field', !v01);
    set_visible('ADXL_Remote_Reporting_Timer_field', !v01);
    set_visible('ADXL_Remote_Reporting_Message_field', !v01);
    set_visible('ADXL_Remote_Reporting_Message1_field', !v01);
    set_visible('ADXL_Remote_Reporting_Message2_field', !v01);
    set_visible('ADXL_Remote_Reporting_Message3_field', !v01);
    set_visible('ADXL_Remote_Reporting_Message4_field', !v01);
"


append js "$javascript_forms" "$N"


##################################################################################################################
header "I\/O" "Accelerometer" "@TR<<Accelerometer Report>>" ' onload="modechange()" ' "$SCRIPT_NAME"
##################################################################################################################

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


footer ?>
<!--

##WEBIF:name:I/O:400:Accelerometer
-->
