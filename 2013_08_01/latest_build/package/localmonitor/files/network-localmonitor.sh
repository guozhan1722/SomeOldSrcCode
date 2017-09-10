#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"


uci_load localmonitor

conf="localmon_conf"

if empty "$FORM_submit"; then

    conf="localmon_conf"

else

 	FORM_LocalMon_Enable="$FORM_LocalMon_Enable_loc"
 	FORM_LocalMon_IP="$FORM_LocalMon_IP_loc"
 	FORM_LocalMon_AutoIP_En="$FORM_LocalMon_AutoIP_En_loc"
 	FORM_LocalMon_Timeout="$FORM_LocalMon_Timeout_loc"
 	FORM_LocalMon_WaitDHCP_Timeout="$FORM_LocalMon_WaitDHCP_Timeout_loc"

validate <<EOF
ip|$FORM_LocalMon_IP_loc|@TR<<Local Monitor IP>>||$FORM_LocalMon_IP
int|$FORM_LocalMon_Timeout_loc|@TR<<Device Connection Timeout>>||$FORM_LocalMon_Timeout
int|$FORM_LocalMon_WaitDHCP_Timeout_loc|@TR<<Wait DHCP Timeout>>||$FORM_LocalMon_WaitDHCP_Timeout
EOF

       [ "$?" = "0" ] && {

            uci_set "localmonitor" "$conf" "LocalMon_Enable" "$FORM_LocalMon_Enable"
            uci_set "localmonitor" "$conf" "LocalMon_IP" "$FORM_LocalMon_IP"
            uci_set "localmonitor" "$conf" "LocalMon_AutoIP_En" "$FORM_LocalMon_AutoIP_En"
            uci_set "localmonitor" "$conf" "LocalMon_Timeout" "$FORM_LocalMon_Timeout"
            uci_set "localmonitor" "$conf" "LocalMon_WaitDHCP_Timeout" "$FORM_LocalMon_WaitDHCP_Timeout"

         }
fi

uci_load "localmonitor"
       config_get FORM_LocalMon_Enable $conf LocalMon_Enable "A"
       config_get FORM_LocalMon_IP $conf LocalMon_IP "0.0.0.0"
       config_get FORM_LocalMon_AutoIP_En $conf LocalMon_AutoIP_En "A"
       config_get FORM_LocalMon_Timeout $conf LocalMon_Timeout "10"
       config_get FORM_LocalMon_WaitDHCP_Timeout $conf LocalMon_WaitDHCP_Timeout "60"

config_options="
start_form| @TR<<Monitor Settings>>
field|@TR<<&nbsp;&nbsp;Status>>|LocalMon_Enable_loc_field|1
select|LocalMon_Enable_loc|$FORM_LocalMon_Enable
option|A|@TR<<Disable>>
option|B|@TR<<Enable Local Device Monitor>>

field|@TR<<&nbsp;&nbsp;IP Mode>>|LocalMon_AutoIP_En_loc_field|1
select|LocalMon_AutoIP_En_loc|$FORM_LocalMon_AutoIP_En
option|A|@TR<<Fixed Local IP>>
option|B|@TR<<Auto Detected IP>>

field|@TR<< &nbsp;&nbsp;Local IP Setting>>|LocalMon_IP_loc_field|hidden
text|LocalMon_IP_loc|$FORM_LocalMon_IP|@TR<<[0.0.0.0]>>

field|@TR<< &nbsp;&nbsp;Status Timeout>>|LocalMon_Timeout_loc_field|hidden
text|LocalMon_Timeout_loc|$FORM_LocalMon_Timeout|@TR<<[5~65535](s)>>

field|@TR<< &nbsp;&nbsp;Waiting DHCP Timeout>>|LocalMon_WaitDHCP_Timeout_loc_field|hidden
text|LocalMon_WaitDHCP_Timeout_loc|$FORM_LocalMon_WaitDHCP_Timeout|@TR<<[30~65535](s)>>

end_form
"

append forms "$config_options" "$N"


######################################################################################
# set JavaScript
# set_visible('Pos_Record_NUMS_rec_field', 0);

javascript_forms="
    v_en = isset('LocalMon_Enable_loc','B');

    set_visible('LocalMon_AutoIP_En_loc_field', v_en);
    set_visible('LocalMon_Timeout_loc_field', v_en);
    set_visible('LocalMon_WaitDHCP_Timeout_loc_field', v_en);

    v_autoIP=isset('LocalMon_AutoIP_En_loc','A');
    if(v_autoIP&&v_en)v_locIP=1;
    else v_locIP=0;
    set_visible('LocalMon_IP_loc_field', v_locIP);

"

append js "$javascript_forms" "$N"


#####################################################################
header "Network" "LocalMonitor" "@TR<<Local Device Monitor>>" ' onload="modechange()" ' "$SCRIPT_NAME"
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
##WEBIF:name:Network:810:LocalMonitor
-->
