#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

[ -n "$FORM_interval" ] || FORM_interval=20

! empty "$FORM_refreshstop" && {
	FORM_interval=0
}

[ "$FORM_interval" -gt 0 ] >/dev/null 2>&1 && {
	meta_refresh="<meta http-equiv=\"refresh\" content=\"$FORM_interval;url=$SCRIPT_NAME?interval=$FORM_interval\" />"
}

header_inject_head=$(cat <<EOF
$meta_refresh
EOF

)

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	#case "$cfg_type" in
	#	general)
	#		append general_cfgs "$cfg_name" "$N"
	#	;;
	#esac
}

CONF_FILE=0
wancount=2
CACHE_FILE="/tmp/.mwan/cache"
query_config() {
    if [ ! -d /tmp/.mwan ]; then
        CONF_FILE=0
    else 
        CONF_FILE=1
    fi

    wan_if_map=$(grep wan_if_map $CACHE_FILE)
    wan_ip_map=$(grep wan_ip_map $CACHE_FILE)
    wan_gw_map=$(grep wan_gw_map $CACHE_FILE)
    wan_id_map=$(grep wan_id_map $CACHE_FILE)
    wan_fail_map=$(grep wan_fail_map $CACHE_FILE)
    wan_recovery_map=$(grep wan_recovery_map $CACHE_FILE)
    wan_monitor_map=$(grep wan_monitor_map $CACHE_FILE)

    case $1 in
	ifname) echo $wan_if_map | grep -o "$2\[\w*.*\]" | awk -F "[" '{print $2}' | awk -F "]" '{print $1}';; 
	ipaddr) echo $wan_ip_map | grep -o "$2\[\w*.*\]" | awk -F "[" '{print $2}' | awk -F "]" '{print $1}';;
	gateway) echo $wan_gw_map | grep -o "$2\[\w*.*\]" | awk -F "[" '{print $2}' | awk -F "]" '{print $1}';;
	wanid) echo $wan_id_map | grep -o "$2\[\w*.*\]" | awk -F "[" '{print $2}' | awk -F "]" '{print $1}';;
	failchk) echo $wan_fail_map | grep -o "$2\[\w*.*\]" | awk -F "[" '{print $2}' | awk -F "]" '{print $1}';;
	recvrychk) echo $wan_recovery_map | grep -o "$2\[\w*.*\]" | awk -F "[" '{print $2}' | awk -F "]" '{print $1}';;
	monitor) echo $wan_monitor_map | grep -o "$2\[\w*.*\]" | awk -F "[" '{print $2}' | awk -F "]" '{print $1}';;
	group) echo $wan_id_map | grep -o "\w*\[$2\]" | awk -F "[" '{print $1}';;
    esac
}

uci_load multiwan
conf="config"

header "MultiWAN" "Status" "@TR<<Multi WAN Status>>" '' "$SCRIPT_NAME"

config_get FORM_enabled $conf enabled 

[ "$FORM_enabled" != "1" ] && { 
        append status_info "string|<tr><td colspan=\"2\"><h3 interface=\"warning\">Multi WAN status is disabled </h3></td></tr>" "$N"
} || {

cat <<EOF
<div class="settings">
EOF

while [ $((i++)) -lt $wancount ]; do 
	group=$(query_config group $i)
	wanid=$(query_config wanid $group)
	ifname=$(query_config ifname $group)
	ipaddr=$(query_config ipaddr $group)
	gateway=$(query_config gateway $group)
	failchk=$(query_config failchk $group)
        recvrychk=$(query_config recvrychk $group)
        #monitorchk=$(query_config monitor $group)
        mdns=$(uci_get_state network $group dns)

        if [ "$group" = "wan" ]; then
            wanname="WAN"
        elif [ "$group" = "wan2" ]; then
            wanname="4G"
        fi

        if [ -z "$failchk" ];then
            failchk="---"
        fi

        if [ -z "$recvrychk" ];then
            recvrychk="---"
        fi

        if [ -z "$mdns" ];then
            mdns="N/A"
        fi

cat <<EOF
<table style="width: 100%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3>
<strong>Multi WAN GROUP $i</strong></h3></th>
</tr>
</table>

<table style="width: 100%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width=40%><strong>WAN Name</strong></td>
	<td width=40%>$wanname</td>
</tr>

<tr>
	<td width=40%><strong>IP Address</strong></td>
	<td width=40%>$ipaddr</td>
</tr>

<tr>
	<td width=40%><strong>Gateway</strong></td>
	<td width=40%>$gateway</td>
</tr>

<tr>
	<td width=40%><strong>DNS</strong></td>
	<td width=40%>$mdns</td>
</tr>

<tr>
	<td width=40%><strong>Failed WAN </strong></td>
	<td width=40%>$failchk</td>
</tr>

<tr>
	<td width=40%><strong>Recovery WAN </strong></td>
	<td width=40%>$recvrychk</td>
</tr>

</table>
EOF
done


cat <<EOF
</div>
EOF

}

display_form <<EOF
$status_info
EOF

echo "<div class=\"settings\" style=\"float: right;\">"
echo "<form name=\"refresh\" method=\"post\" action=\"$SCRIPT_NAME\">"

[ "$FORM_interval" -gt 0 ] >/dev/null 2>&1 && {
	cat <<EOF
<input type="submit" value=" @TR<<status_comports_Stop_Refreshing#Stop Refreshing>> " name="refreshstop" />
@TR<<status_comports_Interval#Interval>>: $FORM_interval (@TR<<status_comports_in_seconds#in seconds>>)
EOF
} || {
	cat <<EOF
<input type="submit" value=" @TR<<status_comports_Auto_Refresh#Auto Refresh>> " name="refreshstart" />
@TR<<status_comports_Interval#Interval>>:

<select name="interval">
EOF
	for sec in $(seq 3 59); do
		[ "$sec" -eq 20 ] && {
			echo "<option value=\"$sec\" selected=\"selected\">$sec</option>"
		} || {
			echo "<option value=\"$sec\">$sec</option>"
		}
	done
	cat <<EOF
</select>
@TR<<status_comports_in_seconds#in seconds>>
EOF
}
echo "</form></div>"

footer_nosubmit ?>
<!--
##WEBIF:name:MultiWAN:100:Status
-->
