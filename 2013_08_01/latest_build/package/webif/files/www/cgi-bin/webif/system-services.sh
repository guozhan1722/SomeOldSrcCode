#!/usr/bin/webif-page
<%
. /usr/lib/webif/webif.sh
###################################################################
# Services
#
# Description:
#	Services control page.
#       This page enables the user to enable/disabled/start 
#		and stop the services in the directory /etc/init.d
#
# Author(s) [in order of work date]:
#       m4rc0 <jansssenmaj@gmail.com>
#
# Major revisions:
#       2008-11-08 - Initial release
#
# Configuration files referenced:
#       none
#
# Required components:
# 

header "System" "Services" "@TR<<Services>>" '' "$SCRIPT_NAME"


# change rowcolors
get_tr() {
	if equal "$cur_color" "odd"; then
		cur_color="even"
		echo "<tr>"
	else
		cur_color="odd"
		echo "<tr class=\"odd\">"
	fi
}

#check if a service with an action is selected
if [ "$FORM_service" != "" ] && [ "$FORM_action" != "" ]; then
	/etc/init.d/$FORM_service $FORM_action > /dev/null 2>&1
fi
#<table style="width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.9em;" border="0" cellpadding="2" cellspacing="1" summary="@TR<<Services>>">
#<h3><strong>@TR<<Available services>></strong></h3>
#<table style="width: 95%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">

cat <<EOF
<div class="settings">
<h3><strong>@TR<<Available Services>></strong></h3>
<table style="width: 100%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="@TR<<Services>>">
<tr>
<td>

<table style="margin-left: 2.5em; text-align: left;" border="0" cellpadding="2" cellspacing="1" summary="@TR<<Services>>">
EOF

# set the color-switch
cur_color="odd"

usr_service="
lte_rssi
iperf
dropbear
telnet
"

get_service_name()
{
    ser="$1"
    if [ "$ser" = "lte_rssi" ]; then
       service_name="RSSI LED" 
    elif [ "$ser" = "dropbear" ]; then
       service_name="SSH Service" 
    elif [ "$ser" = "iperf" ]; then
       service_name="Throughput Test Server" 
    elif [ "$ser" = "telnet" ]; then
       service_name="Telnet Service" 
    else
       service_name=$ser 
    fi

}
#for each service in init.d.....
#for service in $(ls /etc/init.d | grep -v "rcS") ; do
for service in $usr_service ; do
	# select the right color
	get_tr

        get_service_name $service

	#check if current $service is in the rc.d list
	if [ -x /etc/rc.d/S??${service} -o -x /etc/rc.d/K??${service} ] ; then
		echo "<td><img width=\"17\" src=\"/images/service_enabled.png\" alt=\"Service Enabled\" /></td>"
	else
		echo "<td><img width=\"17\" src=\"/images/service_disabled.png\" alt=\"Service Disabled\" /></td>"
	fi
	cat <<EOF
<td>&nbsp;</td>
<td>${service_name}</td>
<td><img height="1" width="5" src="/images/pixel.gif" alt="" /></td>
<td><a href="system-services.sh?service=${service}&amp;action=enable"><img width="13" src="/images/service_enable.png" alt="Enable Service" /></a></td>
<td valign="middle"><a href="system-services.sh?service=${service}&amp;action=enable">@TR<<system_services_service_enable#Auto Start Enable>></a></td>
<td><img height="1" width="5" src="/images/pixel.gif" alt="" /></td>
<td><a href="system-services.sh?service=${service}&amp;action=disable"><img width="13" src="/images/service_disable.png" alt="Disable Service" /></a></td>
<td valign="middle"><a href="system-services.sh?service=${service}&amp;action=disable">@TR<<system_services_service_disable#Auto Start Disable>></a></td>
<td><img height="1" width="5" src="/images/pixel.gif" alt="" /></td>
<td><a href="system-services.sh?service=${service}&amp;action=start"><img width="13" src="/images/service_start.png" alt="Start Service" /></a></td>
<td valign="middle"><a href="system-services.sh?service=${service}&amp;action=start">@TR<<system_services_sevice_start#Start>></a></td>
<td><img height="1" width="5" src="/images/pixel.gif" alt="" /></td>
<td><a href="system-services.sh?service=${service}&amp;action=restart"><img width="13" src="/images/service_restart.png" alt="Restart Service" /></a></td>
<td valign="middle"><a href="system-services.sh?service=${service}&amp;action=restart">@TR<<system_services_service_restart#Restart>></a></td>
<td><img height="1" width="5" src="/images/pixel.gif" alt="" /></td>
<td><a href="system-services.sh?service=${service}&amp;action=stop"><img width="13" src="/images/service_stop.png" alt="Stop Service" /></a></td>
<td valign="middle"><a href="system-services.sh?service=${service}&amp;action=stop">@TR<<system_services_service_stop#Stop>></a></td>
</tr>
EOF
done

	cat <<EOF
</table>
</td>

<td valign="top">
<table style="margin-left: 2.5em; text-align: left;" border="0" cellpadding="2" cellspacing="1" summary="@TR<<Services>>">
<tr>
<td><img width="17" src="/images/service_enabled.png" alt="Service Enabled" /></td>
<td>@TR<<system_services_service_enabled#Service Auto Start Enabled>></td>
</tr>
<tr>
<td><img width="17" src="/images/service_disabled.png" alt="Service Disabled" /></td>
<td>@TR<<system_services_service_disabled#Service Auto Start Disabled>></td>
</tr>

<tr><td colspan="2">&nbsp;</td></tr>
EOF

#if there is a service and an action selected... display status
if [ "$FORM_service" != "" ] && [ "$FORM_action" != "" ]; then
	
	case "$FORM_action" in
		enable)		status="enabled";;
		disable)	status="disabled";;
		start)		status="started";;
		restart)	status="restarted";;
		stop)		status="stopped";;
	esac

fi


cat <<EOF
</table>
</td>
</tr>
</table>
</div>
EOF

cat <<EOF
<div class="settings">
<table style="width: 100%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3><strong>Services Status</strong></h3></th></tr>
</table>
<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
EOF

cur_color="odd"

for service in $usr_service ; do
	# select the right color
	get_tr

        get_service_name $service
	#check if current $service is in the rc.d list

cat <<EOF
    <td>${service_name}</td>
EOF
	if [ -x /etc/rc.d/S??${service} -o -x /etc/rc.d/K??${service} ] ; then
		echo "<td><img width=\"17\" src=\"/images/service_enabled.png\" alt=\"Service Enabled\" /></td>"
		echo "<td valign=\"middle\">Service Auto Start Enabled</td>"
	else
		echo "<td><img width=\"17\" src=\"/images/service_disabled.png\" alt=\"Service Disabled\" /></td>"
        	echo "<td valign=\"middle\">Service Auto Start Disabled</td>"
#		echo "<td><img width=\"17\" src=\"/images/service_disabled.png\" alt=\"Service Disabled\" /></td>"
	fi

        service_s=$(ps |grep $service |grep -v grep)
        if [ -z $service_s ] ;then
            echo "<td><img width=\"13\" src=\"/images/service_stop.png\" alt=\"Stop Service\" /></td>"
            echo "<td valign=\"middle\">@TR<<system_services_service_stop#Stopped>></td>"
        else
            echo "<td><img width=\"13\" src=\"/images/service_start.png\" alt=\"Start Service\" /></td>"
            echo "<td valign=\"middle\">@TR<<system_services_service_Started#Started>></td>"
        fi
cat <<EOF
</tr>
EOF
done
cat <<EOF
</table>
</div>
EOF

footer_nosubmit %>
<!--
##WEBIF:name:System:200:Services
-->
