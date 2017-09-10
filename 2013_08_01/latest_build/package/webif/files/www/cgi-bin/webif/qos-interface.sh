#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		interface)
			append interface_cfgs "$cfg_name" "$N"
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
		tr="string|<tr interface=\"odd\">"
	fi
}

#init the adding field

#target: "High Medium_high Medium Medium_low Low"
#dscp: "0x25 0x24 0x23 0x22 0x21"
#prio: "2 3 4 5 6"
get_dscp_prio() {

    local target_g="$1"
    
    case "$target_g" in
            High)
                    dscp_cfg="0x25"
                    prio_cfg="2"
            ;;
            Medium_high)
                    dscp_cfg="0x24"
                    prio_cfg="3"
            ;;
            Medium)
                    dscp_cfg="0x23"
                    prio_cfg="4"
            ;;
            Medium_low)
                    dscp_cfg="0x22"
                    prio_cfg="5"
            ;;
            Low)
                    dscp_cfg="0x21"
                    prio_cfg="6"
            ;;
    esac
}

uci_load qos
vcfgcnt=0
for rule in $interface_cfgs; do

    if [ "$FORM_submit" = "" ]; then
        config_get FORM_enabled "$rule" enabled
        config_get FORM_target "$rule" target
        config_get FORM_bandwidth_w "$rule" bandwidth
        FORM_bandwidth=$(echo $FORM_bandwidth_w|awk '{print $1+0}')
    else
        eval FORM_enabled="\$FORM_enabled_$vcfgcnt"
        eval FORM_target="\$FORM_target_$vcfgcnt"
        eval FORM_bandwidth="\$FORM_bandwidth_$vcfgcnt" 
validate <<EOF
int|FORM_bandwidth|@TR<<Rate>>|min=5 max=100|$FORM_bandwidth
EOF
        equal "$?" 0 && {
            get_dscp_prio $FORM_target
            uci_set qos "$rule" enabled "$FORM_enabled"
            uci_set qos "$rule" bandwidth "${FORM_bandwidth}Mbit"
            uci_set qos "$rule" target "$FORM_target"
            uci_set qos "$rule" dscp "$dscp_cfg"
            uci_set qos "$rule" prio "$prio_cfg"
        }
    fi

    config_get FORM_iface "$rule" iface
    FORM_iface=$(echo $FORM_iface|cut -c 4-)
   
    form="start_form|Qos Interface $FORM_iface Configuration
    
                field|@TR<<Interface Mode>>
                select|enabled_$vcfgcnt|$FORM_enabled
    	        option|1|Enable
    	        option|0|Disable
    
                field|@TR<<Target>>|target_field_$vcfgcnt|hidden
                select|target_$vcfgcnt|$FORM_target
                option|High|@TR<<High>>
                option|Medium_high|@TR<<Medium_high>>
                option|Medium|@TR<<Medium>>
                option|Medium_low|@TR<<Medium_low>>
                option|Low|@TR<<Low>>
    
                field|@TR<<Bandwidth (Mbit)>>|bandwidth_field_$vcfgcnt|hidden
                text|bandwidth_$vcfgcnt|$FORM_bandwidth
                end_form"
    append forms "$form" "$N"

    javascript_forms="
           v = isset('enabled_$vcfgcnt', '1');
           set_visible('target_field_$vcfgcnt', v);
           set_visible('bandwidth_field_$vcfgcnt', v);
    "
    append js "$javascript_forms" "$N"

    let "vcfgcnt+=1"
done

header "Qos" "Interface" "@TR<<Qos Interface Configuration>>" 'onload="modechange()"' "$SCRIPT_NAME"

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

sysmode=$(uci get system.@system[0].systemmode)

if [ "$sysmode" = "router" -o "$sysmode" = "gateway" ] ; then
display_form <<EOF
onchange|modechange
$forms
EOF
else
display_form <<EOF
string|<tr><td colspan=\"2\"><h3 interface=\"warning\">No Qos Interface Configuration in Bridge mode </h3></td></tr>
EOF
fi

footer ?>
<!--
##WEBIF:name:Qos:500:Interface
-->
