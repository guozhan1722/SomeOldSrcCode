#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"

uci_load updatedd

conf="ddns"

servers=""
ddns_services=$(updatedd -L |grep -v "Services:")
for ser in $ddns_services; do
    append servers "option|$ser" "$N"
done

if empty "$FORM_submit"; then
        config_get FORM_update $conf update 
        config_get FORM_service $conf service 
        config_get FORM_username $conf username 
        config_get FORM_password $conf password 
        config_get FORM_host $conf host 
else
        FORM_update="$FORM_update_ddns"
        FORM_service="$FORM_service_ddns"
        FORM_username="$FORM_username_ddns"
        FORM_password="$FORM_password_ddns"
        FORM_host="$FORM_host_ddns"

        uci_set "updatedd" "ddns" "update" "$FORM_update"
        uci_set "updatedd" "ddns" "service" "$FORM_service"
        uci_set "updatedd" "ddns" "username" "$FORM_username"
        uci_set "updatedd" "ddns" "password" "$FORM_password"
        uci_set "updatedd" "ddns" "host" "$FORM_host"
fi
   
   
ddns_options="start_form| @TR<<Configuration>>
field|@TR<<DDNS status>>
select|update_ddns|$FORM_update
option|1|@TR<<Enable>>
option|0|@TR<<Disable>>

field|@TR<<Service>>|service_ddns_field|hidden
select|service_ddns|$FORM_service
$servers

field|@TR<<User Name>>|username_ddns_field|hidden
text|username_ddns|$FORM_username

field|@TR<<Password>>|password_ddns_field|hidden
text|password_ddns|$FORM_password

field|@TR<<Host>>|host_ddns_field|hidden
text|host_ddns|$FORM_host

end_form"

append forms "$ddns_options" "$N"


###################################################################
# set JavaScript
javascript_forms="
    v = isset('update_ddns', '1')
    set_visible('service_ddns_field', v);
    set_visible('username_ddns_field', v);
    set_visible('password_ddns_field', v);
    set_visible('host_ddns_field', v);
"

append js "$javascript_forms" "$N"

header "Carrier" "Dynamic DNS" "@TR<<Dynamic_DNS Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"


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
##WEBIF:name:Carrier:500:Dynamic DNS
-->
