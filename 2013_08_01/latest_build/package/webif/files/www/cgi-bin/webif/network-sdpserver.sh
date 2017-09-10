#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

uci_load "sdpServer"

local cfg="server"

if empty "$FORM_submit"; then
       config_get FORM_Discovery_Service_Status $cfg Discovery_Service_Status
       config_get FORM_server_port $cfg server_port
else
       eval FORM_Discovery_Service_Status="\$FORM_${cfg}_Discovery_Service_Status"
       eval FORM_server_port="\$FORM_${cfg}_server_port"

   validate <<EOF
int|FORM_${cfg}_server_port|$cfg @TR<<Server port>>||$FORM_server_port
EOF
   equal "$?" 0 && {
         uci_set "sdpServer" "$cfg" "Discovery_Service_Status" "$FORM_Discovery_Service_Status"
         uci_set "sdpServer" "$cfg" "server_port" "$FORM_server_port"
	}
fi

sdpServer_options="start_form|@TR<<Server statuse Settings>>
field|@TR<<Discovery server status>>
radio|${cfg}_Discovery_Service_Status|$FORM_Discovery_Service_Status|A|@TR<<Disable>>
radio|${cfg}_Discovery_Service_Status|$FORM_Discovery_Service_Status|B|@TR<<Discovable>>
radio|${cfg}_Discovery_Service_Status|$FORM_Discovery_Service_Status|C|@TR<<Changable>>
end_form

start_form|@TR<<Server port Settings>>
field|@TR<<Server Port>>
text|${cfg}_server_port|$FORM_server_port
end_form"

append forms "$sdpServer_options" "$N"

header "Network" "sdpServer" "@TR<<sdpServer Settings>>" '' "$SCRIPT_NAME"

display_form <<EOF
$validate_error
$forms
EOF

footer

?>
<!--
##WEBIF:name:Network:800:sdpServer
-->
