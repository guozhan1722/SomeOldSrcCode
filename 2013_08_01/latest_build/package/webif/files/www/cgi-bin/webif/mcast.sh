#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		mcast)
			append mcast_cfgs "$cfg_name" "$N"
		;;
		mcontrol)
			append mcontrol_cfgs "$cfg_name" "$N"
		;;
		mrate)
			append mrate_cfgs "$cfg_name" "$N"
		;;

		interface)
			if [ "$cfg_name" != "loopback" ]; then
				append networks "option|$cfg_name" "$N"
                                append netmode "$cfg_name" "$N"
			fi
                ;;
                wifi-iface)
                                append vface "$cfg_name" "$N"
		;;
	esac
}

get_wireless_iface()
{
    wiface=""
    for dev in $(ls /sys/class/ieee80211); do
        devidx=$(echo $dev|cut -c 4)
        append wiface "wlan$devidx" "$N"
    done
}

#remove rule
if ! empty "$FORM_remove_vcfg"; then
	uci_remove "mcast" "$FORM_remove_vcfg"
fi

if ! empty "$FORM_add_mcast"; then
    #[ -z "$FORM_name_rule" ] && append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: Rule Name not unique</h3></td></tr>" "$N"

    un_names=$(cat /var/run/mcast_mcast_name)
    for chname in $un_names; do
        if [ "$chname" = "mcast_$FORM_name_mcast" ]; then
            append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: Mcast Name not unique</h3></td></tr>" "$N"
            break
        fi
    done

validate <<EOF
ip|FORM_localip_mcast|@TR<<Local IP Address>>||$FORM_localip_mcast
ip|FORM_remoteip_mcast|@TR<<Remote IP Address>>||$FORM_remoteip_mcast
ip|FORM_srcip_mcast|@TR<<Source IP Address>>||$FORM_srcip_mcast
netmask|FORM_srcmask_mcast|@TR<<Source Netmask>>||$FORM_srcmask_mcast
ip|FORM_grpip_mcast|@TR<<Group IP Address>>||$FORM_grpip_mcast
EOF
	equal "$?" 0 && {
		uci_add mcast mcast "mcast_$FORM_name_mcast"; add_mcast_cfg="$CONFIG_SECTION"

                uci_set mcast "$add_mcast_cfg" "tunnel" "no"
                uci_set mcast "$add_mcast_cfg" "localip" "$FORM_localip_mcast"
                uci_set mcast "$add_mcast_cfg" "remoteip" "$FORM_remoteip_mcast"
                uci_set mcast "$add_mcast_cfg" "srcip" "$FORM_srcip_mcast"
                uci_set mcast "$add_mcast_cfg" "srcmask" "$FORM_srcmask_mcast"
                uci_set mcast "$add_mcast_cfg" "grpip" "$FORM_grpip_mcast"
		unset FORM_tunnel_mcast FORM_localip_mcast FORM_remoteip_mcast FORM_srcip_mcast FORM_srcmask_mcast FORM_grpip_mcast
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

uci_load mcast

if [ "$FORM_submit" = "" ]; then
    config_get FORM_enable "$mcontrol_cfgs" enable
else
    FORM_enable="$FORM_enable_000"
    uci_set mcast "$mcontrol_cfgs" enable "$FORM_enable"
fi

form="start_form|Multicast Mode
        field|@TR<<Mode>>
        select|enable_000|$FORM_enable
        option|1|Enable
        option|0|Disable
        end_form"
append forms "$form" "$N"

cnt=0
get_wireless_iface
for wf in $wiface; do
    wf_idx=$(echo $wf|cut -c 5-)
    let "wf_d=wf_idx+1"

    found=0
    for find in $mrate_cfgs; do
        config_get FORM_iface "$find" iface
        if [ "$FORM_iface" = "$wf" ]; then
            found=1
            if [ "$FORM_submit" = "" ]; then
                config_get FORM_rate "$find" rate
            else
                eval FORM_rate="\$FORM_rate_$cnt"
                uci_set mcast "$find" rate "$FORM_rate"
            fi
            wform="start_form|Wireless Interface Radio$wf_d rate Configuration
                    field|@TR<<Rate>>
                    select|rate_$cnt|$FORM_rate
                    option|0|@TR<<6M>>
                    option|1|@TR<<9M>>
                    option|2|@TR<<12M>>
                    option|3|@TR<<18M>>
                    option|4|@TR<<24M>>
                    option|5|@TR<<36M>>
                    option|6|@TR<<48M>>
                    option|7|@TR<<54M>>
                    end_form"
            append wforms "$wform" "$N"
            break        
        fi            
    done

    if [ "$found" = "0" ]; then
         if [ "$FORM_submit" = "" ]; then
            FORM_rate=0
            FORM_iface="$wf"
         else
             uci_add mcast mrate mrate_$wf_idx ; add_mrate_cfg="$CONFIG_SECTION"
             eval FORM_rate="\$FORM_rate_$cnt"
             uci_set mcast "$add_mrate_cfg" rate "$FORM_rate"
             uci_set mcast "$add_mrate_cfg" iface "$wf"
         fi
         wform="start_form|Wireless Interface Radio$wf_d rate Configuration
            field|@TR<<Rate>>
            select|rate_$cnt|$FORM_rate
            option|0|@TR<<6M>>
            option|1|@TR<<9M>>
            option|2|@TR<<12M>>
            option|3|@TR<<18M>>
            option|4|@TR<<24M>>
            option|5|@TR<<36M>>
            option|6|@TR<<48M>>
            option|7|@TR<<54M>>
            end_form"
         append wforms "$wform" "$N"
    fi
    let "cnt+=1"
done

append forms "$wforms" "$N"

FORM_name_mcast="4"
FORM_tunnel_mcast="no"
FORM_localip_mcast="192.168.2.1"
FORM_remoteip_mcast="192.168.5.1"
FORM_srcip_mcast="192.168.2.200"
FORM_srcmask_mcast="255.255.255.0"
FORM_grpip_mcast="239.255.255.200"

#            field|@TR<<Tunnel>>
#            select|tunnel_mcast|$FORM_tunnel_mcast
#            option|yes|@TR<<YES>>
#            option|no|@TR<<NO>>

form="start_form|Multicast Configuration
            field|@TR<<Name>>
	    text|name_mcast|$FORM_name_mcast


            field|@TR<<Local IP>>
            text|localip_mcast|$FORM_localip_mcast

            field|@TR<<Remote IP>>
	    text|remoteip_mcast|$FORM_remoteip_mcast

            field|@TR<<Source IP>>
	    text|srcip_mcast|$FORM_srcip_mcast

            field|@TR<<Source Mask>>
	    text|srcmask_mcast|$FORM_srcmask_mcast

            field|@TR<<Group IP>>
            text|grpip_mcast|$FORM_grpip_mcast

            field|
            submit|add_mcast|@TR<<Add Multicast>>
            end_form"
append forms "$form" "$N"


#    string|<th>@TR<<Tunnel>></th>

form="  string|<div class=\"settings\">
    string|<h3><strong>@TR<<Multicast Configuration Summary>></strong></h3>
    string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<Multicast Configuration Summary>>\">
    string|<th>@TR<<Name>></th>
    string|<th>@TR<<Local IP>></th>
    string|<th>@TR<<Remote IP>></th>
    string|<th>@TR<<Source IP>></th>
    string|<th>@TR<<Source Mask>></th>
    string|<th>@TR<<Group IP>></th>
    string|</tr>"
append forms "$form" "$N"

uci_load network
vcfgcnt=0

for mcast in $mcast_cfgs; do
    echo "$mcast" |grep -q "mcast_" && { 
        config_get FORM_tunnel $mcast tunnel
        config_get FORM_localip $mcast localip
        config_get FORM_remoteip $mcast remoteip
        config_get FORM_srcip $mcast srcip
        config_get FORM_srcmask $mcast srcmask
        config_get FORM_grpip $mcast grpip
       
        name=$(echo "$mcast" |cut -c 7-)
        append uniq_names "$mcast" "$N"
        echo "$uniq_names" > /var/run/mcast_mcast_name 

        get_tr
	form="$tr  
                string|<td>
                string|$name
                string|</td>


		string|<td>
		string|$FORM_localip
		string|</td>

		string|<td>
		string|$FORM_remoteip
		string|</td>

		string|<td>
		string|$FORM_srcip
		string|</td>

		string|<td>
		string|$FORM_srcmask
		string|</td>

		string|<td>
		string|$FORM_grpip
		string|</td>

		string|<td>
		string|<a href=\"$SCRIPT_NAME?remove_vcfg=$mcast\">@TR<<Remove Mcast Item>></a>
		string|</td>
		string|</tr>"
	append forms "$form" "$N"
        let "vcfgcnt+=1"
    }
done

form="  string|</tr>
        string|</table></div>"

append forms "$form" "$N"


header "Multicast" "Mulitcast" "@TR<<Multicast Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"

sysmode=$(uci get system.@system[0].systemmode)

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

if [ "$sysmode" = "router" -o "$sysmode" = "gateway" ] ; then
display_form <<EOF
onchange|modechange
$validate_error
$forms
EOF
else
display_form <<EOF
$wforms
EOF
fi

footer ?>
<!--
##WEBIF:name:Multicast:100:Multicast
-->
