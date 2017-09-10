#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"
###################################################################
header "Wireless" "NanoIPRadio" "@TR<<NanoIP Radio Configuration>>" 'onload="modechange()"' "$SCRIPT_NAME"

if [ -n "$FORM_config_radio" ]; then
cat <<EOF
        <iframe src="http://10.0.0.6/cgi-bin/radio_config.cgi" frameborder=0 width="90%" height="800">
EOF
	unset FORM_submit
fi

display_form <<EOF
submit|config_radio|@TR<<Config>> 
EOF

footer ?>
#<!--
###WEBIF:name:Wireless:400:NanoIPRadio
#-->




