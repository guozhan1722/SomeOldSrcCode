#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"

uci_load keepalive
conf="vip4g"

if empty "$FORM_submit"; then
       config_get FORM_enable $conf enable 
       config_get FORM_type $conf type 
       config_get FORM_hostname $conf hostname 
       config_get FORM_port $conf port 
       config_get FORM_interval $conf interval 
       config_get FORM_count $conf count 
else
       FORM_enable="$FORM_enable_keepalive"
       FORM_type="$FORM_type_keepalive"
       FORM_hostname="$FORM_hostname_keepalive" 
       FORM_port="$FORM_port_keepalive" 
       FORM_interval="$FORM_interval_keepalive" 
       FORM_count="$FORM_count_keepalive" 
validate <<EOF
ports|FORM_port_keepalive|@TR<<Port>>||$FORM_port
int|FORM_interval_keepalive|@TR<<Interval>>|min=60 max=60000|$FORM_interval
int|FORM_count_keepalive|@TR<<Count>>||$FORM_count
EOF
       [ "$?" = "0" ] && {
            uci_set "keepalive" "$conf" "enable" "$FORM_enable"
            uci_set "keepalive" "$conf" "type" "$FORM_type"
            uci_set "keepalive" "$conf" "hostname" "$FORM_hostname"
            uci_set "keepalive" "$conf" "port" "$FORM_port"
            uci_set "keepalive" "$conf" "interval" "$FORM_interval"
            uci_set "keepalive" "$conf" "count" "$FORM_count"
         }
fi
   
connect_options="start_form| @TR<<Configuration>>
field|@TR<<Keep alive status>>
select|enable_keepalive|$FORM_enable
option|1|@TR<<Enable>>
option|0|@TR<<Disable>>

field|@TR<<Type>>|type_keepalive_field|hidden
select|type_keepalive|$FORM_type
option|ICMP|@TR<<ICMP>>
option|HTTP|@TR<<HTTP>>

field|@TR<<Host Name>>|hostname_keepalive_field|hidden
text|hostname_keepalive|$FORM_hostname

field|@TR<<Port>>|port_keepalive_field|hidden
text|port_keepalive|$FORM_port

field|@TR<<Interval (60 ~ 60000)>>|interval_keepalive_field|hidden
text|interval_keepalive|$FORM_interval
string|(s)

field|@TR<<Count>>|count_keepalive_field|hidden
text|count_keepalive|$FORM_count

end_form"

append forms "$connect_options" "$N"

###################################################################
# set JavaScript
javascript_forms="
    v = isset('enable_keepalive', '1')
    set_visible('type_keepalive_field', v);
    set_visible('hostname_keepalive_field', v);
    set_visible('port_keepalive_field', v);
    set_visible('interval_keepalive_field', v);
    set_visible('count_keepalive_field', v);
    v = isset('type_keepalive', 'HTTP')
    set_visible('port_keepalive_field', v);
"

append js "$javascript_forms" "$N"

header "Carrier" "Keepalive" "@TR<<Keepalive Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"


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
##WEBIF:name:Carrier:300:Keepalive
-->
