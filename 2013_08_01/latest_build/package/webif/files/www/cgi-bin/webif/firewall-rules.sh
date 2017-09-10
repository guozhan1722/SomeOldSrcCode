#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		rule)
			append rule_cfgs "$cfg_name" "$N"
		;;
		interface)
			if [ "$cfg_name" != "loopback" ]; then
                                if [ "$cfg_name" = "wan" ];then
                                    iname="WAN"
                                elif [ "$cfg_name" = "wan2" ];then
                                    iname="4G"
                                elif [ "$cfg_name" = "lan" ];then
                                    iname="LAN"
				else	
				    iname=$cfg_name
				fi
				append networks "option|$cfg_name|$iname" "$N"
                                append netmode "$cfg_name" "$N"
			fi
		;;
	esac
}

#remove rule
if ! empty "$FORM_remove_vcfg"; then
	uci_remove "firewall" "$FORM_remove_vcfg"
fi

if ! empty "$FORM_add_rule"; then
    #[ -z "$FORM_name_rule" ] && append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: Rule Name not unique</h3></td></tr>" "$N"

    un_names=$(cat /var/run/firewall_rule_name)
    for chname in $un_names; do
        if [ "$chname" = "CUSTOM_$FORM_name_rule" ]; then
            append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: Rule Name not unique</h3></td></tr>" "$N"
            break
        fi
    done
    validate <<EOF
string|FORM_name_rule|@TR<<Name>>|required|$FORM_name_rule
ip|FORM_src_ip_rule|@TR<<Source IP Address>>||$FORM_src_ip_rule
ip|FORM_src_ip_to_rule|@TR<<Source IP Address To>>||$FORM_src_ip_to_rule
ip|FORM_dest_ip_rule|@TR<<Destination IP Address>>||$FORM_dest_ip_rule
ip|FORM_dest_ip_to_rule|@TR<<Destination IP Address To>>||$FORM_dest_ip_to_rule
ports|FORM_dest_port_rule|@TR<<Destination Port>>||$FORM_dest_port_rule
EOF
	equal "$?" 0 && {
		uci_add firewall rule "CUSTOM_$FORM_name_rule"; add_rule_cfg="$CONFIG_SECTION"
		uci_set firewall "$add_rule_cfg" proto "$FORM_protocol_rule"
		uci_set firewall "$add_rule_cfg" src "$FORM_src_rule"
		uci_set firewall "$add_rule_cfg" dest "$FORM_dest_rule"
		uci_set firewall "$add_rule_cfg" src_ip "$FORM_src_ip_rule"
		uci_set firewall "$add_rule_cfg" src_ip_to "$FORM_src_ip_to_rule"
		uci_set firewall "$add_rule_cfg" dest_ip "$FORM_dest_ip_rule"
		uci_set firewall "$add_rule_cfg" dest_ip_to "$FORM_dest_ip_to_rule"
		uci_set firewall "$add_rule_cfg" dest_port "$FORM_dest_port_rule"
		uci_set firewall "$add_rule_cfg" target "$FORM_target_rule"
		unset FORM_name_rule FORM_dest_ip_rule FORM_dest_ip_to_rule FORM_dest_port_rule FORM_src_ip_rule FORM_src_ip_to_rule FORM_protocol_rule FORM_src_rule FORM_dest_rule FORM_target_rule
	} || {
                append  added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">$ERROR</h3></td></tr>" "$N"

        }
fi

cur_color="odd"
get_tr() {
	if equal "$cur_color" "odd"; then
		cur_color="even"
		tr="string|<tr>"
	else
		cur_color="odd"
		tr="string|<tr class=\"odd\">"
	fi
}

#init the adding field
uci_load network

FORM_name_rule="rule1"
FORM_target_rule="ACCEPT"
FORM_src_rule=""
FORM_src_ip_rule="192.168.0.0"
FORM_src_ip_to_rule="192.168.0.0"
FORM_dest_ip_rule="192.168.0.0"
FORM_dest_ip_to_rule="192.168.0.0"
FORM_dest_rule=""
FORM_dest_port_rule="0"
FORM_protocol_rule="tcp"

form="start_form|Firewall Rules Configuration
            field|@TR<<Rule Name>>
	    text|name_rule|$FORM_name_rule

            field|@TR<<ACTION>>
            select|target_rule|$FORM_target_rule
            option|ACCEPT|@TR<<Accept>>
            option|DROP|@TR<<Drop>>
            option|REJECT|@TR<<Reject>>

            field|@TR<<Source>>
	    select|src_rule|$FORM_src_rule
	    $networks
	    option||@TR<<None>>
            
            field|@TR<<Source IPs>>
	    text|src_ip_rule|$FORM_src_ip_rule
            string| To 
            string|<td>
            text|src_ip_to_rule|$FORM_src_ip_to_rule
            string|</td>

            field|@TR<<Destination>>
            select|dest_rule|$FORM_dest_rule
            $networks
            option||@TR<<None>>

            field|@TR<<Destination IPs>>
            text|dest_ip_rule|$FORM_dest_ip_rule
            string| To 
            string|<td>
            text|dest_ip_to_rule|$FORM_dest_ip_to_rule
            string|</td>

            field|@TR<<Destination Port>>
	    text|dest_port_rule|$FORM_dest_port_rule

            field|@TR<<Protocol>>
            select|protocol_rule|$FORM_protocol_rule
	    option|tcp|TCP
	    option|udp|UDP
	    option|tcpudp|Both
	    option|icmp|ICMP
            field|
            submit|add_rule|@TR<<Add Rule>>
            end_form"
append forms "$form" "$N"

form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<Firewall Rules Summary>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<Firewall Rules Summary>>\">
	string|<th>@TR<<Name>></th>
	string|<th>@TR<<Action>></th>
	string|<th>@TR<<Src>></th>
	string|<th>@TR<<Src IP From>></th>
	string|<th>@TR<<Src IP To>></th>
	string|<th>@TR<<Dest>></th>
	string|<th>@TR<<Dest IP From>></th>
	string|<th>@TR<<Dest IP To>></th>
	string|<th>@TR<<Destination Port>></th>
	string|<th>@TR<<Protocol>></th>
	string|</tr>"
append forms "$form" "$N"

uci_load firewall
config_get_bool disabled_firewall normal disabled_firewall 0

vcfgcnt=0
uniq_names=""
rm -rf /var/run/firewall_rule_name

for rule in $rule_cfgs; do
        logger "the rule is $rule"

	echo "$rule" |grep -q "cfg*****" || echo "$rule" | grep -q "MAC_" || echo "$rule" | grep -q "IPLIST_" || echo "$rule" | grep -q "DEFRule_" || {

        echo "$rule" |grep -q "CUSTOM_" && { 
	if [ "$FORM_submit" = "" -o "$add_rule_cfg" = "$rule" ]; then
		config_get FORM_src "$rule" src 
		config_get FORM_dest "$rule" dest
		config_get FORM_protocol "$rule" proto
		config_get FORM_src_ip "$rule" src_ip
		config_get FORM_src_ip_to "$rule" src_ip_to
		config_get FORM_dest_ip "$rule" dest_ip
		config_get FORM_dest_ip_to "$rule" dest_ip_to
                config_get FORM_dest_port "$rule" dest_port 
		config_get FORM_target "$rule" target "ACCEPT"
	else
		eval FORM_src="\$FORM_src_$vcfgcnt"
		eval FORM_dest="\$FORM_dest_$vcfgcnt"
		eval FORM_protocol="\$FORM_protocol_$vcfgcnt"
		eval FORM_src_ip="\$FORM_src_ip_$vcfgcnt"
		eval FORM_src_ip_to="\$FORM_src_ip_to_$vcfgcnt"
		eval FORM_dest_ip="\$FORM_dest_ip_$vcfgcnt"
		eval FORM_dest_ip_to="\$FORM_dest_ip_to_$vcfgcnt"
		eval FORM_dest_port="\$FORM_dest_port_$vcfgcnt"
		eval FORM_target="\$FORM_target_$vcfgcnt"
validate <<EOF
ip|FORM_src_ip|@TR<<Source IP Address>>||$FORM_src_ip
ip|FORM_src_ip_to|@TR<<Source IP Address>>||$FORM_src_ip_to
ip|FORM_dest_ip|@TR<<Destination IP Address>>||$FORM_dest_ip
ip|FORM_dest_ip_to|@TR<<Destination IP Address>>||$FORM_dest_ip_to
ports|FORM_dest_port|@TR<<Destination Port>>||$FORM_dest_port
EOF
		equal "$?" 0 && {
			uci_set firewall "$rule" src "$FORM_src"
			uci_set firewall "$rule" dest "$FORM_dest"
			uci_set firewall "$rule" proto "$FORM_protocol"
			uci_set firewall "$rule" src_ip "$FORM_src_ip"
			uci_set firewall "$rule" src_ip_to "$FORM_src_ip_to"
			uci_set firewall "$rule" dest_ip "$FORM_dest_ip"
			uci_set firewall "$rule" dest_ip_to "$FORM_dest_ip_to"
                        uci_set firewall "$rule" dest_port "$FORM_dest_port"
			uci_set firewall "$rule" target "$FORM_target"

                } || {
                    append  added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">$ERROR</h3></td></tr>" "$N"
                }  

	fi

        name=$(echo "$rule" |cut -c 8-)
        append uniq_names "$rule" "$N"
        echo "$uniq_names" > /var/run/firewall_rule_name 

        get_tr
	form="$tr  
                string|<td>
                string|$name
                string|</td>
        	string|<td>
        	select|target_$vcfgcnt|$FORM_target
        	option|ACCEPT|@TR<<Accept>>
        	option|DROP|@TR<<Drop>>
        	option|REJECT|@TR<<Reject>>
        	string|</td>
		string|<td>
		select|src_$vcfgcnt|$FORM_src
		$networks
		option||@TR<<None>>
		string|</td>
		string|<td>
		text|src_ip_$vcfgcnt|$FORM_src_ip|||style=\"width:110px\"
		string|</td>
		string|<td>
		text|src_ip_to_$vcfgcnt|$FORM_src_ip_to|||style=\"width:110px\"
		string|</td>
		string|<td>
		select|dest_$vcfgcnt|$FORM_dest
		$networks
		option||@TR<<None>>
		string|</td>
		string|<td>
		text|dest_ip_$vcfgcnt|$FORM_dest_ip|||style=\"width:110px\"
		string|</td>
		string|<td>
		text|dest_ip_to_$vcfgcnt|$FORM_dest_ip_to|||style=\"width:110px\"
		string|</td>
		string|<td>
		text|dest_port_$vcfgcnt|$FORM_dest_port|||style=\"width:110px\"
		string|</td>
		string|<td>
		select|protocol_$vcfgcnt|$FORM_protocol
		option|tcp|TCP
		option|udp|UDP
		option|tcpudp|Both
		option|icmp|ICMP
		string|</td>
		string|<td>
		string|<a href=\"$SCRIPT_NAME?remove_vcfg=$rule\">@TR<<Remove Rule>></a>
		string|</td>
		string|</tr>"
	append forms "$form" "$N"

        let "vcfgcnt+=1"
    }
    }
done

form="  string|</tr>
        string|</table></div>"

append forms "$form" "$N"

header "Firewall" "Rules" "@TR<<Firewall Rules>>" 'onload="modechange()"' "$SCRIPT_NAME"

if [ "$disabled_firewall" = "1" ];then
display_form <<EOF
string|<tr><td colspan=\"2\"><h3 class=\"warning\">Firewall is Disabled </h3></td></tr>
EOF
else
display_form <<EOF
$added_error
$forms
EOF
fi

footer ?>
<!--
##WEBIF:name:Firewall:300:Rules
-->
