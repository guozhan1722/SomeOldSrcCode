#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
. /etc/version

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		general)
			append general_cfgs "$cfg_name" "$N"
		;;
		vlan)
			append vlan_cfgs "$cfg_name" "$N"
                        #vlan_num=$(echo $cfg_name|cut -c 5-)
                        #append available_num "$vlan_num" "$N"
                        append available_vlan "option|$cfg_name" "$N"
		;;
		port)
			append port_cfgs "$cfg_name" "$N"
		;;
		interface)
			if [ "$cfg_name" != "loopback" ]; then
				append networks "option|$cfg_name" "$N"
                                append netmode "$cfg_name" "$N"
			fi
		;;
	esac
}

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

switch_port_lan()
{
    local ch_port="$1"

    if [ "$HARDWARE" = "5.0.0" ]; then
        case "$ch_port" in
            eth0)
                    vlanport_d="WAN"
            ;;
            eth1)
                    vlanport_d="LAN"
            ;;
            wlan0)
                    vlanport_d="Radio1"
            ;;
            wlan1)
                    vlanport_d="Radio2"
            ;;
            wlan2)
                    vlanport_d="Radio3"
            ;;
            wlan3)
                    vlanport_d="Radio4"
            ;;
        esac
    elif [ "$HARDWARE" = "3.0.0" ]; then
        case "$ch_port" in
            eth0)
                    vlanport_d="LAN"
            ;;
            wlan0)
                    vlanport_d="Radio1"
            ;;
        esac
    elif [ "$HARDWARE" = "v1.0.0" ]; then
        case "$ch_port" in
            eth0)
                    vlanport_d="LAN"
            ;;
            wlan0)
                    vlanport_d="Radio1"
            ;;
        esac
    else
        case "$ch_port" in
            eth0)
                    vlanport_d="WAN"
            ;;
            eth1)
                    vlanport_d="LAN"
            ;;
            wlan0)
                    vlanport_d="Radio1"
            ;;
        esac

    fi
   
}

all_ports="eth0 eth1 wlan0 wlan1 wlan2 wlan3"
port_list=""
for port in $all_ports ; do
    # vlan enabled
    port_in_lan=$(uci get vlan.$port.is_bridged)
    ! empty "$port_in_lan" && [ $port_in_lan -gt 0 ] && append port_list "$port" "$N"    	
done

#port_list=$(brctl show br-lan | grep -v interfaces |awk '{print $NF}')
#remove rule
if ! empty "$FORM_remove_vcfg"; then
    #config_get FORM_id "$FORM_remove_vcfg" id
    FORM_id=$(uci get vlan.$FORM_remove_vcfg.id)
    uci_remove "vlan" "$FORM_remove_vcfg"
    for vlanport in $port_list; do
        FORM_vlan=$(uci get vlan.$vlanport.vlan)
        FORM_tag=$(uci get vlan.$vlanport.tag)
        is_set_vlan=""
        is_set_tag=""
        for s_vlan in $FORM_vlan; do
            if [ "$s_vlan" != "$FORM_id" ];then
                append is_set_vlan "$s_vlan"
            fi
        done

        if [ "$FORM_tag" != "$FORM_id" ];then
                is_set_tag="$FORM_tag"
        fi
        uci_set vlan "$vlanport" vlan "$is_set_vlan"
        uci_set vlan "$vlanport" tag "$is_set_tag"
    done  
fi

if ! empty "$FORM_added_vlan"; then
    #[ -z "$FORM_name_rule" ] && append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: Rule Name not unique</h3></td></tr>" "$N"

    un_names=$(cat /var/run/vlan_name)
    for chname in $un_names; do
        if [ "$chname" = "$FORM_id_vlan" ]; then
            append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: VLAN ID not unique</h3></td></tr>" "$N"
            break
        fi
    done
validate <<EOF
int|FORM_id_vlan|@TR<<VLAN ID>>|min=2 max=4094 required|$FORM_id_vlan
EOF
	equal "$?" 0 && {
            have_error=0
            for vlanport in $port_list; do
                FORM_tag=$(uci get vlan.$vlanport.tag)    
                eval FORM_add_tag="\${FORM_tag_vlan_$vlanport}"
                switch_port_lan $vlanport
                if [ "$FORM_add_tag" = "1" ]; then
                    if [ "$FORM_tag" != "" -a "$FORM_tag" != "1" ]; then
                            append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Tag: VLAN have been tagged by $FORM_tag on $vlanport_d Port.</h3></td></tr>" "$N"
                            have_error=1    
                            break
                    fi
                fi
            done

            [ "$have_error" = "0" ] && {
       		uci_add vlan vlan "vlan${FORM_id_vlan}"; add_vlan_cfg="$CONFIG_SECTION"
		uci_set vlan "$add_vlan_cfg" id "$FORM_id_vlan"
	        uci_set vlan "$add_vlan_cfg" description "$FORM_description_vlan"
                
                for vlanport in $port_list; do
                    FORM_vlan=$(uci get vlan.$vlanport.vlan)    
                    FORM_tag=$(uci get vlan.$vlanport.tag)    
                    eval FORM_add_vlan="\${FORM_vlan_vlan_$vlanport}"
                    eval FORM_add_tag="\${FORM_tag_vlan_$vlanport}"
                    if [ "$FORM_add_vlan" = "1" ]; then
                        append FORM_vlan "$FORM_id_vlan"
                        uci_set "vlan" "$vlanport" "vlan" "$FORM_vlan"
                    fi
                    if [ "$FORM_add_tag" = "1" ]; then
                        FORM_tag="$FORM_id_vlan"
                        uci_set "vlan" "$vlanport" "tag" "$FORM_tag"
                    fi
                    unset FORM_add_vlan FORM_add_tag FORM_vlan FORM_tag
                done

		unset FORM_id_vlan FORM_description_vlan
#	} || {
              #  append  added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">$ERROR</h3></td></tr>" "$N"
            }
        }
fi

#Setting general configuration
config_load vlan

if [ "$FORM_submit" = "" ]; then
    config_get FORM_disabled "$general_cfgs" disabled
    config_get FORM_management "$general_cfgs" management
    
else
    FORM_disabled="$FORM_disabled_000"
    FORM_management="$FORM_management_000"

    uci_set vlan "$general_cfgs" disabled "$FORM_disabled"
    uci_set vlan "$general_cfgs" management "$FORM_management"
fi

form="start_form|General Configuration
        field|@TR<<VLAN:>>
        select|disabled_000|$FORM_disabled
        option|0|Enabled
        option|1|Disabled
        field|@TR<<Management VLAN>>|management_field|hidden
        select|management_000|$FORM_management
        $available_vlan
        end_form"
append forms "$form" "$N"

#Setting VLAN1 configuration
if [ "$FORM_submit" = "" ]; then
    config_get FORM_vid1 "vlan1" id
    config_get FORM_description1 "vlan1" description

else
    FORM_description1="$FORM_description_001"
    if ! empty "$FORM_save_vlan1"; then
        uci_set vlan "vlan1" description "$FORM_description1"
    fi
fi

form="  string|<div id=\"vlan1_id_field\" class=\"settings\">
        string|<h3><strong>@TR<<VLAN1 Configuration>></strong></h3>
        string|<div class=\"settings-content\">
        string|<table width=\"100%\" summary=\"Settings\">"
append forms "$form" "$N"

#form="start_form|VLAN1 Configuration
form="
        field|@TR<<VLAN ID>>
	string|1

        field|@TR<<Description>>
	text|description_001|$FORM_description1"
append forms "$form" "$N"

for vlanport in $port_list; do
    if [ "$FORM_submit" = "" ]; then
        config_get FORM_ifname "$vlanport" ifname
        config_get FORM_vlan "$vlanport" vlan
        FORM_vlan1_port="0"
        for set_vlan in $FORM_vlan ; do
            if [ "$set_vlan" = "1" ];then
                FORM_vlan1_port="1"
                break
            fi
        done

    else
        eval FORM_vlan1_port="\${FORM_vlan1_$vlanport}"

        config_get FORM_vlan "$vlanport" vlan
        vlan1_ori=""
        for set_vlan in $FORM_vlan ; do
            if [ "$set_vlan" != "1" ];then
                append vlan1_ori "$set_vlan"                             
            fi
        done
        
        if [ "$FORM_vlan1_port" = "1" ];then
            append  vlan1_ori "1"                             
        fi
    
        if ! empty "$FORM_save_vlan1"; then
            uci_set "vlan" "$vlanport" "vlan" "$vlan1_ori"    
        fi
    fi

    switch_port_lan $vlanport

    form="  field|$vlanport_d
            select|vlan1_$vlanport|$FORM_vlan1_port
            option|1|@TR<<Join VLAN>>
            option|0|@TR<<No VLAN>>"
    append forms "$form" "$N"

done

form="  field||
        submit|save_vlan1|@TR<<Save VLAN1>>
        end_form"
append forms "$form" "$N"
#append forms "end_form" "$N"

#added new vlan config

FORM_id_vlan=""
FORM_description_vlan=""

form="  string|<div id=\"add_new_vlan_id_field\" class=\"settings\">
        string|<h3><strong>@TR<<VLANs Configuration>></strong></h3>
        string|<div class=\"settings-content\">
        string|<table width=\"100%\" summary=\"Settings\">"
append forms "$form" "$N"

#form="start_form|VLANs Configuration
form="
        field|@TR<<VLAN ID>>
	text|id_vlan|$FORM_id_vlan

        field|@TR<<Description>>
	text|description_vlan|$FORM_description_vlan"
append forms "$form" "$N"

FORM_vlan_port_vlan="0"
FORM_tag_port_vlan="0"

for vlanport in $port_list; do

    switch_port_lan $vlanport

    form="  field|$vlanport_d
            select|vlan_vlan_$vlanport|$FORM_vlan_port_vlan
            option|1|@TR<<Join VLAN>>
            option|0|@TR<<No VLAN>>
    
            select|tag_vlan_$vlanport|$FORM_tag_port_vlan
            option|1|@TR<<Tag>>
            option|0|@TR<<Untag>>"
    
    append forms "$form" "$N"
done

form="  field||
        submit|added_vlan|@TR<<Add VLAN>>
        end_form"
append forms "$form" "$N"

# Display all vlan settings
form="  string|<div id=\"vlan_config_summary\" class=\"settings\">
        string|<h3><strong>@TR<<VLANS Configurate Summary>></strong></h3>
        string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<VLANS Configurate Summary>>\">
        string|<th>@TR<<ID>></th>
        string|<th>@TR<<Description>></th>"
append forms "$form" "$N"

for vlanport in $port_list; do
    switch_port_lan $vlanport
    form=" string|<th>${vlanport_d}</th>"
    append forms "$form" "$N"
done

form=" string|</tr>"
append forms "$form" "$N"

vcfgcnt=0
uniq_names=""
rm -rf /var/run/qos_global_name

for rule in $vlan_cfgs; do
    config_get FORM_id "$rule" id
    name=$FORM_id
    append uniq_names "$name" "$N"
    echo "$uniq_names" > /var/run/vlan_name

    if [ "$FORM_id" != "1" ]; then
        if [ "$FORM_submit" = "" -o "$add_vlan_cfg" = "$rule" ]; then
            config_get FORM_description "$rule" description
        else
            eval FORM_description="\$FORM_description_$vcfgcnt"
            uci_set vlan "$rule" description "$FORM_description"
        fi

        get_tr
        form="$tr  
                string|<td>
                string|$FORM_id
                string|</td>

                string|<td>
                text|description_$vcfgcnt|$FORM_description
                string|</td>"

        append forms "$form" "$N"

        for vlanport in $port_list; do
            if [ "$FORM_submit" = "" -o "$add_vlan_cfg" = "$rule" ]; then
                config_get FORM_ifname "$vlanport" ifname
                config_get FORM_vlan "$vlanport" vlan
                config_get FORM_tag "$vlanport" tag

                FORM_vlan_port="0"
                for set_vlan in $FORM_vlan ; do
                    if [ "$set_vlan" = "$FORM_id" ];then
                        FORM_vlan_port="1"
                        break
                    fi
                done
        
                FORM_tag_port="0"
                for set_tag in $FORM_tag ; do
                    if [ "$set_tag" = "$FORM_id" ];then
                        FORM_tag_port="1"
                        break
                    fi
                done
            else
                eval FORM_vlan_port="\${FORM_vlan_$vlanport$vcfgcnt}"
                eval FORM_tag_port="\${FORM_tag_$vlanport$vcfgcnt}"
                eval save_button="\${FORM_save_$vcfgcnt}"
                vlan_ori=""
                tag_ori=""

                config_get FORM_vlan "$vlanport" vlan
                
                for set_vlan in $FORM_vlan ; do
                    if [ "$set_vlan" != "$FORM_id" ];then
                        append vlan_ori "$set_vlan"                             
                    fi
                done
        
                if [ "$FORM_vlan_port" = "1" ];then
                    append  vlan_ori "$FORM_id"                             
                fi
                
                if ! empty "$save_button"; then
                    uci_set vlan "$vlanport" vlan "$vlan_ori"    
                fi                
                
                config_get FORM_tag "$vlanport" tag
                if [ "$FORM_tag" != "$FORM_id" ];then
                    tag_ori="$FORM_tag"                             
                fi

                have_error=0
                switch_port_lan $vlanport
                if [ "$FORM_tag_port" = "1" ];then
                    if [ "$FORM_tag" != "" -a "$FORM_tag" != "1" -a "$FORM_tag" != "$FORM_id" ]; then
                            append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Tag id=$FORM_id: VLAN have been tagged by $FORM_tag on $vlanport_d Port.</h3></td></tr>" "$N"
                            have_error=1
                #        fi
                    fi
                    tag_ori="$FORM_id"                             
                fi
                
                if ! empty "$save_button"; then
                    if [ "$have_error" != "1" ] ;then
                        uci_set vlan "$vlanport" tag "$tag_ori"
                    fi
                    #logger "uci set command : uci_set vlan $vlanport vlan $vlan_ori"
                    #logger "uci set command : uci_set vlan $vlanport tag $tag_ori"
                fi                
            fi

            form="  string|<td>
                    select|vlan_$vlanport$vcfgcnt|$FORM_vlan_port
                    option|1|@TR<<Join VLAN>>
                    option|0|@TR<<No VLAN>>
    
                    select|tag_$vlanport$vcfgcnt|$FORM_tag_port
                    option|1|@TR<<Tag>>
                    option|0|@TR<<UnTag>>
                    string|</td>"
    
            append forms "$form" "$N"
        done


        form=" string|<td>
                submit|save_$vcfgcnt|@TR<<Save>>
                string|</td>"
        append forms "$form" "$N"

        form=" string|<td>
                string|<a href=\"$SCRIPT_NAME?remove_vcfg=$rule\">@TR<<Remove>></a>
                string|</td>
                string|</tr>"
        append forms "$form" "$N"
    fi
    let "vcfgcnt+=1"
done

form="  string|</tr>
        string|</table></div>"
append forms "$form" "$N"

javascript_forms="
       v = isset('disabled_000', '0');
       set_visible('management_field', 1);
       set_visible('vlan1_id_field', v);
       set_visible('add_new_vlan_id_field', v);
       set_visible('vlan_config_summary', v);
"
append js "$javascript_forms" "$N"

vcfgcnt=0
for rule in $vlan_cfgs; do
for vlanport in $port_list; do
    javascript_forms="
       if (isset('vlan_$vlanport$vcfgcnt','0'))
       {
            document.getElementById('tag_$vlanport$vcfgcnt').selectedIndex = '1';
       }

       if (isset('vlan_vlan_$vlanport','0'))
       {
            document.getElementById('tag_vlan_$vlanport').selectedIndex = '1';
       }

    "
    append js "$javascript_forms" "$N"
done
let "vcfgcnt+=1"
done

header "Network" "VLAN" "@TR<<network VLANs Configuration>>" 'onload="modechange()"' "$SCRIPT_NAME"

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
$added_error
$forms
EOF

footer ?>
#<!--
###WEBIF:name:Network:450:VLAN
#-->
