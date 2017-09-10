#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		mwanfw)
			append mwanfw_cfgs "$cfg_name" "$N"
		;;
	esac
}

#remove rule
if ! empty "$FORM_remove_vcfg"; then
	uci_remove "multiwan" "$FORM_remove_vcfg"
fi

if ! empty "$FORM_add_rule"; then
    #[ -z "$FORM_name_rule" ] && append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: Rule Name not unique</h3></td></tr>" "$N"
    un_names=$(cat /var/run/multiwan_rule_name)
    for chname in $un_names; do
        if [ "$chname" = "$FORM_name_rule" ]; then
            FORM_name_rule=""
            append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: Rule Name not unique</h3></td></tr>" "$N"
            break
        fi

    done
    validate <<EOF
string|FORM_name_rule|@TR<<Name>>|required|$FORM_name_rule
ports|FORM_ports_rule|@TR<<Destination Port>>||$FORM_ports_rule
EOF
	equal "$?" 0 && {
		uci_add multiwan mwanfw ; add_rule_cfg="$CONFIG_SECTION"
		uci_set multiwan "$add_rule_cfg" name "$FORM_name_rule"
		uci_set multiwan "$add_rule_cfg" src "$FORM_src_rule"
		uci_set multiwan "$add_rule_cfg" dst "$FORM_dst_rule"
		uci_set multiwan "$add_rule_cfg" proto "$FORM_proto_rule"
		uci_set multiwan "$add_rule_cfg" ports "$FORM_ports_rule"
		uci_set multiwan "$add_rule_cfg" wanrule "$FORM_wanrule_rule"
		uci_set multiwan "$add_rule_cfg" port_type "$FORM_port_type_rule"

		unset FORM_name_rule FORM_dst_rule FORM_src_rule FORM_proto_rule FORM_ports_rule FORM_wanrule_rule FORM_port_type_rule
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

FORM_name_rule="rule_01"
FORM_src_rule="all"
FORM_dst_rule="all"
FORM_proto_rule="all"
FORM_ports_rule="80"
FORM_port_type_rule="all"
FORM_wanrule_rule="wan"

form="start_form|Traffic Rules Configuration
            field|@TR<<Rule Name>>
	    text|name_rule|$FORM_name_rule

            field|@TR<<Source Address>>
	    text|src_rule|$FORM_src_rule

            field|@TR<<Destination Address>>
	    text|dst_rule|$FORM_dst_rule

            field|@TR<<Protocol>>
            select|proto_rule|$FORM_proto_rule
	    option|all|All
	    option|tcp|TCP
	    option|udp|UDP
	    option|icmp|ICMP

            field|@TR<<Ports>>
	    text|ports_rule|$FORM_ports_rule

            field|@TR<<Port Type>>
	    select|port_type_rule|$FORM_port_type_rule
	    option||All
	    option|source-ports|Source Port
	    option|destination-ports|Destination Port

            field|@TR<<WAN Uplink>>
            select|wanrule_rule|$FORM_wanrule_rule
	    option|wan2|4G
	    option|wan|WAN
            option|fastbalancer|Default

            field|
            submit|add_rule|@TR<<Add Rule>>
            end_form"
append forms "$form" "$N"
#            option|balancer|Load Balancer Compatibility
#            option|fastbalancer|Load Balancer Performance

form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<Firewall Rules Summary>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<Firewall Rules Summary>>\">
	string|<th>@TR<<Name>></th>
	string|<th>@TR<<Source Address>></th>
	string|<th>@TR<<Destination Address>></th>
	string|<th>@TR<<Protocol>></th>
	string|<th>@TR<<Ports>></th>
	string|<th>@TR<<P Type>></th>
	string|<th>@TR<<WAN Uplink>></th>
	string|</tr>"
append forms "$form" "$N"

uci_load multiwan
config_get_bool enabled_multiwan config enabled 0

vcfgcnt=0
uniq_names=""
rm -rf /var/run/multiwan_rule_name

for rule in $mwanfw_cfgs; do
        logger "the rule is $rule"

	if [ "$FORM_submit" = "" -o "$add_rule_cfg" = "$rule" ]; then
		config_get FORM_name "$rule" name 
		config_get FORM_src "$rule" src 
		config_get FORM_dst "$rule" dst
		config_get FORM_proto "$rule" proto
		config_get FORM_ports "$rule" ports
		config_get FORM_port_type "$rule" port_type
		config_get FORM_wanrule "$rule" wanrule
	else
		config_get FORM_name "$rule" name 
		#eval FORM_name="\$FORM_name_$vcfgcnt"
		eval FORM_src="\$FORM_src_$vcfgcnt"
		eval FORM_dst="\$FORM_dst_$vcfgcnt"
		eval FORM_proto="\$FORM_proto_$vcfgcnt"
		eval FORM_ports="\$FORM_ports_$vcfgcnt"
		eval FORM_wanrule="\$FORM_wanrule_$vcfgcnt"
		eval FORM_port_type="\$FORM_port_type_$vcfgcnt"
validate <<EOF
ports|FORM_dest_port|@TR<<Destination Port>>||$FORM_dest_port
EOF
		equal "$?" 0 && {
			#uci_set multiwan "$rule" name "$FORM_name"
			uci_set multiwan "$rule" src "$FORM_src"
			uci_set multiwan "$rule" dst "$FORM_dst"
			uci_set multiwan "$rule" proto "$FORM_proto"
			uci_set multiwan "$rule" ports "$FORM_ports"
			uci_set multiwan "$rule" wanrule "$FORM_wanrule"
                        uci_set multiwan "$rule" port_type "$FORM_port_type"
                } || {
                    append  added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">$ERROR</h3></td></tr>" "$N"
                }  

	fi

        #name=$(echo "$FORM_name" |cut -c 8-)
        append uniq_names "$FORM_name" "$N"
        echo "$uniq_names" > /var/run/multiwan_rule_name 

        get_tr
	form="$tr  
                string|<td>
                string|$FORM_name
                string|</td>

		string|<td>
		text|src_$vcfgcnt|$FORM_src
		string|</td>

		string|<td>
		text|dst_$vcfgcnt|$FORM_dst
		string|</td>

		string|<td>
		select|proto_$vcfgcnt|$FORM_proto
		option|all|ALL
		option|tcp|TCP
		option|udp|UDP
		option|icmp|ICMP
		string|</td>

		string|<td>
		text|ports_$vcfgcnt|$FORM_ports
		string|</td>

        	string|<td>
        	select|port_type_$vcfgcnt|$FORM_port_type
                option||All
	        option|source-ports|Src
                option|destination-ports|Dest
        	string|</td>

		string|<td>
		select|wanrule_$vcfgcnt|$FORM_wanrule
                option|wan2|4G
                option|wan|WAN
                option|fastbalancer|Default
		string|</td>

		string|<td>
		string|<a href=\"$SCRIPT_NAME?remove_vcfg=$rule\">@TR<<Remove Rule>></a>
		string|</td>
		string|</tr>"
	append forms "$form" "$N"
        let "vcfgcnt+=1"
done
#                option|balancer|Compatibility

form="  string|</tr>
        string|</table></div>"

append forms "$form" "$N"

header "MultiWAN" "Traffic" "@TR<<Multi WAN Traffic Rules>>" ' onload="modechange()" ' "$SCRIPT_NAME"

if [ "$enabled_multiwan" != "1" ];then
display_form <<EOF
string|<tr><td colspan=\"2\"><h3 class=\"warning\">Multi WAN is Disabled </h3></td></tr>
EOF
else
display_form <<EOF
$added_error
$forms
EOF
fi

footer ?>
<!--
##WEBIF:name:MultiWAN:300:Traffic
-->
