#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"




######### Serach Carrier ##########

FORM_manual_options()
{
    /etc/init.d/lte stop
    /etc/init.d/keepalive stop
    /etc/init.d/twatchdog stop
    /lib/atcmd.sh --port /dev/ttyUSB0 --disconnect
    sleep 8
    /lib/atcmd.sh --port /dev/ttyUSB0 --search > /var/run/availableCarrier
    [ -f "/var/run/availableCarrier" ] && {
        cat /var/run/availableCarrier|grep +COPS:|awk -F "," '{print $2 "," $4}'|sed 's/\"//g'|sort|uniq > /var/run/CarrierList
    }

    awk -v "maclist_url=$MACLIST_URL" -F "," '

        BEGIN { nc = 0; }	
        { 
            
        carrier_name[nc] = $1;
        carrier_id[nc] = $2;
        
        print nc carrier_name[nc] carrier_id[nc] >> "/var/run/abc";
        nc = nc + 1;  
        }
        END {
            printf("string|<select id=\"search_carrier_id_carrier\" name=\"search_carrier_id_carrier\" onchange=\"modechange(this)\">\n");
   
            for (i=0; i < nc; i++) {                 
                 printf("string|<option value=\"%s\"> %s - %s</option>\n", carrier_id[i], carrier_id[i], carrier_name[i]);            
            }
            printf("string|</select>"); 
        }
    
    ' "/var/run/CarrierList"
    ### start keepalive/twatchdog upon user click submit button
#    start-stop-daemon --start --quiet --background --exec "/bin/keepalivebootup"
#    start-stop-daemon --start --quiet --background --exec "/lib/start_twatchdog.sh"
    
}

wan_ip="$(ifconfig br-wan2 | grep inet | grep -v inet6 | awk -F'[: ]+' '{print $4}')"
if [ "$wan_ip" = "1.1.1.1" ];then
   wan_ip=$(cat /var/run/ippth_4g)
elif [ "$wan_ip" = "" ];then
   wan_ip="N/A"
fi


uci_load lte
conf="connect"

uci_load multiwan
 
DESC_FILE="/usr/lib/hardware_desc"
[ -f $DESC_FILE ] && . $DESC_FILE

if empty "$FORM_submit"; then
       config_get FORM_disabled $conf disabled 
       config_get FORM_apn $conf apn 
       config_get FORM_tech $conf tech 
       config_get FORM_call_para $conf call_para 
       config_get FORM_pdns $conf pdns 
       config_get FORM_sdns $conf sdns 
       config_get FORM_pnbns $conf pnbns 
       config_get FORM_snbns $conf snbns 
       config_get FORM_ip $conf ip 
       config_get FORM_authentication $conf authentication 
       config_get FORM_username $conf username 
       config_get FORM_passwd $conf passwd 
       config_get FORM_simcardpin $conf simcardpin 
       config_get FORM_tech_mode $conf tech_mode 
       config_get FORM_ippassthrough $conf ippassthrough 
       config_get FORM_dnspassthrough $conf dnspassthrough 0 
       config_get FORM_carrier_selection $conf carrier_selection 0
       config_get FORM_fixed_carrier_id $conf carrier_id 0
       config_get FORM_search_carrier_id $conf carrier_id 0
       #added ip passthrough user assigned gateway and netMask
       config_get FORM_ippassth_mode $conf ippassth_mode 0
       config_get FORM_ippassth_GW $conf ippassth_GW 
       config_get FORM_ippassth_NM $conf ippassth_NM 
else
       FORM_disabled="$FORM_disabled_carrier"
       FORM_tech_mode="$FORM_tech_mode_carrier"
       FORM_apn="$FORM_apn_carrier"
       FORM_tech="$FORM_tech_carrier" 
       FORM_call_para="$FORM_call_para_carrier" 
       FORM_pdns="$FORM_pdns_carrier" 
       FORM_sdns="$FORM_sdns_carrier" 
       FORM_pnbns="$FORM_pnbns_carrier" 
       FORM_snbns="$FORM_sdns_carrier" 
       FORM_ip="$FORM_ip_carrier" 
       FORM_authentication="$FORM_authentication_carrier" 
       FORM_username="$FORM_username_carrier" 
       FORM_passwd="$FORM_passwd_carrier" 
       FORM_simcardpin="$FORM_simcardpin_carrier" 
       FORM_ippassthrough="$FORM_ippassthrough_carrier" 
       FORM_dnspassthrough="$FORM_dnspassthrough_carrier"
       FORM_carrier_selection="$FORM_carrier_selection_carrier"
       FORM_search_carrier_id="$FORM_search_carrier_id_carrier"
       FORM_fixed_carrier_id="$FORM_fixed_carrier_id_carrier"
        
       #added ip passthrough user assigned gateway and netMask
       FORM_ippassth_mode="$FORM_ippassth_mode_carrier"
       FORM_ippassth_GW="$FORM_ippassth_GW_carrier"
       FORM_ippassth_NM="$FORM_ippassth_NM_carrier"

       #validate ip passthrough user assigned gateway and netMask
       #try to get local IP address first
       if [ "$FORM_ippassth_mode" = "1" -a "$FORM_ippassthrough" = "Ethernet" ]; then
         eval err_check="$(netclass.sh E $FORM_ippassth_GW $FORM_ippassth_NM $wan_ip 0|sed 's/ERRORST=//g')"
         #right now do not do err check
         err_check=""
       fi
validate <<EOF
ip|FORM_ip_carrier|@TR<<IP Address>>||$FORM_ip
ip|FORM_ippassth_GW_carrier|@TR<<IP Passthrough Gateway>>||$FORM_ippassth_GW
ip|FORM_ippassth_NM_carrier|@TR<<IP Passthrough Netmask>>||$FORM_ippassth_NM
ip|FORM_pdns_carrier|@TR<<Primary DNS Address>>||$FORM_pdns
ip|FORM_sdns_carrier|@TR<<Secondary DNS Address>>||$FORM_sdns
ip|FORM_pnbns_carrier|@TR<<Primary NetBIOS Name Server Address>>||$FORM_pnbns
ip|FORM_snbns_carrier|@TR<<Secondary DNS Address>>||$FORM_snbns
int|FORM_call_para_carrier|@TR<<Data Call Parameters>>||$FORM_call_para
int|FORM_simcardpin_carrier|@TR<<SIM Pin>>||$FORM_simcardpin
EOF
       [ "$?" = "0" ] && [ "$err_check" = "" ] && {
            if [ "$FORM_disabled" = "1" ]; then
                # WAN maybe in bridge mode. Changing it to '4g' is a safe choice that means multiwan disabled.
                uci_set "multiwan" "config" "enabled" "0"
            #else
            #    uci_set "multiwan" "config" "enabled" "1"
            fi
            uci_set "lte" "connect" "disabled" "$FORM_disabled"
            uci_set "lte" "$conf" "apn" "$FORM_apn"
            uci_set "lte" "$conf" "tech" "$FORM_tech"
            uci_set "lte" "$conf" "call_para" "$FORM_call_para"
            uci_set "lte" "$conf" "pdns" "$FORM_pdns"
            uci_set "lte" "$conf" "sdns" "$FORM_sdns"
            uci_set "lte" "$conf" "pnbns" "$FORM_pnbns"
            uci_set "lte" "$conf" "snbns" "$FORM_snbns"
            uci_set "lte" "$conf" "ip" "$FORM_ip"
            uci_set "lte" "$conf" "authentication" "$FORM_authentication"
            uci_set "lte" "$conf" "username" "$FORM_username"
            uci_set "lte" "$conf" "passwd" "$FORM_passwd"
            uci_set "lte" "$conf" "simcardpin" "$FORM_simcardpin"
            uci_set "lte" "$conf" "tech_mode" "$FORM_tech_mode"
            uci_set "lte" "$conf" "ippassthrough" "$FORM_ippassthrough"
            uci_set "lte" "$conf" "dnspassthrough" "$FORM_dnspassthrough"
            FORM_datetime_time="`date +%T`"
            uci_set "lte" "$conf" "time" "$FORM_datetime_time"

            uci_set "lte" "$conf" "carrier_selection" "$FORM_carrier_selection"
            if [ "$FORM_carrier_selection" = "0" ]; then
                uci_set "lte" "$conf" "carrier_id" "0"
            elif [ "$FORM_carrier_selection" = "2" ]; then
                uci_set "lte" "$conf" "carrier_id" "$FORM_search_carrier_id"
            elif [ "$FORM_carrier_selection" = "3" ]; then
                uci_set "lte" "$conf" "carrier_id" "$FORM_fixed_carrier_id"
            fi

            uci_set "lte" "$conf" "ippassth_mode" "$FORM_ippassth_mode"
            uci_set "lte" "$conf" "ippassth_GW" "$FORM_ippassth_GW"
            uci_set "lte" "$conf" "ippassth_NM" "$FORM_ippassth_NM"
         } || {
            ERROR="${ERROR}IP Passthrough setting $err_check<br />"
         }
fi
   
if ! empty "$FORM_submit_search_carrier"; then
    FORM_carrier_selection=2 
fi   
   
if [ "$modem_type" = "E371" ];then
connect_options="start_form| @TR<<Configuration>>
field|@TR<<Carrier status>>
select|disabled_carrier|$FORM_disabled
option|0|@TR<<Enable>>
option|1|@TR<<Disable>>

field|@TR<<Carriers>>|carrier_selection_field|hidden
select|carrier_selection_carrier|$FORM_carrier_selection|0
option|0|@TR<<Auto>>
option|1|@TR<<Based on SIM>>
option|2|@TR<<Manual>>
option|3|@TR<<Fixed>>

field|@TR<<Carrier ID>>|search_carrier_id_field|hidden
"

if ! empty "$FORM_submit_search_carrier"; then
    opp=$(FORM_manual_options)    
    append connect_options "$opp" "$N"    
elif [ "$FORM_carrier_selection" = "2" ]; then  
    manual_options="select|search_carrier_id_carrier|$FORM_search_carrier_id
                    option|$FORM_search_carrier_id|@TR<<$FORM_search_carrier_id>>"
    append connect_options "$manual_options" "$N"    
else
    manual_options="select|search_carrier_id_carrier|$FORM_search_carrier_id
                    option|0|@TR<<------>>"
    append connect_options "$manual_options" "$N"
fi  

connect_options1="
field|@TR<<Carrier ID (default:0)>>|fixed_carrier_id_field|hidden
text|fixed_carrier_id_carrier|$FORM_fixed_carrier_id|example:302720

field|@TR<<IP-Passthrough>>|ippassthrough_field|hidden
select|ippassthrough_carrier|$FORM_ippassthrough
option|Disable|@TR<<Disable>>
option|Ethernet|@TR<<Ethernet>>
option|WANport|@TR<<WANport>>

field|@TR<<IP-Passthrough Mode>>|ippassthroughmode_field|hidden
select|ippassth_mode_carrier|$FORM_ippassth_mode
option|0|@TR<<Auto>>
option|1|@TR<<Manual>>

field|@TR<<IP-Passthrough Gateway>>|ippassthroughgw_field|hidden
text|ippassth_GW_carrier|$FORM_ippassth_GW

field|@TR<<IP-Passthrough Netmask>>|ippassthroughnm_field|hidden
text|ippassth_NM_carrier|$FORM_ippassth_NM

field|@TR<<IP-Passthrough Local IP>>|ippassthroughlo_field|hidden
string|$wan_ip

field|@TR<<DNS-Passthrough>>|dnspassthrough_field|hidden
select|dnspassthrough_carrier|$FORM_dnspassthrough
option|0|@TR<<Disable>>
option|1|@TR<<Enable>>

field|@TR<<APN>>|apn_carrier_field|hidden
text|apn_carrier|$FORM_apn

field|@TR<<SIM Pin>>|simcardpin_carrier_field|hidden
text|simcardpin_carrier|$FORM_simcardpin

field|@TR<<Technologies Type>>|tech_carrier_field|hidden
select|tech_carrier|$FORM_tech
option||@TR<<ALL>>
option|0|@TR<<3GPP>>
option|1|@TR<<3GPP2>>

field|@TR<<Technologies Mode>>|tech_mode_carrier_field|hidden
select|tech_mode_carrier|$FORM_tech_mode
option|auto|@TR<<AUTO>>
option|4g|@TR<<LTE Only>>
option|3g|@TR<<WCDMA Only>>
option|2g|@TR<<GSM Only>>

field|@TR<<Data Call Parameters>>|call_para_carrier_field|hidden
text|call_para_carrier|$FORM_call_para

field|@TR<<Primary DNS Address>>|pdns_carrier_field|hidden
text|pdns_carrier|$FORM_pdns

field|@TR<<Secondary DNS Address>>|sdns_carrier_field|hidden
text|sdns_carrier|$FORM_sdns

field|@TR<<Primary NetBIOS Name Server>>|pnbns_carrier_field|hidden
text|pnbns_carrier|$FORM_pnbns

field|@TR<<Secondary NetBIOS Server>>|snbns_carrier_field|hidden
text|snbns_carrier|$FORM_snbns

field|@TR<<IP Address>>|ip_carrier_field|hidden
text|ip_carrier|$FORM_ip

field|@TR<<Authentication>>|auth_carrier_field|hidden
select|authentication_carrier|$FORM_authentication
option|3|@TR<<Device decide>>
option|1|@TR<<PAP>>
option|2|@TR<<CHAP>>

field|@TR<<User Name>>|username_carrier_field|hidden
text|username_carrier|$FORM_username

field|@TR<<Password>>|passwd_carrier_field|hidden
text|passwd_carrier|$FORM_passwd
end_form"
else
connect_options="start_form| @TR<<Configuration>>
field|@TR<<Carrier status>>
select|disabled_carrier|$FORM_disabled
option|0|@TR<<Enable>>
option|1|@TR<<Disable>>

field|@TR<<Carriers>>|carrier_selection_field|hidden
select|carrier_selection_carrier|$FORM_carrier_selection|0
option|0|@TR<<Auto>>
option|1|@TR<<Based on SIM>>
option|2|@TR<<Manual>>
option|3|@TR<<Fixed>>

field|@TR<<Carrier ID>>|search_carrier_id_field|hidden
"

if ! empty "$FORM_submit_search_carrier"; then
    opp=$(FORM_manual_options)    
    append connect_options "$opp" "$N"    
elif [ "$FORM_carrier_selection" = "2" ]; then  
    manual_options="select|search_carrier_id_carrier|$FORM_search_carrier_id
                    option|$FORM_search_carrier_id|@TR<<$FORM_search_carrier_id>>"
    append connect_options "$manual_options" "$N"    
else
    manual_options="select|search_carrier_id_carrier|$FORM_search_carrier_id
                    option|0|@TR<<------>>"
    append connect_options "$manual_options" "$N"
fi  

connect_options1="
field|@TR<<Carrier ID (default:0)>>|fixed_carrier_id_field|hidden
text|fixed_carrier_id_carrier|$FORM_fixed_carrier_id|example:302720

field|@TR<<IP-Passthrough>>|ippassthrough_field|hidden
select|ippassthrough_carrier|$FORM_ippassthrough
option|Disable|@TR<<Disable>>
option|Ethernet|@TR<<Ethernet>>
option|WANport|@TR<<WANport>>

field|@TR<<DNS-Passthrough>>|dnspassthrough_field|hidden
select|dnspassthrough_carrier|$FORM_dnspassthrough
option|0|@TR<<Disable>>
option|1|@TR<<Enable>>

field|@TR<<APN>>|apn_carrier_field|hidden
text|apn_carrier|$FORM_apn

field|@TR<<SIM Pin>>|simcardpin_carrier_field|hidden
text|simcardpin_carrier|$FORM_simcardpin

field|@TR<<Technologies Type>>|tech_carrier_field|hidden
select|tech_carrier|$FORM_tech
option||@TR<<ALL>>
option|0|@TR<<3GPP>>
option|1|@TR<<3GPP2>>

field|@TR<<Technologies Mode>>|tech_mode_carrier_field|hidden
select|tech_mode_carrier|$FORM_tech_mode
option|auto|@TR<<AUTO>>
option|4g|@TR<<LTE Only>>
option|3g|@TR<<WCDMA Only>>
option|2g|@TR<<GSM Only>>
option|1x|@TR<<CDMA (1xRTT) Only>>
option|hdr|@TR<<EvDO Only>>

field|@TR<<Data Call Parameters>>|call_para_carrier_field|hidden
text|call_para_carrier|$FORM_call_para

field|@TR<<Primary DNS Address>>|pdns_carrier_field|hidden
text|pdns_carrier|$FORM_pdns

field|@TR<<Secondary DNS Address>>|sdns_carrier_field|hidden
text|sdns_carrier|$FORM_sdns

field|@TR<<Primary NetBIOS Name Server>>|pnbns_carrier_field|hidden
text|pnbns_carrier|$FORM_pnbns

field|@TR<<Secondary NetBIOS Server>>|snbns_carrier_field|hidden
text|snbns_carrier|$FORM_snbns

field|@TR<<IP Address>>|ip_carrier_field|hidden
text|ip_carrier|$FORM_ip

field|@TR<<Authentication>>|auth_carrier_field|hidden
select|authentication_carrier|$FORM_authentication
option|3|@TR<<Device decide>>
option|1|@TR<<PAP>>
option|2|@TR<<CHAP>>

field|@TR<<User Name>>|username_carrier_field|hidden
text|username_carrier|$FORM_username

field|@TR<<Password>>|passwd_carrier_field|hidden
text|passwd_carrier|$FORM_passwd
end_form"
fi
#append forms "$connect_options" "$N"

###################################################################
# set JavaScript
javascript_forms="
    v = isset('disabled_carrier', '1')
    set_visible('apn_carrier_field', !v);
    set_visible('ippassthrough_field', !v);
    set_visible('dnspassthrough_field', !v);
    set_visible('tech_carrier_field', !v);
    set_visible('call_para_carrier_field', !v);
    set_visible('pdns_carrier_field', !v);
    set_visible('sdns_carrier_field', !v);
    set_visible('pnbns_carrier_field', !v);
    set_visible('snbns_carrier_field', !v);
    set_visible('ip_carrier_field', !v);
    set_visible('auth_carrier_field', !v);
    set_visible('username_carrier_field', !v);
    set_visible('passwd_carrier_field', !v);
    set_visible('tech_mode_carrier_field', !v);
    set_visible('simcardpin_carrier_field', !v);

    set_visible('carrier_selection_field', !v);
    set_visible('search_carrier_id_field', !v);
    set_visible('fixed_carrier_id_field', !v); 
    
    if (isset('disabled_carrier', '0')){
        x = isset('carrier_selection_carrier', '2')
        set_visible('search_carrier_id_field', x);
        x = isset('carrier_selection_carrier', '3')
        set_visible('fixed_carrier_id_field', x);
    } 

    if (isset('disabled_carrier', '0')){
        if(isset('ippassthrough_carrier', 'Ethernet')){
            set_visible('ippassthroughmode_field', 1);
            x = isset('ippassth_mode_carrier', '1')
           set_visible('ippassthroughgw_field', x);
           set_visible('ippassthroughnm_field', x);
           set_visible('ippassthroughlo_field', x);
        } else {
            set_visible('ippassthroughmode_field', 0);
            set_visible('ippassthroughgw_field', 0);
            set_visible('ippassthroughnm_field', 0);
            set_visible('ippassthroughlo_field', 0);
        }

    } 

"

append js "$javascript_forms" "$N"

header "Carrier" "Settings" "@TR<<Carrier Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"


#####################################################################
#  Wait Search Carrier script
#

cat <<EOF
<script type="text/javascript" src="/js/waitbox.js"></script>
<script type="text/javascript">
<!--
function wait_search_carrier()
{
    showLightbox_VIP4G(document.getElementById('waitbox'));
}
-->
</script>

EOF

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
$connect_options
EOF

cat <<EOF
<input type="submit" class="buttons" name="submit_search_carrier" value="@TR<<Search Carrier>>" onclick="wait_search_carrier();"/></td></tr>
EOF

display_form <<EOF
onchange|modechange
$validate_error
$connect_options1
EOF

footer ?>
<!--
##WEBIF:name:Carrier:200:Settings
-->
