#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		class)
			append class_cfgs "$cfg_name" "$N"
		;;
		general)
			append general_cfgs "$cfg_name" "$N"
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

#init the adding field
uci_load qos

if [ "$FORM_submit" = "" ]; then
    config_get FORM_enabled "$general_cfgs" enabled
else
    eval FORM_enabled="$FORM_enabled_000"
    uci_set qos "$general_cfgs" enabled "$FORM_enabled"
fi

form="start_form|Qos Mode
        field|@TR<<Qos Mode>>
        select|enabled_000|$FORM_enabled
        option|1|Enable
        option|0|Disable
        end_form"
append forms "$form" "$N"

form="  string|<div class=\"settings\">
    string|<h3><strong>@TR<<LAN Port Qos Class Configuration>></strong></h3>
    string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<LAN Port Qos Class Configuration>>\">
    string|<th>@TR<<Target>></th>
    string|<th>@TR<<Rate>></th>
    string|<th>@TR<<Ceil>></th>
    string|<th>@TR<<Burst>></th>
    string|</tr>"
append forms "$form" "$N"

order="High Medium_high Medium Medium_low Low"
vcfgcnt=0

for rule in $order; do

    found=0
    for find in $class_cfgs; do
        config_get FORM_iface "$find" iface
        config_get FORM_target "$find" target
        if [ "$FORM_iface" = "br-lan" -a "$FORM_target" = "$rule" ]; then
            if [ "$FORM_submit" = "" ]; then
                FORM_iface="br-lan"
                FORM_target="$rule"
                config_get FORM_rate_w "$find" rate
                FORM_rate=$(echo $FORM_rate_w|awk '{print $1+0}')
                FORM_rate_unit=$(echo $FORM_rate_w|awk -F "$FORM_rate" '{print $2}')
                if [ "$FORM_rate_unit" = "" ];then
                    FORM_rate_unit="kbit"
                fi
                config_get FORM_ceil_w "$find" ceil 
                FORM_ceil=$(echo $FORM_ceil_w|awk '{print $1+0}')
                FORM_ceil_unit=$(echo $FORM_ceil_w|awk -F "$FORM_ceil" '{print $2}')
                if [ "$FORM_ceil_unit" = "" ];then
                    FORM_ceil_unit="kbit"
                fi
                config_get FORM_burs_w "$find" burs
                FORM_burs=$(echo $FORM_burs_w|awk '{print $1+0}')
                FORM_burs_unit=$(echo $FORM_burs_w|awk -F "$FORM_burs" '{print $2}')
                if [ "$FORM_burs_unit" = "" ];then
                    FORM_burs_unit="kbit"
                fi
            else
                FORM_target=$rule
                FORM_iface="br-lan"
                eval FORM_rate="\$FORM_rate_$vcfgcnt"
                eval FORM_rate_unit="\$FORM_rate_unit_$vcfgcnt"
                eval FORM_ceil="\$FORM_ceil_$vcfgcnt"
                eval FORM_ceil_unit="\$FORM_ceil_unit_$vcfgcnt"
                eval FORM_burs="\$FORM_burs_$vcfgcnt"
                eval FORM_burs_unit="\$FORM_burs_unit_$vcfgcnt"
validate <<EOF
int|FORM_rate|@TR<<Rate>>||$FORM_rate
int|FORM_ceil|@TR<<Ceil>>||$FORM_ceil
int|FORM_burs|@TR<<Burst>>||$FORM_burs
EOF
                equal "$?" 0 && {
                    uci_set qos "$find" iface "$FORM_iface"
                    uci_set qos "$find" target "$FORM_target"
                    uci_set qos "$find" rate "${FORM_rate}${FORM_rate_unit}"
                    uci_set qos "$find" ceil "${FORM_ceil}${FORM_ceil_unit}"
                    uci_set qos "$find" burs "${FORM_burs}${FORM_burs_unit}"
                } || {

                    append  added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">$ERROR</h3></td></tr>" "$N"
                }  
                 
            fi

            break
        fi         
    done
    
    get_tr
    form="$tr  
            string|<td>
            string|$rule
            string|</td>
    
            string|<td>
            string|<input type=text name=rate_$vcfgcnt value=$FORM_rate size=8>
            select|rate_unit_$vcfgcnt|$FORM_rate_unit
            option|kbit|kbit
            option|Mbit|Mbit
            string|</td>
    
            string|<td>
            string|<input type=text name=ceil_$vcfgcnt value=$FORM_ceil size=8>
            select|ceil_unit_$vcfgcnt|$FORM_ceil_unit
            option|kbit|kbit
            option|Mbit|Mbit
            string|</td>
    
            string|<td>
            string|<input type=text name=burs_$vcfgcnt value=$FORM_burs size=8>
            select|burs_unit_$vcfgcnt|$FORM_burs_unit
            option|kbit|kbit
            option|Mbit|Mbit
            string|</td>"
    
    append forms "$form" "$N"
    
    let "vcfgcnt+=1"
    
done

form="  string|</tr>
        string|</table></div>"

append forms "$form" "$N"


form="  string|<div class=\"settings\">
    string|<h3><strong>@TR<<WAN Port Qos Class Configuration>></strong></h3>
    string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<WAN Port Qos Class Configuration>>\">
    string|<th>@TR<<Target>></th>
    string|<th>@TR<<Rate>></th>
    string|<th>@TR<<Ceil>></th>
    string|<th>@TR<<Burst>></th>
    string|</tr>"
append forms "$form" "$N"

vcfgcnt=10

for rule in $order; do

    found=0
    for find in $class_cfgs; do
        config_get FORM_iface "$find" iface
        config_get FORM_target "$find" target
        if [ "$FORM_iface" = "br-wan" -a "$FORM_target" = "$rule" ]; then
            if [ "$FORM_submit" = "" ]; then
                FORM_iface="br-wan"
                FORM_target="$rule"
                config_get FORM_rate_w "$find" rate
                FORM_rate=$(echo $FORM_rate_w|awk '{print $1+0}')
                FORM_rate_unit=$(echo $FORM_rate_w|awk -F "$FORM_rate" '{print $2}')
                if [ "$FORM_rate_unit" = "" ];then
                    FORM_rate_unit="kbit"
                fi
                config_get FORM_ceil_w "$find" ceil 
                FORM_ceil=$(echo $FORM_ceil_w|awk '{print $1+0}')
                FORM_ceil_unit=$(echo $FORM_ceil_w|awk -F "$FORM_ceil" '{print $2}')
                if [ "$FORM_ceil_unit" = "" ];then
                    FORM_ceil_unit="kbit"
                fi
                config_get FORM_burs_w "$find" burs
                FORM_burs=$(echo $FORM_burs_w|awk '{print $1+0}')
                FORM_burs_unit=$(echo $FORM_burs_w|awk -F "$FORM_burs" '{print $2}')
                if [ "$FORM_burs_unit" = "" ];then
                    FORM_burs_unit="kbit"
                fi
            else
                FORM_target=$rule
                FORM_iface="br-wan"
                eval FORM_rate="\$FORM_rate_$vcfgcnt"
                eval FORM_rate_unit="\$FORM_rate_unit_$vcfgcnt"
                eval FORM_ceil="\$FORM_ceil_$vcfgcnt"
                eval FORM_ceil_unit="\$FORM_ceil_unit_$vcfgcnt"
                eval FORM_burs="\$FORM_burs_$vcfgcnt"
                eval FORM_burs_unit="\$FORM_burs_unit_$vcfgcnt"
validate <<EOF
int|FORM_rate|@TR<<Rate>>||$FORM_rate
int|FORM_ceil|@TR<<Ceil>>||$FORM_ceil
int|FORM_burs|@TR<<Burst>>||$FORM_burs
EOF
                equal "$?" 0 && {
                    uci_set qos "$find" iface "$FORM_iface"
                    uci_set qos "$find" target "$FORM_target"
                    uci_set qos "$find" rate "${FORM_rate}${FORM_rate_unit}"
                    uci_set qos "$find" ceil "${FORM_ceil}${FORM_ceil_unit}"
                    uci_set qos "$find" burs "${FORM_burs}${FORM_burs_unit}"
                } || {

                    append  added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">$ERROR</h3></td></tr>" "$N"
                }  
                 
            fi

            break
        fi         
    done
    
    get_tr
    form="$tr  
            string|<td>
            string|$rule
            string|</td>
    
            string|<td>
            string|<input type=text name=rate_$vcfgcnt value=$FORM_rate size=8>
            select|rate_unit_$vcfgcnt|$FORM_rate_unit
            option|kbit|kbit
            option|Mbit|Mbit
            string|</td>
    
            string|<td>
            string|<input type=text name=ceil_$vcfgcnt value=$FORM_ceil size=8>
            select|ceil_unit_$vcfgcnt|$FORM_ceil_unit
            option|kbit|kbit
            option|Mbit|Mbit
            string|</td>
    
            string|<td>
            string|<input type=text name=burs_$vcfgcnt value=$FORM_burs size=8>
            select|burs_unit_$vcfgcnt|$FORM_burs_unit
            option|kbit|kbit
            option|Mbit|Mbit
            string|</td>"
    
    append forms "$form" "$N"
    
    let "vcfgcnt+=1"
    
done

form="  string|</tr>
        string|</table></div>
        "

append forms "$form" "$N"


header "Qos" "Class" "@TR<<Qos Class Configuration>>" 'onload="modechange()"' "$SCRIPT_NAME"

sysmode=$(uci get system.@system[0].systemmode)

if [ "$sysmode" = "router" -o "$sysmode" = "gateway" ] ; then
display_form <<EOF
$validate_error
$added_error
$forms
EOF
else
display_form <<EOF
string|<tr><td colspan=\"2\"><h3 class=\"warning\">No Qos Class Configuration in Bridge mode </h3></td></tr>
EOF
fi

footer ?>
<!--
##WEBIF:name:Qos:200:Class
-->
