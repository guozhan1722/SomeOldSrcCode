#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"

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

display_kernel_routes() {
	local ipv="$1"
	[ -z "$ipv" ] && return
	local route_cmd="route -n"
	local table_cols="8"
	[ "$ipv" = "6" ] && {
		route_cmd="$route_cmd -A inet6"
		table_cols="7"
                return
	}
	cat <<EOF
<div class="settings">
<h3><strong>@TR<<network_routes_IPv${ipv}_routing_table#IPv${ipv} Routing Table>></strong></h3>
<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tbody>
<tr>
	<th>@TR<<network_routes_static_th_Destination#Destination>></th>
EOF
	if [ "$ipv" = "4" ]; then
		cat <<EOF
	<th>@TR<<network_routes_static_th_Gateway#Gateway>></th>
	<th>@TR<<network_routes_static_th_Genmask#Netmask>></th>
EOF
	else
		cat <<EOF
	<th>@TR<<network_routes_static_th_Next_Hop#Next Hop>></th>
EOF
	fi
	cat <<EOF
	<th>@TR<<network_routes_static_th_Flags#Flags>></th>
	<th>@TR<<network_routes_static_th_Metric#Metric>></th>
	<th>@TR<<network_routes_static_th_Ref#Ref>></th>
	<th>@TR<<network_routes_static_th_Use#Use>></th>
	<th>@TR<<network_routes_static_th_Interface#Interface>></th>
</tr>
EOF
	$route_cmd 2>/dev/null | awk -v "ifaces=$ifaces" -v "cols=$table_cols" '
BEGIN {
	nic = split(ifaces, pairs)
	for (i = 1; i <= nic; i++) {
		npt = split(pairs[i], parts, ":")
		if (npt > 1) _ifaceifs[parts[2]] = parts[1]
	}
	odd = 1
	count = 0
}
($1 ~ /^[[:digit:]]/) || ($1 !~ /Kernel|Destination/) {
	if (odd == 1) {
		print "<tr>"
		odd--
	} else {
		print "<tr class=\"odd\">"
		odd++
	}
	for (i = 1; i <= NF; i++) {
		if (i == cols) {
			print "<td>" ((_ifaceifs[$(i)] == "") ? "@TR<<network_routes_unknown_iface# >>" : _ifaceifs[$(i)]) " (" $(i) ")</td>"
		} else printf "<td>" $(i) "</td>"
	}
	count++
	print "</tr>"
}
END {
	if (count == 0) print "<tr>\n" td_ind "<td colspan=\""cols"\">@TR<<network_routes_No_kernel_routes#There are no IP routes in the kernel'\''s table?!>></td>\n</tr>"
}
'
	cat <<EOF
</tbody>
</table>
<div class="clearfix">&nbsp;</div>
</div>
EOF
}


header "Network" "Status" "@TR<<Network Status>>"

###################################################################
# Parse Settings, this function is called when doing a config_load
config_cb() {
	cfg_type="$1"
	cfg_name="$2"
	case "$cfg_type" in
        interface)
                append network_devices "$cfg_name"
        ;;
esac
}

uci_load network
#logger "network_devices=$network_devices"
for cfgsec in $network_devices; do
      #logger "cfgsec=$cfgsec"
      [ "$cfgsec" = "loopback" -o "$cfgsec" = "wan2" ] || {
	 eval bridge_name="br-$cfgsec"
	 #logger "bridge $bridge_name"
	 config_get proto "$cfgsec" proto

	 br_mac="$(ifconfig "${bridge_name}" | grep HWaddr | awk -F'[ ]+' '{print $5}')"
	 br_ip="$(ifconfig "${bridge_name}" | grep inet |grep -v inet6 | awk -F'[: ]+' '{print $4}')"
         [ -z $br_ip ] && br_ip="N/A"

	 br_mask="$(ifconfig "${bridge_name}" | grep inet |grep -v inet6| awk -F'[: ]+' '{print $8}')"
         [ -z $br_mask ] && br_mask="N/A"
    
         rx_bytes=$(cat /proc/net/dev |grep "${bridge_name}" |awk -F ':' '{print $2}' | awk '{rx_b+=$1 } END {print rx_b}')
         rx_pkts=$(cat /proc/net/dev |grep "${bridge_name}" |awk -F ':' '{print $2}' | awk '{rx_p+=$2 } END {print rx_p}')

         tx_bytes=$(cat /proc/net/dev |grep "${bridge_name}" |awk -F ':' '{print $2}' | awk '{tx_b+=$9 } END {print tx_b}')
         tx_pkts=$(cat /proc/net/dev |grep "${bridge_name}"  |awk -F ':' '{print $2}' |awk '{tx_p+=$10 } END {print tx_p}')

        if [ "$rx_bytes" -lt "1000" ]; then
            rx_bytes=$(echo "$rx_bytes")B
        elif [ "$rx_bytes" -ge "1000" -a "$rx_bytes" -lt "1000000"  ]; then
            rx_bytes=$(echo "$rx_bytes" | awk '{printf("%s",$1/1000)}')KB
        elif [ "$rx_bytes" -ge "1000000" -a "$rx_bytes" -lt "1000000000" ];then
            rx_bytes=$(echo "$rx_bytes" |awk '{printf("%s",$1/1000000)}')MB
        else
            rx_bytes=$(echo "$rx_bytes" |awk '{printf("%s",$1/1000000000)}')GB
        fi

        if [ "$tx_bytes" -lt "1000" ]; then
            tx_bytes=$(echo "$tx_bytes")B
        elif [ "$tx_bytes" -ge "1000" -a "$tx_bytes" -lt "1000000"  ]; then
            tx_bytes=$(echo "$tx_bytes" | awk '{printf("%s",$1/1000)}')KB
        elif [ "$tx_bytes" -ge "1000000" -a "$tx_bytes" -lt "1000000000" ];then
            tx_bytes=$(echo "$tx_bytes" |awk '{printf("%s",$1/1000000)}')MB
        else
            tx_bytes=$(echo "$tx_bytes" |awk '{printf("%s",$1/1000000000)}')GB
        fi

    if [ "$cfgsec" = "wan" ];then
        cfgname="WAN"
    elif [ "$cfgsec" = "wan2" ];then
        cfgname="4G"
    elif [ "$cfgsec" = "lan" ];then
        cfgname="LAN"
    else
	cfgname=$cfgsec
    fi
cat <<EOF
<div class="settings">
<table style="width: 95%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3><strong>$cfgname Port Status</strong></h3></th>
</table>

<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
        <th><h3>General Status</h3></th>
</tr>
<tr>
	<td width="10%">IP Address</td>
	<td width="10%">Connection Type</td>
	<td width="10%">Net Mask</td>
	<td width="10%">MAC Address</td>
</tr>
<tr class="odd">
	<td width="10%">$br_ip</td>
	<td width="10%">$proto</td>
	<td width="10%">$br_mask</td>
	<td width="10%">$br_mac</td>
</tr>
</table>

<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">

<tr>
        <th><h3><strong>Traffic Status</strong></h3></th>
</tr>

<tr>
	<td width="10%">Receive bytes</td>
	<td width="10%">Receive packets</td>
	<td width="10%">Transmit bytes</td>
	<td width="10%">Transmit packets</td>

</tr>

<tr class="odd">
	<td width="10%">$rx_bytes</th>
	<td width="10%">$rx_pkts</th>
	<td width="10%">$tx_bytes</th>
	<td width="10%">$tx_pkts</th>
</tr>
</table>
</div>
EOF

      }
done

br_gw="$(route -n | grep '^0.0.0.0' | awk '{print $2}')"
br_ns=
for ns in $(grep nameserver /tmp/resolv.conf.auto | awk '{print $2}'); do
        br_ns="${br_ns:+$br_ns <br/> }$ns"
done

[ -z "$br_gw" ] && br_gw="None"
[ -z "$br_ns" ] && br_ns="None"

display_form <<EOF
start_form|@TR<<Default Gateway>>
field|@TR<<Gateway>>
string|$br_gw
end_form|

start_form|@TR<<DNS>>
field|@TR<<DNS Server(s)>>
string|$br_ns
end_form|
EOF

display_kernel_routes "4"
[ -f /proc/net/ipv6_route ] && display_kernel_routes "6"

echo "<div class=\"settings\" style=\"float: right;\">"
echo "<form name=\"refresh\" method=\"post\" action=\"$SCRIPT_NAME\">"

[ "$FORM_interval" -gt 0 ] >/dev/null 2>&1 && {
	cat <<EOF
<input type="submit" value=" @TR<<status_network_Stop_Refreshing#Stop Refreshing>> " name="refreshstop" />
@TR<<status_network_Interval#Interval>>: $FORM_interval (@TR<<status_network_in_seconds#in seconds>>)
EOF
} || {
	cat <<EOF
<input type="submit" value=" @TR<<status_network_Auto_Refresh#Auto Refresh>> " name="refreshstart" />
@TR<<status_network_Interval#Interval>>:

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
@TR<<status_network_in_seconds#in seconds>>
EOF
}
echo "</form></div>"

footer_nosubmit ?>
<!--
##WEBIF:name:Network:100:Status
-->
