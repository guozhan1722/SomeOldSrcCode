#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"

config_cb() {
    local cfg_type="$1"
    local cfg_name="$2"
    
    case "$cfg_type" in
            host)
                    append host_cfgs "$cfg_name" "$N"
            ;;
            interface)
                    append network_if_cfgs "$cfg_name" "$N"
            ;;
            wifi-iface)
                    append vfaces "$cfg_name" "$N"
            ;;
            dnsmasq)
                    append main_dnsmasq "$cfg_name" "$N"
            ;;

    esac
}

if [ "$FORM_submit" != "" -o "$FORM_add_item" != "" -o "$FORM_remove_network_if" != "" -o "$FORM_add_network_if" != "" ] ; then
    #prevent user submit when system is in IP passthrough mode
    local ippassthrough=$(uci get lte.connect.ippassthrough)
    [ "$ippassthrough" = "Ethernet" -o "$ippassthrough" = "WANport" ] && {
         append validate_error "Error in Submit: Can not submit in IP Passthrough Mode" "$N"
    }
fi
######## use click release all button ##############
if ! empty "$FORM_releaseall_button"; then
    rm /tmp/dhcp.leases
    /etc/init.d/dnsmasq restart
    logger "Network DHCP server is restarted."
fi

######## use click refresh button ##############
if ! empty "$FORM_refresh_button"; then
    logger "Network lan webpage is refresh."
fi


######### Release a DHCP ip/mac pair ###############
if [ "$FORM_release_dhcp" != "" ]; then
    #dhcp_release <interface> <address> <MAC address> <client_id>
    dhcp_release $FORM_release_dhcp 
    logger "Network DHCP  $FORM_release_dhcp is released."
fi


#remove static address
if [ "$FORM_remove_static" != "" ]; then
    uci_remove "dhcp" "$FORM_remove_static"
    unset host_cfgs
fi

if ! empty "$FORM_add_item"; then
    un_names=$(cat /var/run/dhcp_staticip_name)
    for chname in $un_names; do
        if [ "$chname" = "$FORM_static_name" ]; then
            append validate_error "Error in Static DHCP Name: Static IP Name not unique" "$N"
            break
        fi
    done

    un_ip=$(cat /var/run/dhcp_staticip_ip)
    for chip in $un_ip; do
        if [ "$chip" = "$FORM_static_ip_addr" ]; then
            append validate_error "Error in Static DHCP IP Address: duplicate IP address $chip" "$N"
            break
        fi
    done

validate <<EOF
string|FORM_static_name|@TR<<Static IP Name>>|required|$FORM_static_name
mac|FORM_static_mac_addr|@TR<<Static Mac Address>>|required|$FORM_static_mac_addr
ip|FORM_static_ip_addr|@TR<<Static IP Address>>|required|$FORM_static_ip_addr
EOF
    [ "$?" = "0" -a "$validate_error" = "" ] && {
        uci_add "dhcp" "host" "$FORM_static_name"
        uci_set "dhcp" "$CONFIG_SECTION" "name" "$FORM_static_name"
        uci_set "dhcp" "$CONFIG_SECTION" "mac" "$FORM_static_mac_addr"
        uci_set "dhcp" "$CONFIG_SECTION" "ip" "$FORM_static_ip_addr"
        unset FORM_static_mac_addr FORM_static_ip_addr host_cfgs dnsmasq_cfgs dnsmasq_cfgs FORM_static_name 
    } 
    #unset FORM_static_mac_addr FORM_static_ip_addr host_cfgs dnsmasq_cfgs dnsmasq_cfgs FORM_static_name 
    append  validate_error "$ERROR" "$N"
fi

### Remove a network interface ###
if [ "$FORM_remove_network_if" != "" ]; then
    # Check if there're any wireless interfaces in this network interface.
    uci_load wireless
    for wifi_vif in $vfaces; do
        config_get network_if $wifi_vif network
        network_ifs=${network_ifs}${network_if}
    done
    if [ `echo $network_ifs | grep $FORM_remove_network_if` ]; then
        append validate_error "Can't remove $FORM_remove_network_if. It's being used." "$N"
    else
    {
        uci_remove "network" "$FORM_remove_network_if"
        uci_remove "dhcp" "$FORM_remove_network_if"
        uci_remove "firewall" "$FORM_remove_network_if"
        uci_remove "firewall" "forward_""$FORM_remove_network_if""_wan"
        uci_remove "firewall" "forward_""$FORM_remove_network_if""_wan2"
        ifconfig "br-""$FORM_remove_network_if" down
        brctl delbr "br-""$FORM_remove_network_if"
        unset network_if_cfgs
        logger "Network interface $FORM_remove_network_if is removed."
    }
    fi
fi

### Add a network interface
if ! empty "$FORM_add_network_if"; then

validate <<EOF
ip|FORM_${interface}_new_if_ipaddr|@TR<<LAN IP Address>>|required|$FORM_new_if_ipaddr
netmask|FORM_${interface}_new_if_netmask|@TR<<LAN Netmask>>|required|$FORM_new_if_netmask
ip|FORM_${interface}_new_if_gateway|@TR<<LAN Default Gateway>>||$FORM_new_if_gateway
EOF
    [ "$?" = "0" -a "$validate_error" = "" ] && {
        uci_add "network" "interface" "$FORM_new_if_ifname"
	uci_set "network" "$CONFIG_SECTION" "ifname" "fake-dev"
	uci_set "network" "$CONFIG_SECTION" "type" "bridge"
        uci_set "network" "$CONFIG_SECTION" "proto" "static"
        uci_set "network" "$CONFIG_SECTION" "netmask" "$FORM_new_if_netmask"
        uci_set "network" "$CONFIG_SECTION" "ipaddr" "$FORM_new_if_ipaddr"
        logger "New network interface $FORM_new_if_ifname was added."

	# Add DHCP service on this interface
        uci_add "dhcp" "dhcp" "$FORM_new_if_ifname"
        uci_set "dhcp" "$CONFIG_SECTION" "interface" "$FORM_new_if_ifname"
        uci_set "dhcp" "$CONFIG_SECTION" "start" "$FORM_new_if_ipaddr"
        uci_set "dhcp" "$CONFIG_SECTION" "limit" "100"
        uci_set "dhcp" "$CONFIG_SECTION" "leasetime" "720m"
       	uci_set "dhcp" "$CONFIG_SECTION" "ignore" "0"

	# Open firewall for this interface
        uci_add "firewall" "zone" "$FORM_new_if_ifname"
        uci_set "firewall" "$FORM_new_if_ifname" "name" "$FORM_new_if_ifname"
        uci_set "firewall" "$FORM_new_if_ifname" "input" "ACCEPT"
        uci_set "firewall" "$FORM_new_if_ifname" "output" "ACCEPT"
        uci_set "firewall" "$FORM_new_if_ifname" "forward" "DROP"

        uci_add "firewall" "forwarding" "forward_""$FORM_new_if_ifname""_wan"
        uci_set "firewall" "$CONFIG_SECTION" "src" "$FORM_new_if_ifname"
        uci_set "firewall" "$CONFIG_SECTION" "dest" "wan"

        uci_add "firewall" "forwarding" "forward_""$FORM_new_if_ifname""_wan2"
        uci_set "firewall" "$CONFIG_SECTION" "src" "$FORM_new_if_ifname"
        uci_set "firewall" "$CONFIG_SECTION" "dest" "wan2"
        logger "New network interface $FORM_new_if_ifname firewall is setup."

        unset FFORM_new_if_ifname FORM_new_if_netmask FORM_new_if_proto FORM_new_if_ipaddr network_if_cfgs FORM_add_network_if 
    }
    append  validate_error "$ERROR" "$N"
fi

## Network LAN Setting ############################################
uci_load network
interface="lan"

if empty "$FORM_add_network_if"; then
if empty "$FORM_submit"; then
    config_get FORM_proto $interface proto
    config_get FORM_ipaddr $interface ipaddr
    config_get FORM_netmask $interface netmask
    config_get FORM_gateway $interface gateway
    config_get FORM_dns $interface dns
    config_get FORM_stp $interface stp
    eval FORM_dnsremove="\$FORM_${interface}_dnsremove"
    if [ "$FORM_dnsremove" != "" ]; then
        list_remove FORM_dns "$FORM_dnsremove"
        uci_set "network" "$interface" "dns" "$FORM_dns"
    fi
else
    eval FORM_proto="\$FORM_${interface}_proto"
    eval FORM_ipaddr="\$FORM_${interface}_ipaddr"
    eval FORM_netmask="\$FORM_${interface}_netmask"
    eval FORM_gateway="\$FORM_${interface}_gateway"
    eval FORM_dnsadd="\$FORM_${interface}_dnsadd"
    eval FORM_stp="\$FORM_${interface}_stp"
    config_get FORM_dns $interface dns
    [ $FORM_dnsadd != "" ] && FORM_dns="$FORM_dns $FORM_dnsadd"
    if [ "$FORM_defaultroute" = "" ]; then
        FORM_defaultroute=0
    fi
fi
fi

cfgname="LAN"

network_options="start_form|$cfgname @TR<<Configuration>>

	field|@TR<<Spanning Tree (STP)>>
	select|${interface}_stp|$FORM_stp
	option|on|@TR<<On>>
	option|off|@TR<<Off>>

	field|@TR<<Connection Type>>
	select|${interface}_proto|$FORM_proto
	option|static|@TR<<Static IP>>
	option|dhcp|@TR<<DHCP>>

	field|@TR<<IP Address>>|field_${interface}_ipaddr|
	text|${interface}_ipaddr|$FORM_ipaddr

	field|@TR<<Netmask>>|field_${interface}_netmask|
	text|${interface}_netmask|$FORM_netmask

	field|@TR<<Default Gateway>>|field_${interface}_gateway|
	text|${interface}_gateway|$FORM_gateway

	end_form

	start_form|$cfgname @TR<<DNS Servers>>|field_${interface}_dns|hidden
	listedit|${interface}_dns|$SCRIPT_NAME?${interface}_proto=static&amp;|$FORM_dns
	end_form"

append forms "$network_options" "$N"

## Add new network interface setting ############################################

network_if_options="start_form|$cfgname @TR<<Add New Interface>>
	field|@TR<<Interface Name>>|field_${interface}_new_if_ifname|
	text|new_if_ifname|$FORM_new_if_ifname

	field|@TR<<IP Address>>|field_${interface}_new_if_ipaddr|
	text|new_if_ipaddr|$FORM_new_if_ipaddr

	field|@TR<<Netmask>>|field_${interface}_new_if_netmask|
	text|new_if_netmask|$FORM_new_if_netmask

	field|@TR<<Default Gateway>>|field_${interface}_new_if_gateway|
	text|new_if_gateway|$FORM_new_if_gateway

	field|
	submit|add_network_if|@TR<<Add Network Interface>>

	end_form"

append forms "$network_if_options" "$N"

### DHCP Setting ###############################################
config=lan
uci_load dhcp
if [ "$FORM_proto" = "static" ]; then
    brlan_ip="$FORM_ipaddr"
    brlan_mask="$FORM_netmask"
else
    brlan_ip="$(ifconfig br-lan | grep inet | grep -v inet6 | awk -F'[: ]+' '{print $4}')"
    brlan_mask="$(ifconfig br-lan | grep inet |grep -v inet6| awk -F'[: ]+' '{print $8}')"
fi


if empty "$FORM_add_network_if"; then
if empty "$FORM_submit"; then
    config_get FORM_dstart $config start 
    config_get FORM_limit $config limit
    config_get FORM_leasetime $config leasetime
    config_get_bool FORM_ignore $config ignore 0
    if [ -z "$(echo $FORM_dstart | grep '\.')" ]; then
        eval "$(netclass.sh S $brlan_ip $brlan_mask $FORM_dstart)"
    fi   
    config_get FORM_preferred_dns $config preferred_dns
    config_get FORM_alternate_dns $config alternate_dns
    config_get FORM_alternate_gateway $config alternate_gateway
    config_get FORM_domain_name $main_dnsmasq domain_name lan
    config_get FORM_wins_server $config wins_server 
    config_get FORM_wins_node_type $config wins_node_type
else
    eval FORM_dstart="\$FORM_dstart_$config"
    eval FORM_limit="\$FORM_limit_$config"
    eval FORM_leasetime="\$FORM_leasetime_$config"
    eval FORM_ignore="\$FORM_ignore_$config"
    eval FORM_preferred_dns="\$FORM_preferred_dns_$config"
    eval FORM_alternate_dns="\$FORM_alternate_dns_$config"
    eval FORM_alternate_gateway="\$FORM_alternate_gateway_$config"
    eval FORM_domain_name="\$FORM_domain_name_$config"
    eval FORM_wins_server="\$FORM_wins_server_$config"
    eval FORM_wins_node_type="\$FORM_wins_node_type_$config"
fi
fi

convert leasetime to minutes
lease_int=$(echo "$FORM_leasetime" | tr -d [a-z][A-Z])			
time_units=$(echo "$FORM_leasetime" | tr -d [0-9])
time_units=${time_units:-m}

case "$time_units" in
        "h" | "H" ) let "FORM_leasetime=$lease_int*60";;
        "d" | "D" ) let "FORM_leasetime=$lease_int*24*60";;
        "s" | "S" ) let "FORM_leasetime=$lease_int/60";;
        "w" | "W" ) let "FORM_leasetime=$lease_int*7*24*60";;
        "m" | "M" ) FORM_leasetime="$lease_int";;  # minutes 			
#        *) FORM_leasetime="$lease_int"; echo "<br />WARNING: Unknown suffix found on dhcp lease time: $leasetime";;
        *) FORM_leasetime="$lease_int"; append  validate_error "WARNING: Unknown suffix found on dhcp lease time: $FORM_leasetime";;
esac

if empty "$FORM_add_network_if"; then
if ! empty "$FORM_submit"; then
    if [ "$FORM_ignore" = "0" ]; then
validate <<EOF
int|FORM_leasetime_$config|@TR<<LAN DHCP Lease Time>>|min=1 max=2147483647|$FORM_leasetime
int|FORM_limit_$config|@TR<<LAN DHCP Limit>>|min=0 max=16777214|$FORM_limit
ip|FORM_dstart_$config|@TR<<LAN DHCP Start>>||$FORM_dstart
ip|FORM_preferred_dns_$config|@TR<<LAN Preferred DNS SERVER>>||$FORM_preferred_dns
ip|FORM_alternate_dns_$config|@TR<<LAN Alternate DNS SERVER>>||$FORM_alternate_dns
ip|FORM_alternate_gateway_$config|@TR<<Alternate Gateway>>||$FORM_alternate_gateway
EOF
        append validate_error "$ERROR" "$N"
    
        eval "$(netclass.sh E $brlan_ip $brlan_mask $FORM_dstart $FORM_limit)"
        if ! empty "${ERRORLim}" ;then
            append validate_error "${ERRORLim}" "$N"
            append validate_error "<br />" "$N"
        fi
        if ! empty "${ERRORST}" ;then
            append validate_error "${ERRORST}" "$N"
            append validate_error "<br />" "$N"
        fi
        
        #added preferred dns for customer
        #first need know if the dnspassthrough have been set on carrier page. 
        #do nothing when carrier dns passthrough have been set.
        local dnspassthrough=$(uci get lte.connect.dnspassthrough)
        if [ "$dnspassthrough" = "1" ];then
            [ -n "${FORM_preferred_dns}${FORM_alternate_dns}" ] && {
               append validate_error "Error in DHCP PREFERRED DNS: carrier dns passthrough is on" "$N"
               append validate_error "<br />" "$N"
            }
        fi
    fi

    if [ "$FORM_proto" = "static" ];then
validate <<EOF
ip|FORM_${interface}_ipaddr|@TR<<LAN IP Address>>||$FORM_ipaddr
netmask|FORM_${interface}_netmask|@TR<<LAN Netmask>>||$FORM_netmask
ip|FORM_${interface}_gateway|@TR<<LAN Default Gateway>>||$FORM_gateway
ip|FORM_dnsadd|@TR<<DNS Address>>||$FORM_dnsadd
EOF
        append validate_error "$ERROR" "$N"
    fi

    empty "$validate_error" && {
        uci_set "dhcp" "$config" "ignore" "$FORM_ignore"
        uci_set "network" "$interface" "proto" "$FORM_proto"

        uci_set "network" "$interface" "ipaddr" "$FORM_ipaddr"
        uci_set "network" "$interface" "netmask" "$FORM_netmask"
        uci_set "network" "$interface" "gateway" "$FORM_gateway"
        uci_set "network" "$interface" "dns" "$FORM_dns"
        uci_set "network" "$interface" "stp" "$FORM_stp"
        
        uci_set "dhcp" "$config" "start" "$FORM_dstart"
        uci_set "dhcp" "$config" "limit" "$FORM_limit"
        uci_set "dhcp" "$config" "preferred_dns" "$FORM_preferred_dns"
        uci_set "dhcp" "$config" "alternate_dns" "$FORM_alternate_dns"
        uci_set "dhcp" "$config" "alternate_gateway" "$FORM_alternate_gateway"
        uci_set "dhcp" "$main_dnsmasq" "domain" "$FORM_domain_name"
        uci_set "dhcp" "$config" "wins_server" "$FORM_wins_server"
        uci_set "dhcp" "$config" "wins_node_type" "$FORM_wins_node_type"
        #if no error here will set option here
        #uci_remove "dhcp" "lan" "dhcp_option" 
        #[ -n "${FORM_preferred_dns}${FORM_alternate_dns}" ] && uci add_list dhcp.lan.dhcp_option="6,$FORM_preferred_dns,$FORM_alternate_dns"
        #[ -n "${FORM_alternate_gateway}" ] && uci add_list dhcp.lan.dhcp_option="3,$FORM_alternate_gateway"
        #[ -n "${FORM_wins_server}" ] && uci add_list dhcp.lan.dhcp_option="44,$FORM_wins_server"
        #[ -n "${FORM_wins_node_type}" ] && uci add_list dhcp.lan.dhcp_option="46,$FORM_wins_node_type"
        uci_set "dhcp" "$config" "leasetime" "${FORM_leasetime}m"
        uci_set "dhcp" "$config" "ignore" "$FORM_ignore"
    }
fi
fi

form_dhcp="start_form|$cfgname DHCP

    field|@TR<<DHCP>>
    select|ignore_$config|$FORM_ignore
    option|0|@TR<<Enable>>
    option|1|@TR<<Disable>>

    field|@TR<<Start>>|field_start_dhcp|hidden
    text|dstart_${config}|$FORM_dstart

    field|@TR<<Limit>>|field_limit_dhcp|hidden
    text|limit_$config|$FORM_limit

    field|@TR<<Lease Time (in minutes)>>|field_leasetime_dhcp|hidden
    text|leasetime_$config|$FORM_leasetime

    field|@TR<<Alternate Gateway>>|field_alternate_gateway|hidden
    text|alternate_gateway_$config|$FORM_alternate_gateway

    field|@TR<<Preferred DNS server>>|field_preferred_dns|hidden
    text|preferred_dns_$config|$FORM_preferred_dns

    field|@TR<<Alternate DNS server>>|field_alternate_dns|hidden
    text|alternate_dns_$config|$FORM_alternate_dns

    field|@TR<<Domain Name>>|field_domain_name|hidden
    text|domain_name_$config|$FORM_domain_name

    field|@TR<<WINS/NBNS Servers>>|field_wins_server|hidden
    text|wins_server_$config|$FORM_wins_server

    field|@TR<<WINS/NBT Node Type>>|field_wins_node_type|hidden
    select|wins_node_type_$config|$FORM_wins_node_type
    option||@TR<<none>>
    option|1|@TR<<b-node>>
    option|2|@TR<<p-node>>
    option|4|@TR<<m-node>>
    option|8|@TR<<h-node>>

    end_form
    "
append forms "$form_dhcp" "$N"

### Static DHCP Setting ###############################################

form_static="
    start_form|@TR<<network_hosts_DHCP_Static_IPs#Static IP addresses (for DHCP)>>
    field|@TR<<Name>>
    text|static_name|$FORM_static_name
    field|@TR<<MAC Address>>
    text|static_mac_addr|$FORM_static_mac_addr
    field|@TR<<IP Address>>
    text|static_ip_addr|$FORM_static_ip_addr
    field|
    submit|add_item|@TR<<Add static IP>>
    end_form
    "
append forms "$form_static" "$N"


###################################################################
# set JavaScript
javascript_forms="
        v = (isset('${interface}_proto', 'static'));
        set_visible('field_${interface}_ipaddr', v);
        set_visible('field_${interface}_netmask', v);
        set_visible('field_${interface}_gateway', v);
        set_visible('field_${interface}_dns', v);
        m = (isset('ignore_$config','0'));
        set_visible('field_start_dhcp', m);
        set_visible('field_limit_dhcp', m);
        set_visible('field_alternate_gateway', m);
        set_visible('field_preferred_dns', m);
        set_visible('field_alternate_dns', m);
        set_visible('field_leasetime_dhcp', m);
        set_visible('field_domain_name', m);
        set_visible('field_wins_server', m);
        set_visible('field_wins_node_type', m);
        "
append js "$javascript_forms" "$N"



ERROR=$validate_error

header "Network" "LAN" "@TR<<Network LAN Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"

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
$forms
EOF

cat <<EOF
<hr class="separator" />
<h5><strong>@TR<<network_dhcp_static_addresses#Static Addresses>></strong></h5>
<table style="width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<th width="25%">@TR<<network_hosts_MAC#MAC Address>></th>
	<th width="25%">@TR<<network_hosts_IP#IP Address>></th>
	<th width="25%">@TR<<network_hosts_Name#Name>></th>
	<th>@TR<<NetStatus>></th>
</tr>
EOF

odd=0

uniq_names=""
rm -f /var/run/dhcp_staticip_name 
uniq_ip=""
rm -f /var/run/dhcp_staticip_ip 

for hconf in $host_cfgs; do
	config_get mac $hconf mac
	config_get ip $hconf ip
	config_get name $hconf name
        append uniq_names "$name" "$N"
        append uniq_ip "$ip" "$N"

	if [ "$odd" = 0 ]; then
		echo "<tr>"
		let odd+=1
	else
		echo "<tr class=\"odd\">"
		let odd-=1
	fi
	echo "<td width=\"25%\">${mac}</td>"
	echo "<td width=\"25%\">${ip}</td>"
	echo "<td width=\"25%\">${name}</td>"
	echo "<td><a href=\"$SCRIPT_NAME?remove_static=${hconf}\">@TR<<Remove>> ${name}</a></td></tr>"
done
echo "$uniq_names" > /var/run/dhcp_staticip_name 
echo "$uniq_ip" > /var/run/dhcp_staticip_ip 
echo "</table><br />"

cat <<EOF
<hr class="separator" />
<h5><strong>@TR<<network_hosts_Active_Leases#Active DHCP Leases>></strong></h5>
<table style="width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<th width="25%">@TR<<network_hosts_MAC#MAC Address>></th>
	<th width="25%">@TR<<network_hosts_IP#IP Address>></th>
	<th width="25%">@TR<<network_hosts_Name#Name>></th>
	<th>@TR<<network_hosts_Expires#Expires in>></th>
</tr>
EOF

cat /tmp/dhcp.leases 2>/dev/null | awk -v date="$(date +%s)" '
BEGIN {
	odd=1
	counter = 0
}
$1 > 0 {
	counter++
	if (odd == 1)
	{
		print "	<tr>"
		odd--
	} else {
		print "	<tr class=\"odd\">"
		odd++
	}
	print "		<td width=\"25%\">" $2 "</td>"
	print "		<td width=\"25%\">" $3 "</td>"
	print "		<td width=\"25%\">" $4 "</td>"
	print "		<td>"
	t = $1 - date
	h = int(t / 60 / 60)
	if (h > 0) printf h "@TR<<network_hosts_h#h>> "
	m = int(t / 60 % 60)
	if (m > 0) printf m "@TR<<network_hosts_min#min>> "
	s = int(t % 60)
	printf s "@TR<<network_hosts_sec#sec>> "
	print "		</td>"
        print "<td><a href=\"/cgi-bin/webif/network-lan.sh?release_dhcp=br-lan " $3 " " $2 " \">@TR<<Release>> </a></td>"
	print "	</tr>"
}
END {
	if (counter == 0) {
		print "	<tr>"
		print "		<td colspan=\"4\">@TR<<network_hosts_No_leases#There are no known DHCP leases.>></td>"
		print "	</tr>"
	}
	print "</table>"
}'

display_form <<EOF
start_form|@TR<<>>
submit|releaseall_button|@TR<<   Release All   >>
submit|refresh_button|@TR<<   Refresh   >>
end_form
EOF

###### Network Interface list start ######
cat <<EOF
<hr class="separator" />
<h5><strong>@TR<<network_interfaces#Network Interfaces>></strong></h5>
<table style="width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<th width="20%">@TR<<network_interfaces_name#Interface Name>></th>
	<th width="20%">@TR<<network_interfaces_ipaddr#Protocol>></th>
	<th width="20%">@TR<<network_interfaces_ipaddr#IP Address>></th>
	<th width="20%">@TR<<network_interfaces_netmask#Net Mask>></th>
	<th>@TR<<Interface Status>></th>
</tr>
EOF
 
odd = 0
 
for cfgsec in $network_if_cfgs; do
      [ "$cfgsec" = "loopback" -o "$cfgsec" = "wan" -o "$cfgsec" = "wan2" -o "$cfgsec" = "lan" ] || {
	 config_get ifname "$cfgsec" ifname 
	 config_get ipaddr "$cfgsec" ipaddr
	 config_get netmask "$cfgsec" netmask
	 config_get proto "$cfgsec" proto
         [ -z $ipaddr ] && ipaddr="N/A"
         [ -z $netmask ] && netmask="N/A"
    
	if [ "$odd" = 0 ]; then
		echo "<tr>"
		let odd+=1
	else
		echo "<tr class=\"odd\">"
		let odd-=1
	fi
    
cat <<EOF
	<td width="20%">$cfgsec</td>
	<td width="20%">$proto</td>
	<td width="20%">$ipaddr</td>
	<td width="20%">$netmask</td>
	<td><a href="$SCRIPT_NAME?remove_network_if=${cfgsec}">@TR<<Remove>> ${cfgsec}</a></td></tr>
EOF
}
done
echo "</table><br />"
###### Network Interface list end #######

footer ?>
<!--
##WEBIF:name:Network:101:LAN
-->
