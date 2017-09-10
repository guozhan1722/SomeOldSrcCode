#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
###################################################################
# DHCP configuration
#
# Description:
#	DHCP configuration.
#
# Author(s) [in order of work date]:
#	Travis Kemen	<thepeople@users.berlios.de>
#	Adam Hill	<adam@voip-x.co.uk>
# Major revisions:
#	Allow DHCP options to be specified ( Adam H )
#
# UCI variables referenced:
#
# Configuration files referenced:
#   dhcp, network
#


###################################################################
# Parse Settings, this function is called when doing a config_load
config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		interface)
			if [ "$cfg_name" != "loopback" ]; then
				append networks "$cfg_name" "$N"
			fi
		;;
		dhcp)
			append dhcp_cfgs "$cfg_name" "$N"
		;;
		dnsmasq)
			append dnsmasq_cfgs "$cfg_name" "$N"
		;;
		host)
			append host_cfgs "$cfg_name" "$N"
		;;
	esac
}
uci_load network
uci_load dhcp
# create dnsmasq's section when missing
[ -z "$dnsmasq_cfgs" ] && {
	uci_add dhcp dnsmasq
	unset dhcp_cfgs dnsmasq_cfgs
	uci_load dhcp
}

vcfg_number=$(echo "$dhcp_cfgs" "$dnsmasq_cfgs" |wc -l)
let "vcfg_number+=1"

#add dhcp network
if [ "$FORM_add_dhcp" != "" ]; then
	uci_add "dhcp" "dhcp" ""
	uci_set "dhcp" "$CONFIG_SECTION" "interface" "$FORM_network_add"
	unset host_cfgs dnsmasq_cfgs dnsmasq_cfgs
	uci_load dhcp
	let "vcfg_number+=1"
fi

#remove static address
if [ "$FORM_remove_static" != "" ]; then
	uci_remove "dhcp" "$FORM_remove_static"
	unset host_cfgs dnsmasq_cfgs dnsmasq_cfgs
	uci_load dhcp
fi

added_error=""
#add static address
if ! empty "$FORM_submit"; then
    if [ -n "$FORM_static_name" -a -n "$FORM_static_mac_addr" -a -n "FORM_static_ip_addr" ]; then
        un_names=$(cat /var/run/dhcp_staticip_name)
        for chname in $un_names; do
            if [ "$chname" = "$FORM_static_name" ]; then
                append added_error "Error in Name: Static IP Name not unique" "$N"
                FORM_static_name=""
                break
            fi
        done
validate <<EOF
string|FORM_static_name|@TR<<Static IP Name>>|required|$FORM_static_name
mac|FORM_static_mac_addr|@TR<<Static Mac Address>>||$FORM_static_mac_addr
ip|FORM_static_ip_addr|@TR<<Static IP Address>>||$FORM_static_ip_addr
EOF
        [ "$?" = "0" -a "$added_error" = "" ] && {
            uci_add "dhcp" "host" ""
            uci_set "dhcp" "$CONFIG_SECTION" "name" "$FORM_static_name"
            uci_set "dhcp" "$CONFIG_SECTION" "mac" "$FORM_static_mac_addr"
            uci_set "dhcp" "$CONFIG_SECTION" "ip" "$FORM_static_ip_addr"
            unset FORM_static_mac_addr FORM_static_ip_addr host_cfgs dnsmasq_cfgs dnsmasq_cfgs FORM_static_name 
            uci_load dhcp
	} 
        append  added_error "$ERROR" "$N"
    fi
fi

dnsmasq_cfgs=$(echo "$dnsmasq_cfgs" |uniq)
dhcp_cfgs=$(echo "$dhcp_cfgs" |uniq)


#dhcp_option_select=$(awk -F: '{ print "option|" $1 "|" $2 }' /usr/lib/webif/dhcp_options.dat)
config="lan"
#for config in $dhcp_cfgs; do
	count=1
        config_get netmask $config netmask

	if [ "$FORM_submit" = "" ]; then
		config_get interface $config interface
		config_get start $config start
		config_get limit $config limit
		config_get leasetime $config leasetime
		config_get_bool ignore $config ignore 0
               
                start0=$(echo $start |awk -F '.' '{print $1}')
                start1=$(echo $start |awk -F '.' '{print $2}')
                start2=$(echo $start |awk -F '.' '{print $3}')
                start3=$(echo $start |awk -F '.' '{print $4}')
	else
		config_get interface $config interface
		eval start0="\$FORM_start0_$config"
		eval start1="\$FORM_start1_$config"
		eval start2="\$FORM_start2_$config"
		eval start3="\$FORM_start3_$config"
		eval limit="\$FORM_limit_$config"
                eval start="${start0}.${start1}.${start2}.${start3}"

		eval leasetime="\$FORM_leasetime_$config"
		eval ignore="\$FORM_ignore_$config"

         	#convert leasetime to minutes
        	lease_int=$(echo "$leasetime" | tr -d [a-z][A-Z])			
        	time_units=$(echo "$leasetime" | tr -d [0-9])
        	time_units=${time_units:-m}
        	case "$time_units" in
        		"h" | "H" ) let "leasetime=$lease_int*60";;
        		"d" | "D" ) let "leasetime=$lease_int*24*60";;
        		"s" | "S" ) let "leasetime=$lease_int/60";;
        		"w" | "W" ) let "leasetime=$lease_int*7*24*60";;
        		"m" | "M" ) leasetime="$lease_int";;  # minutes 			
        		*) leasetime="$lease_int"; echo "<br />WARNING: Unknown suffix found on dhcp lease time: $leasetime";;
        	esac

validate <<EOF
int|leasetime_$config|@TR<<DHCP Lease Time>>|min=1 max=2147483647|$leasetime
int|start0_$config|@TR<<Start IP ADDR field 1>>|min=0 max=255|$start0
int|start1_$config|@TR<<Start IP ADDR field 2>>|min=0 max=255|$start1
int|start2_$config|@TR<<Start IP ADDR field 3>>|min=0 max=255|$start2
int|start3_$config|@TR<<Start IP ADDR field 4>>|min=0 max=255|$start3
EOF
                append added_error "$ERROR" "$N"
                logger "added111 erro is $added_error"
                brlan_ip="$(ifconfig br-lan | grep inet | grep -v inet6 | awk -F'[: ]+' '{print $4}')"
                eval "$(netclass.sh E $brlan_ip $netmask $start $limit)"
                logger "netclass  E $brlan_ip $netmask $start $limit"
                logger "netclass  error lim is $ERRORLim and error st is $ERRORST"
                append added_error "${ERRORLim}" "$N"
                append added_error "${ERRORST}" "$N"
               
                logger "added erro is $added_error"
                empty "$added_error" && {
                    uci_set "dhcp" "$config" "start" "$start"
                    uci_set "dhcp" "$config" "limit" "$limit"
                    uci_set "dhcp" "$config" "leasetime" "$leasetime"
                    uci_set "dhcp" "$config" "ignore" "$ignore"
                }
	fi
        

	#Save networks with a dhcp interface.
	append dhcp_networks "$interface" "$N"


        if [ "$interface" = "lan" ];then
            ifacename="LAN"
        elif [ "$interface" = "wan" ];then
            ifacename="WAN"
        fi
get_netclass()
{
    eval "$(netclass.sh C $netmask)"
    if [ "$NetType" = "C" ];then
        cal_start="dtext|start0_$config|$start0
            string|.
            dtext|start1_$config|$start1
            string|.
            dtext|start2_$config|$start2
            string|.
            stext|start3_$config|$start3"
    elif [ "$NetType" = "B" ];then
        cal_start=" dtext|start0_$config|$start0
            string|.
            dtext|start1_$config|$start1
            string|.
            stext|start2_$config|$start2
            string|.
            stext|start3_$config|$start3"
    elif [ "$NetType" = "A" ];then
        cal_start=" dtext|start0_$config|$start0
            string|.
            stext|start1_$config|$start1
            string|.
            stext|start2_$config|$start2
            string|.
            stext|start3_$config|$start3"
    else
        cal_start=" stext|start0_$config|$start0
                string|.
                stext|start1_$config|$start1
                string|.
                stext|start2_$config|$start2
                string|.
                stext|start3_$config|$start3"
    fi
}

	form_dhcp="start_form|$ifacename DHCP
		field|@TR<<DHCP>>
		radio|ignore_$config|$ignore|0|@TR<<On>>
		radio|ignore_$config|$ignore|1|@TR<<Off>>"
            
        get_netclass
        mid_form_dhcp="field|@TR<<Start>>
                $cal_start
                "
	append form_dhcp "$mid_form_dhcp" "$N"

	last_form_dhcp="field|@TR<<Limit>>
                text|limit_$config|$limit
                field|@TR<<Lease Time (in minutes)>>
		text|leasetime_$config|$leasetime
                "
	
	append form_dhcp "$last_form_dhcp" "$N"
	
	append form_dhcp "end_form" "$N"

	append forms "$form_dhcp" "$N"

#done
ERROR=$added_error
		
header "Network" "DHCP" "@TR<<DHCP Configuration>>" 'onload="modechange()"' "$SCRIPT_NAME"

#####################################################################
# modechange script
#
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

display_form <<EOF
start_form|@TR<<network_hosts_DHCP_Static_IPs#Static IP addresses (for DHCP)>>
field|@TR<<Name>>
text|static_name|$FORM_static_name
field|@TR<<MAC Address>>
text|static_mac_addr|$FORM_static_mac_addr
field|@TR<<IP Address>>
text|static_ip_addr|$FORM_static_ip_addr
end_form
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

for config in $host_cfgs; do
	config_get mac $config mac
	config_get ip $config ip
	config_get name $config name
        append uniq_names "$name" "$N"

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
	echo "<td><a href=\"$SCRIPT_NAME?remove_static=${config}\">@TR<<Remove>> ${name}</a></td></tr>"
done
echo "$uniq_names" > /var/run/dhcp_staticip_name 
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

footer ?>
#<!--
###WEBIF:name:Network:425:DHCP
#-->
