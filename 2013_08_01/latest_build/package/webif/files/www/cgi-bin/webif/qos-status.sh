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

	case "$cfg_type" in
		general)
			append general_cfgs "$cfg_name" "$N"
		;;
	esac
}

show_qos_status(){

    local iface="$1"
    tc -s class show dev "$iface" > /var/run/qos_status_$iface
}

parse_status() {
    local parse_item="$1"

    awk -v "parse=$parse_item" '
    BEGIN { nc = 0; }
	$1 == "class" && $2 == "htb" {
	nc = nc + 1;	# strip leading zero
        target[$8] = $8
        idx[nc]=$8
        rate[$8] = $10
        ceil[$8] = $12
        burst[$8] = $14
    }
    nc > 0 {
        
	if ($1 ~ /^Sent/) {
		sent_b[idx[nc]] = $2;
		sent_p[idx[nc]] = $4;
		next;
	}

	if ($1 ~ /^rate/) {
		rate_b[idx[nc]] = $2;
	}

    }
    END {	
        for (i=1; i <= nc; i++) {
            if(parse=="general") {
                if(target[i] != 1 ) {
                    if(target[i]==2) {
                        level="High"
                    } else if (target[i]==3) {
                        level="Medium_high"
                    } else if (target[i]==4) {
                        level="Medium"
                    } else if (target[i]==5) {
                        level="Medium_low"
                    } else if (target[i]==6) {
                        level="low"
                    }
    
                    if (i % 2 == 0)
                        print("<tr class=\"odd\">");
                    else
                        print("<tr>");
    
                    printf("<td width=\"15\">%s</td>",level);
                    printf("<td width=\"15\">%s</td>",rate[i]);
                    printf("<td width=\"15\">%s</td>",ceil[i]);
                    printf("<td width=\"15\">%s</td>",burst[i]);
                    print("</tr>");
                }
            }

            if(parse=="traffic") {
                if(target[i] != 1 ) {
                    if(target[i]==2) {
                        level="High"
                    } else if (target[i]==3) {
                        level="Medium_high"
                    } else if (target[i]==4) {
                        level="Medium"
                    } else if (target[i]==5) {
                        level="Medium_low"
                    } else if (target[i]==6) {
                        level="low"
                    }
    
                    if (i % 2 == 0)
                        print("<tr class=\"odd\">");
                    else
                        print("<tr>");
    
                    printf("<td width=\"15\">%s</td>",level);
                    printf("<td width=\"15\">%s</td>",sent_b[i]);
                    printf("<td width=\"15\">%s</td>",sent_p[i]);
                    printf("<td width=\"15\">%s</td>",rate_b[i]);
                    print("</tr>");
                }
            }
        }
    }

    ' "$2"
}

header "Qos" "Status" "@TR<<Quality of Service Statistics>>"

uci_load "qos"
sysmode=$(uci get system.@system[0].systemmode)

config_get FORM_enabled "$general_cfgs" enabled

if equal "$FORM_enabled" "1"; then
    if [ "$sysmode" = "router" -o "$sysmode" = "gateway" ] ; then

    #show br-lan qos_status
    show_qos_status br-lan

cat <<EOF
<div class="settings">
<table style="width: 95%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<th><h3><strong>LAN Port Status</strong></h3></th></table>

<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
        <th><h3>General Status</h3></th>
</tr>
   
    <tr>
        <td width="15%">@TR<<Target>></td>
        <td width="15%">@TR<<Rate>></td>
        <td width="15%">@TR<<Ceil>></td>
        <td width="15%">@TR<<Burst>></td>
    </tr>
EOF

    parse_status general /var/run/qos_status_br-lan

cat <<EOF
    </table>
EOF

cat <<EOF
<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
        <th><h3>Traffic Status</h3></th>
</tr>
   
    <tr>
        <td width="15%">@TR<<Target>></td>
        <td width="15%">@TR<<Sent (bytes)>></td>
        <td width="15%">@TR<<Sent (pkts)>></td>
        <td width="15%">@TR<<Rate (bit)>></td>
    </tr>
EOF
    parse_status traffic /var/run/qos_status_br-lan

cat <<EOF
    </table>
EOF

cat <<EOF
    </div>
EOF

    #show br-wan qos_status
    show_qos_status br-wan

cat <<EOF
<div class="settings">
<table style="width: 95%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<th><h3><strong>WAN Port Status</strong></h3></th></table>

<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
        <th><h3>General Status</h3></th>
</tr>
   
    <tr>
        <td width="15%">@TR<<Target>></td>
        <td width="15%">@TR<<Rate>></td>
        <td width="15%">@TR<<Ceil>></td>
        <td width="15%">@TR<<Burst>></td>
    </tr>
EOF

    parse_status general /var/run/qos_status_br-wan

cat <<EOF
    </table>
EOF

cat <<EOF
<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
        <th><h3>Traffic Status</h3></th>
</tr>
   
    <tr>
        <td width="15%">@TR<<Target>></td>
        <td width="15%">@TR<<Sent (bytes)>></td>
        <td width="15%">@TR<<Sent (pkts)>></td>
        <td width="15%">@TR<<Rate (bit)>></td>
    </tr>
EOF
    parse_status traffic /var/run/qos_status_br-wan

cat <<EOF
    </table>
EOF

cat <<EOF
    </div>
EOF
    else
        append Qos_info "string|<tr><td colspan=\"2\"><h3 interface=\"warning\">No Qos Status in Bridge mode  </h3></td></tr>" "$N"
    fi
else
    append Qos_info "string|<tr><td colspan=\"2\"><h3 interface=\"warning\">Qos Mode is Disabled </h3></td></tr>" "$N"
fi

display_form <<EOF
$Qos_info
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
##WEBIF:name:Qos:100:Status
-->
