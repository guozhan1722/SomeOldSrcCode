#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
. /www/cgi-bin/webif/graphs-subcategories.sh


uci_load bandwidthd
conf="connect"

if empty "$FORM_submit"; then
       config_get FORM_disabled bandwidthd bandwidthd_disable 
       #logger "bandwidthd server: $FORM_disabled"
else
       FORM_disabled="$FORM_disabled_bandwidthd"
       #logger "bandwidthd server: $FORM_disabled"
       uci_set "bandwidthd" "bandwidthd" "bandwidthd_disable" "$FORM_disabled"
fi


header "Graphs" "graphs_bandwidth_subcategory#Bandwidth" "@TR<<Bandwidth>>" 'onload="modechange()"' "$SCRIPT_NAME"

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
start_form| @TR<<Configuration>>
field|@TR<<Bandwidthd status>>
select|disabled_bandwidthd|$FORM_disabled
option|0|@TR<<Enable>>
option|1|@TR<<Disable>>
end_form  
EOF


if [ "$FORM_install_bandwidthd" != "" ]; then
	echo "Installing $service package ...<pre>"
	install_package bandwidthd
	/etc/init.d/bandwidthd enable
	/etc/init.d/bandwidthd start
	echo "</pre>"
fi
is_package_installed bandwidthd
[ "$?" = "0" ] && bandwidthd_installed=1
[ "$FORM_timeline" = "" ] && FORM_timeline=1

display_menu() {
display_form <<EOF
formtag_begin|filterform|$SCRIPT_NAME
start_form
field|@TR<<Select Display Mode>>
select|timeline|$FORM_timeline
option|5|@TR<<Last 5-Minutes>>
option|6|@TR<<Hourly>>
option|1|@TR<<Daily>>
option|2|@TR<<Weekly>>
option|3|@TR<<Monthly>>
option|4|@TR<<Yearly>>
string|</td></tr><tr><td>
submit|timelinesubmit|@TR<<Select>>
end_form
EOF
}
if [ "$FORM_timeline" = "1" ]; then
	cat /www/bandwidthd/index.html |grep -q "bandwidthd has nothing to graph."
	[ "$?" = "0" ] && graphs=0
else
	cat /www/bandwidthd/index${FORM_timeline}.html |grep -q "bandwidthd has nothing to graph."
	[ "$?" = "0" ] && graphs=0
fi
if [ "$bandwidthd_installed" != "1" ]; then
display_form <<EOF
string|<div class=\"warning\">@TR<<status_bandwidth_feature_requires_bandwidthd#Bandwidthd is not installed>>:</div>
submit|install_bandwidthd|@TR<<system_settings_Install_Bandwidthd#Install Bandwidthd>>
EOF
elif [ "$FORM_disabled" = "0" -a "$graphs" != "0" ]; then
	display_menu
	echo "<center><table width="100%" border=1 cellspacing=0>"
	if [ "$FORM_timeline" = "1" ]; then
		cat /www/bandwidthd/index.html |grep "TR"
	else
		cat /www/bandwidthd/index${FORM_timeline}.html |grep "TR"
	fi
	echo "</table></center><br/>"
	ips="`ls /www/bandwidthd/ |cut -d"-" -f1|egrep '([[:digit:]]{1,3}\.){3}[[:digit:]]{1,3}'|uniq`"

	cat <<EOF
<a name="Total-${FORM_timeline}">
<H2>Total</H2><BR>
Send:<br>
<img src=/bandwidthd/Total-${FORM_timeline}-S.png ALT="Sent traffic for Total"><BR>
<img src=/bandwidthd/legend.gif ALT="Legend"><br>
Receive:<br>
<img src=/bandwidthd/Total-${FORM_timeline}-R.png ALT="Sent traffic for Total"><BR>
<img src=/bandwidthd/legend.gif ALT="Legend"><br>
<BR>
EOF

	for ip in $ips; do
		if [ -e /tmp/bandwidthd/${ip}-${FORM_timeline}-R.png ]; then
		cat <<EOF
<a name="${ip}-${FORM_timeline}">
<a name="/bandwidthd/${ip}-${FORM_timeline}"></a><H2> ${ip}</H2><BR>
Send:<br>
<img src=/bandwidthd/${ip}-${FORM_timeline}-S.png ALT="Sent traffic for ${ip}"><BR>
<img src=/bandwidthd/legend.gif ALT="Legend"><br>
Receive:<br>
<img src=/bandwidthd/${ip}-${FORM_timeline}-R.png ALT="Sent traffic for ${ip}"><BR>
<img src=/bandwidthd/legend.gif ALT="Legend"><br>
<BR>
EOF
		fi
	done
elif [ "$FORM_disabled" = "0" -a "$graphs" = "0" ]; then
	display_menu
	echo "@TR<<Bandwidth chart has not yet been generated, please try again later.>>"
fi
footer ?>
<!--
##WEBIF:name:Graphs:160:graphs_bandwidth_subcategory#Bandwidth
-->
