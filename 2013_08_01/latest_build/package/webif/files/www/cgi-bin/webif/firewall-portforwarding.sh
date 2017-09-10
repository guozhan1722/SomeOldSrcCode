#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		redirect)
			append redirect_cfgs "$cfg_name" "$N"
		;;
		dmz)
			append admz_cfgs "$cfg_name" "$N"
		;;

		interface)
			if [ "$cfg_name" != "loopback" ]; then
				append networks "option|$cfg_name" "$N"
                                append netmode "$cfg_name" "$N"
			fi
		;;
	esac
}


#remove rule
if ! empty "$FORM_remove_vcfg"; then
	uci_remove "firewall" "$FORM_remove_vcfg"
fi

if ! empty "$FORM_add_redirect"; then
    un_names=$(cat /var/run/firewall_portforwarding_name)
    for chname in $un_names; do
        if [ "$chname" = "PORTFD_$FORM_name_redirect" ]; then
            append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: Port Forwarding Name not unique</h3></td></tr>" "$N"
            break
        fi
    done

validate <<EOF
string|FORM_name_redirect|@TR<<Name>>|required|$FORM_name_redirect
ip|FORM_src_ip_redirect|@TR<<Source IP Address>>||$FORM_src_ip_redirect
ports|FORM_src_dport_redirect|@TR<<Destination Port>>|required|$FORM_src_dport_redirect
ip|FORM_dest_ip_redirect|@TR<<To IP Address>>|required|$FORM_dest_ip_redirect
ports|FORM_dest_port_redirect|@TR<<To Port>>||$FORM_dest_port_redirect
EOF
	equal "$?" 0 && {
		uci_add firewall redirect "PORTFD_$FORM_name_redirect"; add_redirect_cfg="$CONFIG_SECTION"
		uci_set firewall "$add_redirect_cfg" src "$FORM_port_redirect"
		uci_set firewall "$add_redirect_cfg" proto "$FORM_protocol_redirect"
		uci_set firewall "$add_redirect_cfg" src_ip "$FORM_src_ip_redirect"
		uci_set firewall "$add_redirect_cfg" src_dport "$FORM_src_dport_redirect"
		uci_set firewall "$add_redirect_cfg" dest_ip "$FORM_dest_ip_redirect"
		uci_set firewall "$add_redirect_cfg" dest_port "$FORM_dest_port_redirect"
		unset FORM_name_redirect FORM_dest_port_redirect FORM_port_redirect FORM_dest_ip_redirect FORM_src_dport_redirect FORM_src_ip_redirect FORM_protocol_redirect 
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
uci_load firewall

dmz_cfgs=dmz1

if [ "$FORM_submit" = "" ]; then
    config_get_bool FORM_disabled_dmz "$dmz_cfgs" disabled_dmz 1
    config_get FORM_dmz_ip "$dmz_cfgs" dmz_ip 192.168.100.100
    config_get FORM_dmz_src "$dmz_cfgs" dmz_src wan2
    config_get FORM_dmz_ex_port "$dmz_cfgs" dmz_ex_port 0
else
    FORM_disabled_dmz="$FORM_disabled_dmz_g"
    FORM_dmz_src="$FORM_dmz_src_g"
    FORM_dmz_ip="$FORM_dmz_ip_g"
    FORM_dmz_ex_port="$FORM_dmz_ex_port_g"


validate <<EOF
ip|FORM_dmz_ip|@TR<<DMZ Server IP Address>>||$FORM_dmz_ip
ports|FORM_dmz_ex_port|@TR<<DMZ Exception Port>>||$FORM_dmz_ex_port
EOF
    equal "$?" 0 && {        
  #      empty "$dmz_cfgs" && {
  #          uci_add firewall dmz "dmz" 
  #          add_dmz_cfg="$CONFIG_SECTION"
  #          uci_set firewall "$add_dmz_cfg" disabled_dmz "$FORM_disabled_dmz"
  #          uci_set firewall "$add_dmz_cfg" dmz_ip "$FORM_dmz_ip"
  #          uci_set firewall "$add_dmz_cfg" dmz_ex_port "$FORM_dmz_ex_port"

  #      } || {

            uci_set firewall "$dmz_cfgs" disabled_dmz "$FORM_disabled_dmz"
            uci_set firewall "$dmz_cfgs" dmz_src "$FORM_dmz_src"
            uci_set firewall "$dmz_cfgs" dmz_ip "$FORM_dmz_ip"
            uci_set firewall "$dmz_cfgs" dmz_ex_port "$FORM_dmz_ex_port"
   #     }
    }

fi

form="start_form|Firewall DMZ Configuration
    field|@TR<<DMZ Mode>>
    select|disabled_dmz_g|$FORM_disabled_dmz
    option|0|@TR<<Enable>>
    option|1|@TR<<Disable>>

    field|@TR<<DMZ Source>>
    select|dmz_src_g|$FORM_dmz_src
    option|wan2|@TR<<4G>>
    option|wan|@TR<<WAN>>

    field|@TR<<DMZ Server IP>>
    text|dmz_ip_g|$FORM_dmz_ip

    field|@TR<<Exception Port>>
    text|dmz_ex_port_g|$FORM_dmz_ex_port
    end_form"
append forms "$form" "$N"


FORM_name_redirect="forward1"
FORM_src_dport_redirect="2000"
FORM_dest_ip_redirect="192.168.2.1"
FORM_port_redirect="wan2"
FORM_dest_port_redirect="3000"
FORM_protocol_redirect="tcp"

form="start_form|Firewall Port Forwarding Configuration
            field|@TR<<Name>>
	    text|name_redirect|$FORM_name_redirect

            field|@TR<<Source>>
            select|port_redirect|$FORM_port_redirect
	    option|wan2|4G
	    option|wan|WAN

            field|@TR<<Internal Server IP>>
            text|dest_ip_redirect|$FORM_dest_ip_redirect

            field|@TR<<Internal Port>>
            text|dest_port_redirect|$FORM_dest_port_redirect

            field|@TR<<Protocol>>
            select|protocol_redirect|$FORM_protocol_redirect
	    option|tcp|TCP
	    option|udp|UDP
	    option|tcpudp|Both

            field|@TR<<External Port>>
            text|src_dport_redirect|$FORM_src_dport_redirect

            field|
            submit|add_redirect|@TR<<Add Port Forwarding>>
            end_form"
append forms "$form" "$N"

form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<Firewall Port Forwarding Summary>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<>>\">
	string|<th>@TR<<Name>></th>
	string|<th>@TR<<Source>></th>
	string|<th>@TR<<Internal IP>></th>
	string|<th>@TR<<Internal Port>></th>
	string|<th>@TR<<Protocol>></th>
	string|<th>@TR<<External Port>></th>
	string|</tr>"
append forms "$form" "$N"

config_get_bool disabled_firewall normal disabled_firewall 0

vcfgcnt=0
uniq_names=""
rm -rf /var/run/firewall_portforwarding_name
for rule in $redirect_cfgs; do
	echo "$rule" |grep -q "cfg*****" || {

        echo "$rule" |grep -q "PORTFD_" && { 
	if [ "$FORM_submit" = "" -o "$add_redirect_cfg" = "$rule" ]; then
		config_get FORM_protocol "$rule" proto
		config_get FORM_dest_ip "$rule" dest_ip
		config_get FORM_port "$rule" src
		config_get FORM_src_dport "$rule" src_dport
		config_get FORM_dest_port "$rule" dest_port
	else
		eval FORM_protocol="\$FORM_protocol_$vcfgcnt"
		eval FORM_dest_ip="\$FORM_dest_ip_$vcfgcnt"
		eval FORM_port="\$FORM_port_$vcfgcnt"
		eval FORM_dest_port="\$FORM_dest_port_$vcfgcnt"
		eval FORM_src_dport="\$FORM_src_dport_$vcfgcnt"
		validate <<EOF
ports|FORM_src_dport|@TR<<External Port>>|required|$FORM_src_dport
ipmask|FORM_dest_ip|@TR<<Internal Server IP>>|required|$FORM_dest_ip
ports|FORM_dest_dport|@TR<<Internal Port>>|required|$FORM_dest_port
EOF
		equal "$?" 0 && {
			uci_set firewall "$rule" proto "$FORM_protocol"
			uci_set firewall "$rule" dest_ip "$FORM_dest_ip"
			uci_set firewall "$rule" src "$FORM_port"
			uci_set firewall "$rule" src_dport "$FORM_src_dport"
			uci_set firewall "$rule" dest_port "$FORM_dest_port"
		}|| {
                    append  added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">$ERROR</h3></td></tr>" "$N"
                } 
	fi

        append uniq_names "$rule" "$N"
        echo "$uniq_names" > /var/run/firewall_portforwarding_name 

        name=$(echo "$rule" |cut -c 8-)
        get_tr
	form="  $tr
		string|<td>$name</td>
		string|<td>
		select|port_$vcfgcnt|$FORM_port
		option|wan2|4G
		option|wan|WAN
		string|</td>
		string|<td>
		text|dest_ip_$vcfgcnt|$FORM_dest_ip
		string|</td>
		string|<td>
		text|dest_port_$vcfgcnt|$FORM_dest_port
		string|</td>
		string|<td>
		select|protocol_$vcfgcnt|$FORM_protocol
		option|tcp|TCP
		option|udp|UDP
		option|tcpudp|Both
		string|</td>
		string|<td>
		text|src_dport_$vcfgcnt|$FORM_src_dport
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

form="	string|</tr>
	string|</table></div>"
append forms "$form" "$N"

header "Firewall" "Port Forwarding" "@TR<<Firewall Port Forwarding>>" 'onload="modechange()"' "$SCRIPT_NAME"

#PORT Forwarding
#no port forwarding when network is bridge mode
sysmode=$(uci get system.@system[0].systemmode)

if [ "$sysmode" = "gateway" ] ; then
display_form <<EOF
$added_error
$forms
EOF
elif [ "$sysmode" = "router" -o "$sysmode" = "bridge" ] ; then
display_form <<EOF
string|<tr><td colspan=\"2\"><h3 class=\"warning\">No port forwarding in Bridge or router mode </h3></td></tr>
EOF
elif [ "$disabled_firewall" = "1" ];then
display_form <<EOF
string|<tr><td colspan=\"2\"><h3 class=\"warning\">Firewall is Disabled </h3></td></tr>
EOF
else
display_form <<EOF
string|<tr><td colspan=\"2\"><h3 class=\"warning\">Can not get System mode</h3></td></tr>
EOF
fi

footer ?>
<!--
##WEBIF:name:Firewall:400:Port Forwarding
-->
