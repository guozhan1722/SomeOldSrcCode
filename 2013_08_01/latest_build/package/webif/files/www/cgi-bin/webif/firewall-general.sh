#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

fw_file="/etc/config/firewall"
config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		forwarding)
			append forwarding_cfgs "$cfg_name"
		;;
		general)
			append general_cfgs "$cfg_name"
		;;
		defaults)
			append default_cfgs "$cfg_name" "$N"
		;;
		zone)
			append zone_cfgs "$cfg_name" "$N"
		;;
		rule)
			append rule_cfgs "$cfg_name" "$N"
		;;
		redirect)
			append redirect_cfgs "$cfg_name" "$N"
		;;
		interface)
			if [ "$cfg_name" != "loopback" ]; then
				append networks "option|$cfg_name" "$N"
                                append netmode "$cfg_name" "$N"
			fi
		;;
	esac
}

uci_load network
sysmode=$(uci get system.@system[0].systemmode)

if ! empty "$FORM_reset_default"; then
    cp -af /rom${fw_file} ${fw_file} 2>/dev/null
    config_clear
    config_load firewall
    config_get_bool FORM_disabled_firewall normal disabled_firewall 0
    config_get_bool FORM_remote_man normal remote_man 0
    config_get_bool FORM_wan_request normal wan_request 0
    config_get_bool FORM_lanwan_access normal lanwan_access 0

    config_get_bool FORM_remote_man2 normal remote_man2 0
    config_get_bool FORM_wan2_request normal wan2_request 0
    config_get_bool FORM_lanwan2_access normal lanwan2_access 0

    config_get_bool FORM_antispoof normal anti_spoof 0
    config_get_bool FORM_packet_norm normal packet_norm 0
else
    uci_load firewall

    if [ "$FORM_submit" = "" ]; then
        config_get_bool FORM_disabled_firewall normal disabled_firewall 0
        config_get_bool FORM_remote_man normal remote_man 1
        config_get_bool FORM_wan_request normal wan_request 1
        config_get_bool FORM_lanwan_access normal lanwan_access 1

        config_get_bool FORM_remote_man2 normal remote_man2 1
        config_get_bool FORM_wan2_request normal wan2_request 1
        config_get_bool FORM_lanwan2_access normal lanwan2_access 1

        config_get_bool FORM_antispoof normal anti_spoof 1
        config_get_bool FORM_packet_norm normal packet_norm 1

    else
        FORM_disabled_firewall="$FORM_disabled_firewall_g"

        if [ "$sysmode" = "router" -o "$sysmode" = "gateway" ] ; then
            FORM_remote_man="$FORM_remote_man_g"
            FORM_wan_request="$FORM_wan_request_g"
            FORM_lanwan_access="$FORM_lanwan_access_g"

            FORM_remote_man2="$FORM_remote_man2_g"
            FORM_wan2_request="$FORM_wan2_request_g"
            FORM_lanwan2_access="$FORM_lanwan2_access_g"
            FORM_antispoof="$FORM_antispoof_g"
            FORM_packet_norm="$FORM_packet_norm_g"

        else
            FORM_remote_man="1"
            FORM_wan_request="1"
            FORM_lanwan_access="1"
        fi    
        empty "$general_cfgs" && {
            uci_add firewall general "normal"
        }        
        uci_set firewall normal disabled_firewall "$FORM_disabled_firewall"
        uci_set firewall normal remote_man "$FORM_remote_man"
        uci_set firewall normal wan_request "$FORM_wan_request"
        uci_set firewall normal lanwan_access "$FORM_lanwan_access"
        #uci_set firewall normal wanlan_access "$FORM_wanlan_access"
        
        uci_set firewall normal remote_man2 "$FORM_remote_man2"
        uci_set firewall normal wan2_request "$FORM_wan2_request"
        uci_set firewall normal lanwan2_access "$FORM_lanwan2_access"
        #uci_set firewall normal wan2lan_access "$FORM_wan2lan_access"
        uci_set firewall normal anti_spoof "$FORM_antispoof"
        uci_set firewall normal packet_norm "$FORM_packet_norm"
        
        remote_port=`uci get httpd.@httpd[0].port`
        empty "$remote_port" && remote_port="80"

        remote_https_port=`uci get webif.general.ssl_port`
        empty "$remote_https_port" && remote_https_port="443"

        if [ "$FORM_remote_man" = "1" ];then
            add_rule_cfg="DEFRule_remote_man"
            #uci_add firewall rule $add_rule_cfg ; add_rule_cfg="$CONFIG_SECTION"
            uci_set firewall "$add_rule_cfg" proto tcp
    	    uci_set firewall "$add_rule_cfg" src wan
    	    uci_set firewall "$add_rule_cfg" dest wan
    	    uci_set firewall "$add_rule_cfg" dest_port "$remote_port"
    	    uci_set firewall "$add_rule_cfg" target ACCEPT
    	    uci_set firewall "$add_rule_cfg" pri 300

            add_rule_cfg="DEFRule_remote_https_man"
            #uci_add firewall rule $add_rule_cfg ; add_rule_cfg="$CONFIG_SECTION"
            uci_set firewall "$add_rule_cfg" proto tcp
    	    uci_set firewall "$add_rule_cfg" src wan
    	    uci_set firewall "$add_rule_cfg" dest wan
    	    uci_set firewall "$add_rule_cfg" dest_port "$remote_https_port"
    	    uci_set firewall "$add_rule_cfg" target ACCEPT
    	    uci_set firewall "$add_rule_cfg" pri 302

        else
            add_rule_cfg="DEFRule_remote_man"
#            uci_set firewall "$add_rule_cfg" proto tcp
    	    uci_set firewall "$add_rule_cfg" src wan
    	    uci_set firewall "$add_rule_cfg" dest wan
    	    uci_set firewall "$add_rule_cfg" dest_port "$remote_port"
    	    uci_set firewall "$add_rule_cfg" target DROP
    	    uci_set firewall "$add_rule_cfg" pri 300

            add_rule_cfg="DEFRule_remote_https_man"
#            uci_set firewall "$add_rule_cfg" proto tcp
    	    uci_set firewall "$add_rule_cfg" src wan
    	    uci_set firewall "$add_rule_cfg" dest wan
    	    uci_set firewall "$add_rule_cfg" dest_port "$remote_https_port"
    	    uci_set firewall "$add_rule_cfg" target DROP
    	    uci_set firewall "$add_rule_cfg" pri 302
        fi
    
        if [ "$FORM_remote_man2" = "1" ];then
            add_rule_cfg="DEFRule_remote_man2"
            #uci_add firewall rule $add_rule_cfg ; add_rule_cfg="$CONFIG_SECTION"
            uci_set firewall "$add_rule_cfg" proto tcp
    	    uci_set firewall "$add_rule_cfg" src wan2
    	    uci_set firewall "$add_rule_cfg" dest wan2
    	    uci_set firewall "$add_rule_cfg" dest_port "$remote_port"
    	    uci_set firewall "$add_rule_cfg" target ACCEPT
    	    uci_set firewall "$add_rule_cfg" pri 300

            add_rule_cfg="DEFRule_remote_https_man2"
            #uci_add firewall rule $add_rule_cfg ; add_rule_cfg="$CONFIG_SECTION"
            uci_set firewall "$add_rule_cfg" proto tcp
    	    uci_set firewall "$add_rule_cfg" src wan2
    	    uci_set firewall "$add_rule_cfg" dest wan2
    	    uci_set firewall "$add_rule_cfg" dest_port "$remote_https_port"
    	    uci_set firewall "$add_rule_cfg" target ACCEPT
    	    uci_set firewall "$add_rule_cfg" pri 302

        else
            add_rule_cfg="DEFRule_remote_man2"
#            uci_set firewall "$add_rule_cfg" proto tcp
    	    uci_set firewall "$add_rule_cfg" src wan2
    	    uci_set firewall "$add_rule_cfg" dest wan2
    	    uci_set firewall "$add_rule_cfg" dest_port "$remote_port"
    	    uci_set firewall "$add_rule_cfg" target DROP
    	    uci_set firewall "$add_rule_cfg" pri 300

            add_rule_cfg="DEFRule_remote_https_man2"
#            uci_set firewall "$add_rule_cfg" proto tcp
    	    uci_set firewall "$add_rule_cfg" src wan2
    	    uci_set firewall "$add_rule_cfg" dest wan2
    	    uci_set firewall "$add_rule_cfg" dest_port "$remote_https_port"
    	    uci_set firewall "$add_rule_cfg" target DROP
    	    uci_set firewall "$add_rule_cfg" pri 302
        fi
    
        if [ "$FORM_wan_request" = "1" ];then
            add_rule_cfg="DEFRule_wan_reques1"
            #uci_add firewall rule DEFRule_wan_reques1 ; add_rule_cfg="$CONFIG_SECTION"
    	    uci_set firewall "$add_rule_cfg" src wan
    	    uci_set firewall "$add_rule_cfg" ""
    	    uci_set firewall "$add_rule_cfg" proto tcpudp
    	    uci_set firewall "$add_rule_cfg" target ACCEPT
    	    uci_set firewall "$add_rule_cfg" pri 700
    
            add_rule_cfg="DEFRule_wan_reques2"
            #uci_add firewall rule DEFRule_wan_reques2 ; add_rule_cfg="$CONFIG_SECTION"
    	    uci_set firewall "$add_rule_cfg" src wan
    	    uci_set firewall "$add_rule_cfg" dest ""
    	    uci_set firewall "$add_rule_cfg" proto icmp
    	    uci_set firewall "$add_rule_cfg" target ACCEPT
    	    uci_set firewall "$add_rule_cfg" pri 700

        else
    	    uci_set firewall DEFRule_wan_reques1 src wan
    	    uci_set firewall DEFRule_wan_reques1 dest ""
    	    uci_set firewall DEFRule_wan_reques1 proto tcpudp
   	    uci_set firewall DEFRule_wan_reques1 target DROP
    	    uci_set firewall DEFRule_wan_reques1 pri 700

    	    uci_set firewall DEFRule_wan_reques2 src wan
    	    uci_set firewall DEFRule_wan_reques2 dest ""
    	    uci_set firewall DEFRule_wan_reques2 proto icmp
   	    uci_set firewall DEFRule_wan_reques2 target DROP
    	    uci_set firewall DEFRule_wan_reques2 pri 700
        fi
    
        if [ "$FORM_wan2_request" = "1" ];then
            add_rule_cfg="DEFRule_wan2_reques1"
            #uci_add firewall rule DEFRule_wan_reques1 ; add_rule_cfg="$CONFIG_SECTION"
    	    uci_set firewall "$add_rule_cfg" src wan2
    	    uci_set firewall "$add_rule_cfg" dest ""
    	    uci_set firewall "$add_rule_cfg" proto tcpudp
    	    uci_set firewall "$add_rule_cfg" target ACCEPT
    	    uci_set firewall "$add_rule_cfg" pri 700
    
            add_rule_cfg="DEFRule_wan2_reques2"
            #uci_add firewall rule DEFRule_wan_reques2 ; add_rule_cfg="$CONFIG_SECTION"
    	    uci_set firewall "$add_rule_cfg" src wan2
    	    uci_set firewall "$add_rule_cfg" dest ""
    	    uci_set firewall "$add_rule_cfg" proto icmp
    	    uci_set firewall "$add_rule_cfg" target ACCEPT
    	    uci_set firewall "$add_rule_cfg" pri 700

        else
    	    uci_set firewall DEFRule_wan2_reques1 src wan2
    	    uci_set firewall DEFRule_wan2_reques1 dest ""
    	    uci_set firewall DEFRule_wan2_reques1 proto tcpudp
   	    uci_set firewall DEFRule_wan2_reques1 target DROP
    	    uci_set firewall DEFRule_wan2_reques1 pri 700

    	    uci_set firewall DEFRule_wan2_reques2 src wan2
    	    uci_set firewall DEFRule_wan2_reques2 dest ""
    	    uci_set firewall DEFRule_wan2_reques2 proto icmp
   	    uci_set firewall DEFRule_wan2_reques2 target DROP
    	    uci_set firewall DEFRule_wan2_reques2 pri 700
        fi
    
        if [ "$FORM_lanwan_access" = "1" ];then
            add_rule_cfg="DEFRule_lanwan_access_rule1"
            #uci_add firewall rule DEFRule_lanwan_access_rule1 ; add_rule_cfg="$CONFIG_SECTION"
    	    uci_set firewall "$add_rule_cfg" src lan
            uci_set firewall "$add_rule_cfg" dest wan
    	    uci_set firewall "$add_rule_cfg" proto tcpudp
    	    uci_set firewall "$add_rule_cfg" target ACCEPT
    	    uci_set firewall "$add_rule_cfg" pri 640
    
            add_rule_cfg="DEFRule_lanwan_access_rule2"
            #uci_add firewall rule DEFRule_lanwan_access_rule2 ; add_rule_cfg="$CONFIG_SECTION"
    	    uci_set firewall "$add_rule_cfg" src lan
            uci_set firewall "$add_rule_cfg" dest wan
    	    uci_set firewall "$add_rule_cfg" proto icmp
    	    uci_set firewall "$add_rule_cfg" target ACCEPT
    	    uci_set firewall "$add_rule_cfg" pri 640
            
        else
            add_rule_cfg="DEFRule_lanwan_access_rule1"
            #uci_add firewall rule DEFRule_lanwan_access_rule1 ; add_rule_cfg="$CONFIG_SECTION"
    	    uci_set firewall "$add_rule_cfg" src lan
            uci_set firewall "$add_rule_cfg" dest wan
    	    uci_set firewall "$add_rule_cfg" proto tcpudp
    	    uci_set firewall "$add_rule_cfg" target DROP
    	    uci_set firewall "$add_rule_cfg" pri 640

            add_rule_cfg="DEFRule_lanwan_access_rule2"
            #uci_add firewall rule DEFRule_lanwan_access_rule2 ; add_rule_cfg="$CONFIG_SECTION"
    	    uci_set firewall "$add_rule_cfg" src lan
            uci_set firewall "$add_rule_cfg" dest wan
    	    uci_set firewall "$add_rule_cfg" proto icmp
    	    uci_set firewall "$add_rule_cfg" target DROP
    	    uci_set firewall "$add_rule_cfg" pri 640
            
        fi

        if [ "$FORM_lanwan2_access" = "1" ];then
            add_rule_cfg="DEFRule_lanwan2_access_rule1"
            #uci_add firewall rule DEFRule_lanwan_access_rule1 ; add_rule_cfg="$CONFIG_SECTION"
    	    uci_set firewall "$add_rule_cfg" src lan
            uci_set firewall "$add_rule_cfg" dest wan2
    	    uci_set firewall "$add_rule_cfg" proto tcpudp
    	    uci_set firewall "$add_rule_cfg" target ACCEPT
    	    uci_set firewall "$add_rule_cfg" pri 640
    
            add_rule_cfg="DEFRule_lanwan2_access_rule2"
            #uci_add firewall rule DEFRule_lanwan_access_rule2 ; add_rule_cfg="$CONFIG_SECTION"
    	    uci_set firewall "$add_rule_cfg" src lan
            uci_set firewall "$add_rule_cfg" dest wan2
    	    uci_set firewall "$add_rule_cfg" proto icmp
    	    uci_set firewall "$add_rule_cfg" target ACCEPT
    	    uci_set firewall "$add_rule_cfg" pri 640
            
        else
            add_rule_cfg="DEFRule_lanwan2_access_rule1"
            #uci_add firewall rule DEFRule_lanwan_access_rule1 ; add_rule_cfg="$CONFIG_SECTION"
    	    uci_set firewall "$add_rule_cfg" src lan
            uci_set firewall "$add_rule_cfg" dest wan2
    	    uci_set firewall "$add_rule_cfg" proto tcpudp
    	    uci_set firewall "$add_rule_cfg" target DROP
    	    uci_set firewall "$add_rule_cfg" pri 640

            add_rule_cfg="DEFRule_lanwan2_access_rule2"
            #uci_add firewall rule DEFRule_lanwan_access_rule2 ; add_rule_cfg="$CONFIG_SECTION"
    	    uci_set firewall "$add_rule_cfg" src lan
            uci_set firewall "$add_rule_cfg" dest wan2
    	    uci_set firewall "$add_rule_cfg" proto icmp
    	    uci_set firewall "$add_rule_cfg" target DROP
    	    uci_set firewall "$add_rule_cfg" pri 640
            
        fi

        #if [ "$FORM_wanlan_access" = "1" ];then
        #    add_rule_cfg="DEFRule_wanlan_access_rule1"
            #uci_add firewall rule DEFRule_lanwan_access_rule1 ; add_rule_cfg="$CONFIG_SECTION"
    	#    uci_set firewall "$add_rule_cfg" src wan
        #    uci_set firewall "$add_rule_cfg" dest lan
    	#    uci_set firewall "$add_rule_cfg" proto tcpudp
    	#    uci_set firewall "$add_rule_cfg" target ACCEPT
    	#    uci_set firewall "$add_rule_cfg" pri 650
    
        #    add_rule_cfg="DEFRule_wanlan_access_rule2"
            #uci_add firewall rule DEFRule_lanwan_access_rule2 ; add_rule_cfg="$CONFIG_SECTION"
    	#    uci_set firewall "$add_rule_cfg" src wan
        #    uci_set firewall "$add_rule_cfg" dest lan
    	#    uci_set firewall "$add_rule_cfg" proto icmp
    	#    uci_set firewall "$add_rule_cfg" target ACCEPT
    	#    uci_set firewall "$add_rule_cfg" pri 650
            
        #else
        #    add_rule_cfg="DEFRule_wanlan_access_rule1"
            #uci_add firewall rule DEFRule_lanwan_access_rule1 ; add_rule_cfg="$CONFIG_SECTION"
    	#    uci_set firewall "$add_rule_cfg" src wan
        #    uci_set firewall "$add_rule_cfg" dest lan
    	#    uci_set firewall "$add_rule_cfg" proto tcpudp
    	#    uci_set firewall "$add_rule_cfg" target DROP
    	#    uci_set firewall "$add_rule_cfg" pri 650

         #   add_rule_cfg="DEFRule_wanlan_access_rule2"
            #uci_add firewall rule DEFRule_lanwan_access_rule2 ; add_rule_cfg="$CONFIG_SECTION"
    	 #   uci_set firewall "$add_rule_cfg" src wan
         #   uci_set firewall "$add_rule_cfg" dest lan
    	 #   uci_set firewall "$add_rule_cfg" proto icmp
    	 #   uci_set firewall "$add_rule_cfg" target DROP
    	 #   uci_set firewall "$add_rule_cfg" pri 650

        #fi

        #if [ "$FORM_wan2lan_access" = "1" ];then
        #    add_rule_cfg="DEFRule_wan2lan_access_rule1"
            #uci_add firewall rule DEFRule_lanwan_access_rule1 ; add_rule_cfg="$CONFIG_SECTION"
    	#    uci_set firewall "$add_rule_cfg" src wan2
        #    uci_set firewall "$add_rule_cfg" dest lan
    	#    uci_set firewall "$add_rule_cfg" proto tcpudp
    	#    uci_set firewall "$add_rule_cfg" target ACCEPT
    	#    uci_set firewall "$add_rule_cfg" pri 650
    
         #   add_rule_cfg="DEFRule_wan2lan_access_rule2"
            #uci_add firewall rule DEFRule_lanwan_access_rule2 ; add_rule_cfg="$CONFIG_SECTION"
    	 #   uci_set firewall "$add_rule_cfg" src wan2
         #   uci_set firewall "$add_rule_cfg" dest lan
    	 #   uci_set firewall "$add_rule_cfg" proto icmp
    	 #   uci_set firewall "$add_rule_cfg" target ACCEPT
    	 #   uci_set firewall "$add_rule_cfg" pri 650
            
        #else
         #   add_rule_cfg="DEFRule_wan2lan_access_rule1"
            #uci_add firewall rule DEFRule_lanwan_access_rule1 ; add_rule_cfg="$CONFIG_SECTION"
    	 #   uci_set firewall "$add_rule_cfg" src wan2
         #   uci_set firewall "$add_rule_cfg" dest lan
    	 #   uci_set firewall "$add_rule_cfg" proto tcpudp
    	 #   uci_set firewall "$add_rule_cfg" target DROP
    	  #  uci_set firewall "$add_rule_cfg" pri 650

           # add_rule_cfg="DEFRule_wan2lan_access_rule2"
            #uci_add firewall rule DEFRule_lanwan_access_rule2 ; add_rule_cfg="$CONFIG_SECTION"
    	   # uci_set firewall "$add_rule_cfg" src wan2
           # uci_set firewall "$add_rule_cfg" dest lan
    	   # uci_set firewall "$add_rule_cfg" proto icmp
    	   # uci_set firewall "$add_rule_cfg" target DROP
    	   # uci_set firewall "$add_rule_cfg" pri 650

        #fi


    fi
fi

form="start_form|Firewall Mode Configuration
        field|@TR<<Firewall Status>>
       	select|disabled_firewall_g|$FORM_disabled_firewall
        option|0|@TR<<Enable>>
        option|1|@TR<<Disable>>
        end_form"
append forms1 "$form" "$N"

form="start_form|Firewall General Configuration
        field|@TR<<WAN Remote Management>> |remote_man_field|hidden
	radio|remote_man_g|$FORM_remote_man|1|@TR<<Enable>>
	radio|remote_man_g|$FORM_remote_man|0|@TR<<Disable>>

        field|@TR<<4G Remote Management>> |remote_man2_field|hidden
	radio|remote_man2_g|$FORM_remote_man2|1|@TR<<Enable>>
	radio|remote_man2_g|$FORM_remote_man2|0|@TR<<Disable>>

        field|@TR<<WAN Request>>|wan_request_field|hidden
	radio|wan_request_g|$FORM_wan_request|0|@TR<<Block>>
	radio|wan_request_g|$FORM_wan_request|1|@TR<<Allow>>

        field|@TR<<4G Request>>|wan2_request_field|hidden
	radio|wan2_request_g|$FORM_wan2_request|0|@TR<<Block>>
	radio|wan2_request_g|$FORM_wan2_request|1|@TR<<Allow>>

        field|@TR<<LAN to WAN Access Control>>|lanwan_access_field|hidden
	radio|lanwan_access_g|$FORM_lanwan_access|0|@TR<<Block>>
	radio|lanwan_access_g|$FORM_lanwan_access|1|@TR<<Allow>>

        field|@TR<<LAN to 4G Access Control>>|lanwan2_access_field|hidden
	radio|lanwan2_access_g|$FORM_lanwan2_access|0|@TR<<Block>>
	radio|lanwan2_access_g|$FORM_lanwan2_access|1|@TR<<Allow>>

        field|@TR<<Anti-Spoof>>|antispoof_field|hidden
	radio|antispoof_g|$FORM_antispoof|1|@TR<<Enable>>
	radio|antispoof_g|$FORM_antispoof|0|@TR<<Disable>>

        field|@TR<<Packet Normalization>>|packet_norm_field|hidden
	radio|packet_norm_g|$FORM_packet_norm|1|@TR<<Enable>>
	radio|packet_norm_g|$FORM_packet_norm|0|@TR<<Disable>>

        end_form"
append forms2 "$form" "$N"

form="start_form|Firewall reset Configuration
        field||reset_button_field|hidden
        submit|reset_default|@TR<<Reset Firewall To Default Now>>
        end_form"
append forms3 "$form" "$N"


javascript_forms="
       v = isset('disabled_firewall_g', '0');
       set_visible('remote_man_field', v);
       set_visible('wan_request_field', v);
       set_visible('lanwan_access_field', v);
       set_visible('reset_button_field', v);

       set_visible('remote_man2_field', v);
       set_visible('wan2_request_field', v);
       set_visible('lanwan2_access_field', v);
       set_visible('antispoof_field', v);
       set_visible('packet_norm_field', v);
"
append js "$javascript_forms" "$N"

if [ -n "$FORM_add_rule_add" ]; then
	uci_add firewall forwarding ""; add_forward_cfg="$CONFIG_SECTION"
	uci_set firewall "$add_forward_cfg" src "$FORM_src_add"
	uci_set firewall "$add_forward_cfg" dest "$FORM_dest_add"
fi

header "Firewall" "General" "@TR<<Firewall General>>" 'onload="modechange()"' "$SCRIPT_NAME"

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
$forms1
$forms2
$forms3
EOF

else
display_form <<EOF
onchange|modechange
$forms1
$forms3
EOF

fi
footer ?>
<!--
##WEBIF:name:Firewall:200:General
-->
