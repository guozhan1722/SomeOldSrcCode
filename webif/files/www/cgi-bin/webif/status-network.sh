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
logger "network_devices=$network_devices"
for cfgsec in $network_devices; do
      logger "cfgsec=$cfgsec"
      [ "$cfgsec" = "loopback" ] || {
	 eval bridge_name="br-$cfgsec"
	 logger "bridge $bridge_name"
	 config_get proto "$cfgsec" proto

	 br_mac="$(ifconfig "${bridge_name}" | grep HWaddr | awk -F'[ ]+' '{print $5}')"
	 br_ip="$(ifconfig "${bridge_name}" | grep inet | awk -F'[: ]+' '{print $4}')"
	 br_mask="$(ifconfig "${bridge_name}" | grep inet | awk -F'[: ]+' '{print $8}')"

	 display_form <<EOF
start_form|$cfgsec
field|@TR<<Connection Type>>
string|$proto
field|@TR<<IP Address>>
string|$br_ip
field|@TR<<Net Mask>>
string|$br_mask
field|@TR<<MAC Address>>
string|$br_mac
end_form|
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
echo "</form></div><br/>"

footer ?>
<!--
##WEBIF:name:Network:100:Status
-->
