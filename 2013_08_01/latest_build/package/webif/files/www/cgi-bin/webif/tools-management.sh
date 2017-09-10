#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		system)
			hostname_cfg="$cfg_name"
		;;
		nms)
		        nms_cfg="$cfg_name"
		;;
		s2stunnel)
		    append s2s_cfgs "$cfg_name" "$N"
		;;
		x2ctunnel)
		    append x2c_cfgs "$cfg_name" "$N"
		;;
	esac
}
uci_load "ipsec"

uci_load "eurd"


conf_vpn="eur_conf_vpn"

if empty "$FORM_submit"; then

	conf_vpn="eur_conf_vpn"

else



    FORM_nmsserver=$(echo $FORM_nmsserver_nms)
    FORM_domainname=$(echo $FORM_domainname_nms)
    FORM_domainpasswd1="$FORM_domainpasswd1_nms"
    FORM_domainpasswd2="$FORM_domainpasswd2_nms"


       	config_get FORM_Event_Remote_Reporting_Status "eur_conf" Event_Remote_Reporting_Status
       	config_get FORM_domainpasswd0 "nms_conf" domainpasswd
        FORM_domainpasswd=$(echo $FORM_domainpasswd0 | cut -c-8)

	FORM_Event_Remote_Reporting_Status_0=$(echo $FORM_Event_Remote_Reporting_Status | cut -c1)
	FORM_Event_Remote_Reporting_Status_1=$(echo $FORM_Event_Remote_Reporting_Status | cut -c2)
	FORM_Event_Remote_Reporting_Status_2=$(echo $FORM_Event_Remote_Reporting_Status | cut -c3)

	FORM_Event_Remote_IP_address3="0.0.0.0"
 	FORM_Event_Remote_PORT3="$FORM_Event_Remote_PORT3_eurd"
	FORM_Event_Timer3="$FORM_Event_Timer3_eurd"
	FORM_Event_Management3_eth="$FORM_Event_Management3_eth_eurd"
	FORM_Event_Management3_carr="$FORM_Event_Management3_carr_eurd"
	FORM_Event_Management3_rad="$FORM_Event_Management3_rad_eurd"
	FORM_Event_Management3_com="$FORM_Event_Management3_com_eurd"
	FORM_Event_Management3_io="$FORM_Event_Management3_io_eurd"
	FORM_Event_Management3=$FORM_Event_Management3_eth$FORM_Event_Management3_carr$FORM_Event_Management3_rad$FORM_Event_Management3_com$FORM_Event_Management3_io"AAAAAAAAAAAAAAAA"
	FORM_Event_Remote_Reporting_Status_3="$FORM_Event_Remote_Reporting_Status_3_eurd"
	FORM_Event_Remote_Reporting_Status=$FORM_Event_Remote_Reporting_Status_0$FORM_Event_Remote_Reporting_Status_1$FORM_Event_Remote_Reporting_Status_2$FORM_Event_Remote_Reporting_Status_3
        FORM_Cell_Loc_Network="$FORM_Cell_Loc_Network_eurd"

	    FORM_type="$FORM_wsvrtype"
	    FORM_serverip="$FORM_nmsserver_nms"
	    FORM_port="$FORM_wsport"
	    FORM_usr="$FORM_wsusr"
	    FORM_passwd="$FORM_wspasswd"
	    FORM_period="$FORM_wsperiod"
	    FORM_status="$FORM_wstatus"

if [ "$FORM_domainpasswd1" != "$FORM_domainpasswd2" ]; then
    errormsg="<font color=#0000ff>Can't be saved. Please confirm with the same password.</font>"
else

validate <<EOF
string|FORM_nmsserver_nms|@TR<<Hostname>>|required|$FORM_nmsserver
string|FORM_domainname_nms|@TR<<Hostname>>|required|$FORM_domainname
string|FORM_domainpasswd1_nms|@TR<<Domain Password>>|required min=5|$FORM_domainpasswd1
port|FORM_wsport|@TR<<Server Port>>|required|$FORM_port
string|FORM_wsusr|@TR<<User Name>>|required|$FORM_usr
string|FORM_wspasswd|@TR<<Password>>|required|$FORM_passwd
int|FORM_Event_Remote_PORT3_eurd|@TR<<Remote Port3>>||$FORM_Event_Remote_PORT3
int|FORM_Event_Timer3_eurd|@TR<<Intervals Timer3>>||$FORM_Event_Timer3
EOF
           
       [ "$?" = "0" ] && {
 	    uci_set "wsclient" "general" "wsvrtype"      "$FORM_type"
            uci_set "wsclient" "general" "wserverip"     "$FORM_serverip"
            uci_set "wsclient" "general" "wsport"        "$FORM_port"
            uci_set "wsclient" "general" "wsusr"         "$FORM_usr"
            uci_set "wsclient" "general" "wspasswd"     "$FORM_passwd"
            if [ "$FORM_period" != "0" ]; then
                eval FORM_period=$(($FORM_period * 60))
            else
                FORM_period="60";
            fi 
            uci_set "wsclient" "general" "period"        "$FORM_period"
            uci_set "wsclient" "general" "status"        "$FORM_status"

            uci_set "eurd" "eur_conf" "Event_Remote_Reporting_Status" "$FORM_Event_Remote_Reporting_Status"
            uci_set "eurd" "eur_conf" "Event_Remote_IP_address3" "$FORM_Event_Remote_IP_address3"
            uci_set "eurd" "eur_conf" "Event_Remote_PORT3" "$FORM_Event_Remote_PORT3"
            uci_set "eurd" "eur_conf" "Event_Timer3" "$FORM_Event_Timer3"
            uci_set "eurd" "eur_conf" "Event_Management3" "$FORM_Event_Management3"
            uci_set "eurd" "eur_conf" "Cell_Loc_Network" "$FORM_Cell_Loc_Network"

	    uci_remove "eurd" "eur_conf_vpn3"
	    uci_add "eurd" "udprpt" "eur_conf_vpn3"
		for vpn_i in 3 ; do
			for s2s in $s2s_cfgs; do
				eval vpn_chk="\$FORM_Event_VPN${vpn_i}s_${s2s}_eurd"
				if [ "$vpn_chk" = "B" ]; then
				vpn_chk="B"
				else
				vpn_chk="A"
				fi    
			    	uci_set "eurd" "eur_conf_vpn3" "Event_VPN${vpn_i}s_${s2s}" "$vpn_chk"
			done
			for x2c in $x2c_cfgs; do
				eval vpn_chk="\$FORM_Event_VPN${vpn_i}c_${x2c}_eurd"
				if [ "$vpn_chk" = "B" ]; then
				vpn_chk="B"
				else
				vpn_chk="A"
				fi    
			    	uci_set "eurd" "eur_conf_vpn3" "Event_VPN${vpn_i}c_${x2c}" "$vpn_chk"
			done
		done

            if empty "$nms_cfg"; then
            	    uci_remove "eurd" "nms_conf"
            	    uci_add "eurd" "nms" "nms_conf"
                    FORM_domainpasswd="21232f29"
	            uci_set "eurd" "nms_conf" "domainpasswd"        "21232f297a57a5a743894a0e4a801fc3"
            fi
            uci_set "eurd" "nms_conf" "nmsserver"        "$FORM_nmsserver"
            uci_set "eurd" "nms_conf" "domainname"        "$FORM_domainname"
	    if [ "$FORM_domainpasswd " != "$FORM_domainpasswd1 " ]; then
		    FORM_domainpasswdnew=$(echo -n "$FORM_domainpasswd1" |md5sum | cut -c-32)
	            uci_set "eurd" "nms_conf" "domainpasswd"        "$FORM_domainpasswdnew"
	    fi

       }

fi
fi


if  [ "$FORM_setting" != "default" ];  then

uci_load "wsclient"
    config_get FORM_type            general wsvrtype
    config_get FORM_port            general wsport
    config_get FORM_usr             general wsusr
    config_get FORM_passwd          general wspasswd 
    config_get PERIOD               general period 300
    config_get FORM_status          general status
    eval FORM_period=$(($PERIOD / 60))


uci_load "eurd"
    config_get FORM_nmsserver	"nms_conf"	nmsserver
    config_get FORM_domainname	"nms_conf"	domainname
    config_get FORM_domainpasswd1	"nms_conf"	domainpasswd
    if empty "$FORM_nmsserver"; then
        FORM_nmsserver="nms.microhardcorp.com"
        FORM_domainname="default"
        FORM_domainpasswd1="21232f297a57a5a743894a0e4a801fc3"
    fi
    FORM_domainpasswd1=$(echo $FORM_domainpasswd1 | cut -c-8)

       config_get FORM_Event_Remote_Reporting_Status "eur_conf" Event_Remote_Reporting_Status
       config_get FORM_Event_Remote_PORT3 "eur_conf" Event_Remote_PORT3 
       config_get FORM_Event_Timer3 "eur_conf" Event_Timer3
       config_get FORM_Event_Management3 "eur_conf" Event_Management3
       config_get FORM_Cell_Loc_Network "eur_conf" Cell_Loc_Network

	FORM_Event_Management3_eth=$(echo $FORM_Event_Management3 | cut -c1)
	FORM_Event_Management3_carr=$(echo $FORM_Event_Management3 | cut -c2)
	FORM_Event_Management3_rad=$(echo $FORM_Event_Management3 | cut -c3)
	FORM_Event_Management3_com=$(echo $FORM_Event_Management3 | cut -c4)
	FORM_Event_Management3_io=$(echo $FORM_Event_Management3 | cut -c5)
	FORM_Event_Remote_Reporting_Status_3=$(echo $FORM_Event_Remote_Reporting_Status | cut -c4)
	if [ $FORM_Event_Remote_Reporting_Status_3 = "D" ]; then
		FORM_Event_Remote_Reporting_Status_3="D"
	else
		FORM_Event_Remote_Reporting_Status_3="A"
                FORM_Event_Timer3="300"
                FORM_Event_Remote_PORT3="20200"
                FORM_Event_Management3_eth="A"
                FORM_Event_Management3_carr="B"
                FORM_Event_Management3_rad="A"
                FORM_Event_Management3_com="A"
                FORM_Event_Management3_io="A"
	fi

else
        FORM_nmsserver="nms.microhardcorp.com"
        FORM_domainname="default"
        FORM_domainpasswd1="admin"

        FORM_Cell_Loc_Network="A"

	FORM_Event_Remote_Reporting_Status_3="D"
        FORM_Event_Timer3="300"
        FORM_Event_Remote_PORT3="20200"
        FORM_Event_Management3_eth="A"
        FORM_Event_Management3_carr="B"
        FORM_Event_Management3_rad="A"
        FORM_Event_Management3_com="A"
        FORM_Event_Management3_io="A"

        FORM_status="enable"
        FORM_type="https"        
        FORM_port="9998"
        FORM_usr="admin"
        FORM_passwd="admin"
        FORM_period="60"
fi


#string|@TR<<test text>>
config_options="
start_form
field|@TR<<Default Settings>>
string| <a href=tools-management.sh?setting=default>Edit with default configuration</a>
end_form
start_form|@TR<<System Setting>>
field|@TR<<NMS Server/IP>>
text|nmsserver_nms|$FORM_nmsserver| <a href=http://${FORM_nmsserver} target=_blank>Login NMS</a>
field|@TR<<Domain Name>>
text|domainname_nms|$FORM_domainname
field|@TR<<Domain Password>>
password|domainpasswd1_nms|$FORM_domainpasswd1|@TR<<Min 5 characters>>
field|@TR<<Confirm Password>>
password|domainpasswd2_nms|$FORM_domainpasswd1
end_form

start_form| @TR<<NMS Report Setting>>
field|@TR<< <strong>Carrier Location</strong> >>|Cell_Loc_Network_eurd_field|1
select|Cell_Loc_Network_eurd|$FORM_Cell_Loc_Network
option|A|@TR<<Disable Update Over Network>>
option|B|@TR<<Enable Update Over Network>>

field|@TR<< <strong>Report Status</strong> >>|Event_Remote_Reporting_Status_3_eurd_field|1
select|Event_Remote_Reporting_Status_3_eurd|$FORM_Event_Remote_Reporting_Status_3
option|A|@TR<<Disable NMS Report>>
option|D|@TR<<Enable NMS Report>>

field|@TR<<&nbsp;&nbsp;Remote PORT>>|Event_Remote_PORT3_eurd_field|hidden
text|Event_Remote_PORT3_eurd|$FORM_Event_Remote_PORT3|@TR<<[0 ~ 65535](default:20200)>>

field|@TR<<&nbsp;&nbsp;Interval Time(s)>>|Event_Timer3_eurd_field|hidden
text|Event_Timer3_eurd|$FORM_Event_Timer3|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Information Selection>>|Event_Management3_field|hidden
string|@TR<<Available Items:>>
field|@TR<<&nbsp;&nbsp;Ethernet:>>|Event_Management3_field1|hidden
radio|Event_Management3_eth_eurd|$FORM_Event_Management3_eth|A|@TR<<Disable>>
radio|Event_Management3_eth_eurd|$FORM_Event_Management3_eth|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;Carrier:>>|Event_Management3_field2|hidden
radio|Event_Management3_carr_eurd|$FORM_Event_Management3_carr|A|@TR<<Disable>>
radio|Event_Management3_carr_eurd|$FORM_Event_Management3_carr|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;Radio:>>|Event_Management3_field3|hidden
radio|Event_Management3_rad_eurd|$FORM_Event_Management3_rad|A|@TR<<Disable>>
radio|Event_Management3_rad_eurd|$FORM_Event_Management3_rad|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;Com:>>|Event_Management3_field4|hidden
radio|Event_Management3_com_eurd|$FORM_Event_Management3_com|A|@TR<<Disable>>
radio|Event_Management3_com_eurd|$FORM_Event_Management3_com|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;DI/DO:>>|Event_Management3_field5|hidden
radio|Event_Management3_io_eurd|$FORM_Event_Management3_io|A|@TR<<Disable>>
radio|Event_Management3_io_eurd|$FORM_Event_Management3_io|B|@TR<<Enable>>


"

vpn_i=3
check_vpn="A"
for s2s in $s2s_cfgs; do

    if  [ "$FORM_setting" != "default" ];  then
       	config_get check_vpn "eur_conf_vpn3" "Event_VPN${vpn_i}s_${s2s}"
	if [ "$check_vpn" = "B" ];then
		check_vpn="B"
       	else
	 	check_vpn="A"
        fi
    else
        check_vpn="A"
    fi
            
config_options=$config_options"
field|@TR<<&nbsp;&nbsp;VPN S-S:&nbsp;${s2s}>>|Event_VPN${vpn_i}s_${s2s}_field|hidden
radio|Event_VPN${vpn_i}s_${s2s}_eurd|$check_vpn|A|@TR<<Disable>>
radio|Event_VPN${vpn_i}s_${s2s}_eurd|$check_vpn|B|@TR<<Enable>>
"
done

check_vpn="A"
for x2c in $x2c_cfgs; do

    if  [ "$FORM_setting" != "default" ];  then
       	config_get check_vpn "eur_conf_vpn3" "Event_VPN${vpn_i}c_${x2c}"
	if [ "$check_vpn" = "B" ];then
		check_vpn="B"
       	else
	 	check_vpn="A"
        fi    

    else
        check_vpn="A"
    fi

config_options=$config_options"
field|@TR<<&nbsp;&nbsp;VPN C-S:&nbsp;${x2c}>>|Event_VPN${vpn_i}c_${x2c}_field|hidden
radio|Event_VPN${vpn_i}c_${x2c}_eurd|$check_vpn|A|@TR<<Disable>>
radio|Event_VPN${vpn_i}c_${x2c}_eurd|$check_vpn|B|@TR<<Enable>>
"
done

config_options=$config_options"

end_form


start_form|@TR<<Webclient Setting>>
field|@TR<< <strong>Status</strong>>>
select|wstatus|$FORM_status
option|enable|@TR<<Enable>>
option|disable|@TR<<Disable>>
field|@TR<<Server Type>>|wsvrtype_field|hidden
select|wsvrtype|$FORM_type
option|https|@TR<<HTTPS>>
option|http|@TR<<HTTP>>

field|@TR<<Server Port>>|wsport_field|hidden
text|wsport|$FORM_port
field|@TR<<User Name>>|wsusr_field|hidden
text|wsusr|$FORM_usr
field|@TR<<Password>>|wspasswd_field|hidden
password|wspasswd|$FORM_passwd

field|@TR<<Interval(min)>>|wsperiod_field|hidden
text|wsperiod|$FORM_period| [1...65535]
end_form

"

append forms "$config_options" "$N"

######################################################################################
# set JavaScript
javascript_forms="

    wc0 = isset('wstatus', 'enable')
    set_visible('wsvrtype_field', wc0);
    set_visible('wsport_field', wc0);
    set_visible('wsusr_field', wc0);
    set_visible('wspasswd_field', wc0);
    set_visible('wsperiod_field', wc0);


    v31 = isset('Event_Remote_Reporting_Status_3_eurd', 'A');
    set_visible('Event_Remote_PORT3_eurd_field', !v31);
    set_visible('Event_Timer3_eurd_field', !v31);
    v312 = isset('Event_Remote_Reporting_Status_3_eurd', 'D');
    set_visible('Event_Management3_field', v312);
    set_visible('Event_Management3_field1', v312);
    set_visible('Event_Management3_field2', v312);
    set_visible('Event_Management3_field3', v312);
    set_visible('Event_Management3_field4', v312);
    set_visible('Event_Management3_field5', v312);

"
for vpn_i in 3; do
for s2s in $s2s_cfgs; do
javascript_forms=$javascript_forms"
    set_visible('Event_VPN${vpn_i}s_${s2s}_field', v${vpn_i}12);
"
done
for x2c in $x2c_cfgs; do
javascript_forms=$javascript_forms"
    set_visible('Event_VPN${vpn_i}c_${x2c}_field', v${vpn_i}12);
"
done
done

append js "$javascript_forms" "$N"


#####################################################################
header "Tools" "NMS Settings" "@TR<<NMS Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"
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

echo $errormsg

display_form <<EOF
onchange|modechange
$validate_error
$forms
EOF

footer ?> 
<!--
##WEBIF:name:Tools:200:NMS Settings
-->

