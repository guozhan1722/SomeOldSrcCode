#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

uci_load twatchdog
conf="vip4g"

if empty "$FORM_submit"; then
       config_get FORM_enable $conf enable 
       config_get FORM_reset_timer $conf reset_timer 
       config_get FORM_reboot_timer $conf reboot_timer 
       #config_get FORM_iface $conf iface 
else
       FORM_enable="$FORM_enable_twatchdog"
       FORM_reset_timer="$FORM_reset_timer_twatchdog"
       FORM_reboot_timer="$FORM_reboot_timer_twatchdog" 

       min_t=300
       if [ "$min_t" -lt "$FORM_reset_timer" ]; then
            min_t="$FORM_reset_timer"
       fi
validate <<EOF
int|FORM_reset_timer_twatchdog|@TR<<Check Interval>>|min=1 max=60000|$FORM_reset_timer
int|FORM_reboot_timer_twatchdog|@TR<<Reboot Time Limit>>|min=$min_t|$FORM_reboot_timer
EOF
       [ "$?" = "0" ] && {
            uci_set "twatchdog" "$conf" "enable" "$FORM_enable"
            uci_set "twatchdog" "$conf" "reset_timer" "$FORM_reset_timer"
            uci_set "twatchdog" "$conf" "reboot_timer" "$FORM_reboot_timer"
         }
fi
   
twatchdog_options="start_form| @TR<<Configuration>>
field|@TR<<Traffic Watchdog>>
select|enable_twatchdog|$FORM_enable
option|1|@TR<<Enable>>
option|0|@TR<<Disable>>

field|@TR<<Check Interval >>|reset_timer_twatchdog_field|hidden
text|reset_timer_twatchdog|$FORM_reset_timer
string|(1~60000s)

field|@TR<<Reboot Time Limit >>|reboot_timer_twatchdog_field|hidden
text|reboot_timer_twatchdog|$FORM_reboot_timer
string|(300~60000s)

end_form"

append forms "$twatchdog_options" "$N"

###################################################################
# set JavaScript
javascript_forms="
    v = isset('enable_twatchdog', '1')
    set_visible('reset_timer_twatchdog_field', v);
    set_visible('reboot_timer_twatchdog_field', v);
"

append js "$javascript_forms" "$N"

header "Carrier" "Traffic Watchdog" "@TR<<Traffic Watchdog Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"

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
##WEBIF:name:Carrier:400:Traffic Watchdog
-->
