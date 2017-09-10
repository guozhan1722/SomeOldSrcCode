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

if ! empty "$FORM_add_mac"; then
    un_names=$(cat /var/run/firewall_mac_name)
    for chname in $un_names; do
        if [ "$chname" = "MAC_$FORM_name_mac" ]; then
            append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: MAC List Name not unique</h3></td></tr>" "$N"
            break
        fi
    done
    validate <<EOF
string|FORM_name_mac|@TR<<Name>>|required|$FORM_name_mac
mac|FORM_src_mac_mac|@TR<<Source Mac Address>>||$FORM_src_mac_mac
EOF
	equal "$?" 0 && {
		uci_add firewall rule "MAC_$FORM_name_mac"; add_rule_cfg="$CONFIG_SECTION"
		uci_set firewall "$add_rule_cfg" src_mac "$FORM_src_mac_mac"
		uci_set firewall "$add_rule_cfg" target "$FORM_target_mac"
		unset FORM_name_mac FORM_src_mac_mac FORM_target_mac
	} || {
                append  added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">$ERROR</h3></td></tr>" "$N"

        }
fi

if ! empty "$FORM_add_ip"; then
    un_names=$(cat /var/run/firewall_ip_name)
    for chname in $un_names; do
        if [ "$chname" = "IPLIST_$FORM_name_ip" ]; then
            append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: Rule Name not unique</h3></td></tr>" "$N"
            break
        fi
    done
    validate <<EOF
string|FORM_name_ip|@TR<<Name>>|required|$FORM_name_ip
ip|FORM_src_ip_ip|@TR<<Source IP Address>>||$FORM_src_ip_ip
ip|FORM_src_ip_ip_to|@TR<<Source IP Address To>>||$FORM_src_ip_ip_to
ip|FORM_dest_ip_ip|@TR<<Destination IP Address>>||$FORM_dest_ip_ip
ip|FORM_dest_ip_ip_to|@TR<<Destination IP Address To>>||$FORM_dest_ip_ip_to
EOF
	equal "$?" 0 && {
		uci_add firewall rule "IPLIST_$FORM_name_ip"; add_rule_cfg="$CONFIG_SECTION"
		uci_set firewall "$add_rule_cfg" proto "$FORM_protocol_ip"
		uci_set firewall "$add_rule_cfg" src "$FORM_src_ip"
		uci_set firewall "$add_rule_cfg" src_ip "$FORM_src_ip_ip"
		uci_set firewall "$add_rule_cfg" src_ip_to "$FORM_src_ip_ip_to"
		uci_set firewall "$add_rule_cfg" dest_ip "$FORM_dest_ip_ip"
		uci_set firewall "$add_rule_cfg" dest_ip_to "$FORM_dest_ip_ip_to"
		uci_set firewall "$add_rule_cfg" target "$FORM_target_ip"
		unset FORM_name_ip FORM_dest_ip_ip FORM_dest_ip_ip_to FORM_src_ip_ip FORM_src_ip_ip_to FORM_src_ip FORM_target_ip FORM_protocol_ip
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
FORM_src_mac_mac="00:00:00:00:00:00"
FORM_name_mac="mac1"
FORM_target_mac="ACCEPT"

form="start_form|Firewall MAC List Configuration
        field|@TR<<Name>>
        text|name_mac|$FORM_name_mac

        field|@TR<<Action>>
	select|target_mac|$FORM_target_mac
	option|ACCEPT|@TR<<Accept>>
	option|DROP|@TR<<Drop>>
	option|REJECT|@TR<<Reject>>

        field|@TR<<Mac Address>>
	text|src_mac_mac|$FORM_src_mac_mac
        field|
	submit|add_mac|@TR<<Add Mac List>>
        end_form"
append forms "$form" "$N"

FORM_name_ip="ip1"
FORM_src_ip=""
FORM_target_ip="ACCEPT"
FORM_src_ip_ip="192.168.0.0"
FORM_dest_ip_ip="192.168.0.0"
FORM_src_ip_ip_to="192.168.0.0"
FORM_dest_ip_ip_to="192.168.0.0"
form="start_form|Firewall IP List Configuration
        field|@TR<<Name>>
	text|name_ip|$FORM_name_ip

        field|@TR<<Action>>
	select|target_ip|$FORM_target_ip
	option|ACCEPT|@TR<<Accept>>
	option|DROP|@TR<<Drop>>
	option|REJECT|@TR<<Reject>>

        field|@TR<<Source>>
	select|src_ip|$FORM_src_ip
	$networks
	option||@TR<<None>>

        field|@TR<<Source IPs>>
	text|src_ip_ip|$FORM_src_ip_ip
        string| To 
        string|<td>
        text|src_ip_ip_to|$FORM_src_ip_ip_to
        string|</td>

        field|@TR<<Destination IPs>>
	text|dest_ip_ip|$FORM_dest_ip_ip
        string| To 
        string|<td>
        text|dest_ip_ip_to|$FORM_dest_ip_ip_to
        string|</td>

        field|
        submit|add_ip|@TR<<Add IP List>>
        end_form"
append forms "$form" "$N"

form="string|<div class=\"settings\">
	string|<h3><strong>@TR<<Firewall MAC List Summary>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<>>\">
	string|<th width="10%">@TR<<Name>></th>
	string|<th width="10%">@TR<<Action>></th>
	string|<th>@TR<<Mac Address>></th>
	string|</tr>"
append forms "$form" "$N"

uci_load firewall
config_get_bool disabled_firewall normal disabled_firewall 0

vcfgcnt=0
uniq_names=""
rm -f /var/run/firewall_mac_name 

for rule in $rule_cfgs; do

	echo "$rule" |grep -q "cfg*****" || {

        echo "$rule" |grep -q "MAC_" && { 
        	if [ "$FORM_submit" = "" -o "$add_rule_cfg" = "$rule" ]; then
        		config_get FORM_src_mac "$rule" src_mac "00:00:00:00:00:00"
        		config_get FORM_target "$rule" target "ACCEPT"
        	else
        		eval FORM_src_mac="\$FORM_src_mac_$vcfgcnt"
        		eval FORM_target="\$FORM_target_$vcfgcnt"
validate <<EOF
mac|FORM_src_mac|@TR<<Mac Address>>|required|$FORM_src_mac
EOF
        		equal "$?" 0 && {
        			uci_set firewall "$rule" src_mac "$FORM_src_mac"
        			uci_set firewall "$rule" target "$FORM_target"
        
                        } || {
                            append  added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">$ERROR</h3></td></tr>" "$N"
                        }  
        
        	fi
        
                append uniq_names "$rule" "$N"
                echo "$uniq_names" > /var/run/firewall_mac_name 
                
                name=$(echo "$rule" |cut -c 5-)
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
        		text|src_mac_$vcfgcnt|$FORM_src_mac
        		string|</td>
        		string|<td>
        		string|<a href=\"$SCRIPT_NAME?remove_vcfg=$rule\">@TR<<Remove MAC List >></a>
        		string|</td>
        		string|</tr>"
        	append forms "$form" "$N"
                let "vcfgcnt+=1"
        }
    }
done

form="	string|</tr>
	string|</table></div>"
append forms "$form" "$N"

form="string|<div class=\"settings\">
	string|<h3><strong>@TR<<Firewall IP List Summary>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<>>\">
	string|<th width="10%">@TR<<Name>></th>
	string|<th width="10%">@TR<<Action>></th>
	string|<th>@TR<<Src>></th>
	string|<th>@TR<<Src IP From>></th>
	string|<th>@TR<<Src IP To>></th>
	string|<th>@TR<<Dest IP From>></th>
	string|<th>@TR<<Dest IP To>></th>
	string|</tr>"
append forms "$form" "$N"

vcfgcnt=0
uniq_names=""
rm -f /var/run/firewall_ip_name 

for rule in $rule_cfgs; do

	echo "$rule" |grep -q "cfg*****" || {

        echo "$rule" |grep -q "IPLIST_" && { 
        	if [ "$FORM_submit" = "" -o "$add_rule_cfg" = "$rule" ]; then
        		config_get FORM_src "$rule" src 
        		config_get FORM_src_ip "$rule" src_ip
        		config_get FORM_src_ip_to "$rule" src_ip_to
        		config_get FORM_dest_ip "$rule" dest_ip
        		config_get FORM_dest_ip_to "$rule" dest_ip_to
        		config_get FORM_target "$rule" target "ACCEPT"
        	else
        		eval FORM_src="\$FORM_src_$vcfgcnt"
        		eval FORM_src_ip="\$FORM_src_ip_$vcfgcnt"
        		eval FORM_src_ip_to="\$FORM_src_ip_to_$vcfgcnt"
        		eval FORM_dest_ip="\$FORM_dest_ip_$vcfgcnt"
        		eval FORM_dest_ip_to="\$FORM_dest_ip_to_$vcfgcnt"
        		eval FORM_target="\$FORM_target_$vcfgcnt"
validate <<EOF
ipmask|FORM_src_ip|@TR<<Source IP Address>>||$FORM_src_ip
ipmask|FORM_dest_ip|@TR<<Destination IP Address>>||$FORM_dest_ip
EOF
        		equal "$?" 0 && {
        			uci_set firewall "$rule" src "$FORM_src"
        			uci_set firewall "$rule" src_ip "$FORM_src_ip"
        			uci_set firewall "$rule" src_ip_to "$FORM_src_ip_to"
        			uci_set firewall "$rule" dest_ip "$FORM_dest_ip"
        			uci_set firewall "$rule" dest_ip_to "$FORM_dest_ip_to"
        			uci_set firewall "$rule" target "$FORM_target"
                        } || {
                            append  added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">$ERROR</h3></td></tr>" "$N"
                        }  
        	fi
                append uniq_names "$rule" "$N"
                echo "$uniq_names" > /var/run/firewall_ip_name 
                
                name=$(echo "$rule" |cut -c 8-)
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
        		text|dest_ip_$vcfgcnt|$FORM_dest_ip|||style=\"width:110px\"
        		string|</td>
        		string|<td>
        		text|dest_ip_to_$vcfgcnt|$FORM_dest_ip_to|||style=\"width:110px\"
        		string|</td>
        		string|<td>
        		string|<a href=\"$SCRIPT_NAME?remove_vcfg=$rule\">@TR<<Remove IP List>></a>
        		string|</td>
        		string|</tr>"
        	append forms "$form" "$N"
                let "vcfgcnt+=1"
        }
    }
done

form="	string|</tr>
	string|</table></div>"
append forms "$form" "$N"

header "Firewall" "MAC-IP List" "@TR<<Firewall MAC/IP List>>" 'onload="modechange()"' "$SCRIPT_NAME"

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
##WEBIF:name:Firewall:500:MAC-IP List
-->
