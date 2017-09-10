#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"

# Parse Settings, this function is called when doing a config_load
config_cb() {
        local cfg_type="$1"
        local cfg_name="$2"

        case "$cfg_type" in
                interface)
                        if [ "wan" = "$cfg_name" ]; then
                           wan_cfg="$cfg_name"
                        fi
                ;;
        esac
}

uci_load multiwan
conf="config"
in_wan="wan"
in_wan2="wan2"
ports="wan wan2"

unset wan_cfg
uci_load network

added_error=""

if [ "$FORM_submit" != "" ] ; then
    local ippassthrough=$(uci get lte.connect.ippassthrough)
    [ "$ippassthrough" = "Ethernet" ] && {
         append added_error "Error in Submit: Can not submit in IP Passthrough Mode" "$N"
    }
fi

if empty "$FORM_submit"; then
       config_get FORM_enabled $conf enabled
       config_get FORM_default_route $conf default_route fastbalancer
       if [ -z "$wan_cfg" -a "$FORM_enabled"="wan" ]; then
            uci_set "multiwan" "$conf" "enabled" "0"
       fi
else
       [ "$added_error" = "" ] && {
            FORM_enabled="$FORM_enabled_mwan"
            FORM_default_route="$FORM_default_route_mwan"
            uci_set "multiwan" "$conf" "enabled" "$FORM_enabled"
            uci_set "multiwan" "$conf" "default_route" "$FORM_default_route"
       }
fi
   
status="start_form| @TR<<Configuration>>
field|@TR<<Multi Wan status>>
select|enabled_mwan|$FORM_enabled
option|0|@TR<<Disable>>
option|1|@TR<<Enable>>
field|@TR<<Primary Connection>>|primary_connection_field|hidden
select|default_route_mwan|$FORM_default_route
option|fastbalancer|@TR<<None>>
option|wan|@TR<<WAN>>
option|wan2|@TR<<4G>>
end_form"

append forms "$status" "$N"

for port in $ports; do
    if empty "$FORM_submit"; then
           config_get FORM_weight $port weight
           config_get FORM_health_interval $port health_interval
           config_get FORM_icmp_hosts $port icmp_hosts
           config_get FORM_icmp_count $port icmp_count
           config_get FORM_timeout $port timeout
           config_get FORM_health_fail_retries $port health_fail_retries
           config_get FORM_health_recovery_retries $port health_recovery_retries
           config_get FORM_failover_to $port failover_to
           config_get FORM_dns $port dns
    else
           eval FORM_weight="\$FORM_weight_$port"
           eval FORM_health_interval="\$FORM_health_interval_$port"
           eval FORM_icmp_hosts="\$FORM_icmp_hosts_$port"
           eval FORM_icmp_count="\$FORM_icmp_count_$port"
           eval FORM_timeout="\$FORM_timeout_$port"
           eval FORM_health_fail_retries="\$FORM_health_fail_retries_$port"
           eval FORM_health_recovery_retries="\$FORM_health_recovery_retries_$port"
           eval FORM_failover_to="\$FORM_failover_to_$port"
           eval FORM_dns="\$FORM_dns_$port"

           [ "$added_error" = "" ] && {
                 uci_set "multiwan" "$port" "weight" "$FORM_weight"
                 uci_set "multiwan" "$port" "health_interval" "$FORM_health_interval"
                 uci_set "multiwan" "$port" "icmp_hosts" "$FORM_icmp_hosts"
                 #uci_set "multiwan" "$port" "icmp_count" "$FORM_icmp_count"
                 uci_set "multiwan" "$port" "timeout" "$FORM_timeout"
                 uci_set "multiwan" "$port" "health_fail_retries" "$FORM_health_fail_retries"
                 uci_set "multiwan" "$port" "health_recovery_retries" "$FORM_health_recovery_retries"
                 uci_set "multiwan" "$port" "failover_to" "$FORM_failover_to"
                 uci_set "multiwan" "$port" "dns" "$FORM_dns"
           }
    fi

    if [ "$port" = "wan" ]; then
        dev="WAN"
    elif [ "$port" = "wan2" ];then
        dev="4G"
    fi
    append forms "start_form|$dev Interface|interface_filed_$port|hidden" "$N"

    ########### weight ########################################
    weight_s='10 9 8 7 6 5 4 3 2 1 disable'
    weight_field="field|@TR<<Load Balancer Distribution>>
                select|weight_$port|$FORM_weight"
    for w in $weight_s; do
        weight_field="$weight_field
            option|$w|$w"
    done                           
    append forms "$weight_field" "$N"
    ########### weight END ########################################

    ########### health_interval ###################################
    health_interval_s='5 10 20 30 60 120'
    health_interval_field="field|@TR<<Health Monitor Interval>>
                select|health_interval_$port|$FORM_health_interval"
    for w in $health_interval_s; do
        health_interval_field="$health_interval_field
            option|$w|$w sec."
    done                           
    health_interval_field="$health_interval_field
            option|disable|Disable"
    append forms "$health_interval_field" "$N"
    ########### health_interval END ###################################

    ########### icmp_hosts ##########################################
    icmp_hosts_field="field|@TR<<Health Monitor ICMP Host>>
            text|icmp_hosts_$port|$FORM_icmp_hosts"
    append forms "$icmp_hosts_field" "$N"
    ########### icmp_hosts END #####################################

    ########### timeout ###########################################
    timeout_s='1 2 3 4 5 10'
    timeout_field="field|@TR<<Health Monitor ICMP Timeout>>
                select|timeout_$port|$FORM_timeout"
    for w in $timeout_s; do
        timeout_field="$timeout_field
            option|$w|$w sec."
    done                           
    append forms "$timeout_field" "$N"
    ########### timeout END ###################################

    ########### health_fail_retries ###################################
    health_fail_retries_s='1 3 5 10 15 20'
    health_fail_retries_field="field|@TR<<Attempts Before $dev Failover>>
                select|health_fail_retries_$port|$FORM_health_fail_retries"
    for w in $health_fail_retries_s; do
        health_fail_retries_field="$health_fail_retries_field
            option|$w|$w"
    done                           
    append forms "$health_fail_retries_field" "$N"
    ########### health_fail_retries END ###################################

    ########### health_recovery_retries ###################################
    health_recovery_retries_s='1 3 5 10 15 20'
    health_recovery_retries_field="field|@TR<<Attempts Before $dev Recovery>>
                select|health_recovery_retries_$port|$FORM_health_recovery_retries"
    for w in $health_recovery_retries_s; do
        health_recovery_retries_field="$health_recovery_retries_field
            option|$w|$w"
    done                           
    append forms "$health_recovery_retries_field" "$N"
    ########### health_recovery_retries END ###################################

    ########### failover_to ###################################
    if [ "$port" = "wan" ];then
        w="wan2"
        w_n="4G"
    elif [ "$port" = "wan2" ];then
        w="wan"
        w_n="WAN"
    fi

    failover_to_field="field|@TR<<Failover Traffic Destination>>
                select|failover_to_$port|$FORM_failover_to
                option|$w|$w_n
                option|fastbalancer|Default
                option|disable|Disable"

    append forms "$failover_to_field" "$N"
                #option|balancer|Load Balancer Compatibility
                #option|fastbalancer|Load Balancer Performance

    ########### failover_to END ###################################

    ########### dns ###################################
    dns_field="field|@TR<<DNS Servers>>
            text|dns_$port|$FORM_dns"
    append forms "$dns_field" "$N"
    ########### dns END ###################################

    append forms "end_form" "$N"

    ###################################################################
    # set JavaScript
    javascript_forms="
        v = isset('enabled_mwan', '1')
        set_visible('interface_filed_$port', v);
        set_visible('primary_connection_field',v);
    "
    append js "$javascript_forms" "$N"

done

ERROR=$added_error

header "MultiWAN" "Settings" "@TR<<Multi WAN Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"


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
$validate_error
$forms
EOF

footer ?>
<!--
##WEBIF:name:MultiWAN:200:Settings
-->
