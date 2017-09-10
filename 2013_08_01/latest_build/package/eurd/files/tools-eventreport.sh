#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"

#eval FORM_maxassoc="\$FORM_maxassoc_$device"  is ok.
#it must be radio for all vpn
config_cb() {
    local cfg_type="$1"
    local cfg_name="$2"

    case "$cfg_type" in
        s2stunnel)
            append s2s_cfgs "$cfg_name" "$N"
        ;;
        x2ctunnel)
            append x2c_cfgs "$cfg_name" "$N"
        ;;
        vca)
            append vca_cfgs "$cfg_name" "$N"
        ;;
    esac
}
uci_load ipsec

uci_load eurd
conf="eur_conf"
conf_vpn="eur_conf_vpn"

if empty "$FORM_submit"; then

	conf="eur_conf"
	conf_vpn="eur_conf_vpn"

else

 	FORM_Event_Remote_IP_address0="$FORM_Event_Remote_IP_address0_eurd"
        FORM_Event_Remote_IP_address1="$FORM_Event_Remote_IP_address1_eurd"
        FORM_Event_Remote_IP_address2="$FORM_Event_Remote_IP_address2_eurd"
        FORM_Event_Remote_IP_address3="$FORM_Event_Remote_IP_address3_eurd"
 	FORM_Event_Remote_PORT0="$FORM_Event_Remote_PORT0_eurd"
 	FORM_Event_Remote_PORT1="$FORM_Event_Remote_PORT1_eurd"
 	FORM_Event_Remote_PORT2="$FORM_Event_Remote_PORT2_eurd"
 	FORM_Event_Remote_PORT3="$FORM_Event_Remote_PORT3_eurd"
	FORM_Event_Timer0="$FORM_Event_Timer0_eurd"
	FORM_Event_Timer1="$FORM_Event_Timer1_eurd"
	FORM_Event_Timer2="$FORM_Event_Timer2_eurd"
	FORM_Event_Timer3="$FORM_Event_Timer3_eurd"
	FORM_Event_Remote_Reporting_Status_0="$FORM_Event_Remote_Reporting_Status_0_eurd"
	FORM_Event_Remote_Reporting_Status_1="$FORM_Event_Remote_Reporting_Status_1_eurd"
	FORM_Event_Remote_Reporting_Status_2="$FORM_Event_Remote_Reporting_Status_2_eurd"
	FORM_Event_Remote_Reporting_Status_3="$FORM_Event_Remote_Reporting_Status_3_eurd"
	FORM_Event_Remote_Reporting_Status=$FORM_Event_Remote_Reporting_Status_0$FORM_Event_Remote_Reporting_Status_1$FORM_Event_Remote_Reporting_Status_2$FORM_Event_Remote_Reporting_Status_3
	FORM_Event_Message_type0_0="$FORM_Event_Message_type0_0_eurd"
	FORM_Event_Message_type0_1="$FORM_Event_Message_type0_1_eurd"
	FORM_Event_Message_type0_2="$FORM_Event_Message_type0_2_eurd"
	FORM_Event_Message_type0_3="$FORM_Event_Message_type0_3_eurd"
	FORM_Event_Message_type1_0="$FORM_Event_Message_type1_0_eurd"
	FORM_Event_Message_type1_1="$FORM_Event_Message_type1_1_eurd"
	FORM_Event_Message_type1_2="$FORM_Event_Message_type1_2_eurd"
	FORM_Event_Message_type1_3="$FORM_Event_Message_type1_3_eurd"
	FORM_Event_Message_type2_0="$FORM_Event_Message_type2_0_eurd"
	FORM_Event_Message_type2_1="$FORM_Event_Message_type2_1_eurd"
	FORM_Event_Message_type2_2="$FORM_Event_Message_type2_2_eurd"
	FORM_Event_Message_type2_3="$FORM_Event_Message_type2_3_eurd"
	FORM_Event_Message_type0=$FORM_Event_Message_type0_0$FORM_Event_Message_type0_1$FORM_Event_Message_type0_2$FORM_Event_Message_type0_3
	FORM_Event_Message_type1=$FORM_Event_Message_type1_0$FORM_Event_Message_type1_1$FORM_Event_Message_type1_2$FORM_Event_Message_type1_3
	FORM_Event_Message_type2=$FORM_Event_Message_type2_0$FORM_Event_Message_type2_1$FORM_Event_Message_type2_2$FORM_Event_Message_type2_3
	FORM_Event_Management0_eth="$FORM_Event_Management0_eth_eurd"
	FORM_Event_Management0_carr="$FORM_Event_Management0_carr_eurd"
	FORM_Event_Management0_rad="$FORM_Event_Management0_rad_eurd"
	FORM_Event_Management0_com="$FORM_Event_Management0_com_eurd"
	FORM_Event_Management0_io="$FORM_Event_Management0_io_eurd"
	FORM_Event_Management0=$FORM_Event_Management0_eth$FORM_Event_Management0_carr$FORM_Event_Management0_rad$FORM_Event_Management0_com$FORM_Event_Management0_io"AAAAAAAAAAAAAAAA"
	FORM_Event_Management1_eth="$FORM_Event_Management1_eth_eurd"
	FORM_Event_Management1_carr="$FORM_Event_Management1_carr_eurd"
	FORM_Event_Management1_rad="$FORM_Event_Management1_rad_eurd"
	FORM_Event_Management1_com="$FORM_Event_Management1_com_eurd"
	FORM_Event_Management1_io="$FORM_Event_Management1_io_eurd"
	FORM_Event_Management1=$FORM_Event_Management1_eth$FORM_Event_Management1_carr$FORM_Event_Management1_rad$FORM_Event_Management1_com$FORM_Event_Management1_io"AAAAAAAAAAAAAAAA"
	FORM_Event_Management2_eth="$FORM_Event_Management2_eth_eurd"
	FORM_Event_Management2_carr="$FORM_Event_Management2_carr_eurd"
	FORM_Event_Management2_rad="$FORM_Event_Management2_rad_eurd"
	FORM_Event_Management2_com="$FORM_Event_Management2_com_eurd"
	FORM_Event_Management2_io="$FORM_Event_Management2_io_eurd"
	FORM_Event_Management2=$FORM_Event_Management2_eth$FORM_Event_Management2_carr$FORM_Event_Management2_rad$FORM_Event_Management2_com$FORM_Event_Management2_io"AAAAAAAAAAAAAAAA"
	FORM_Event_Management3_eth="$FORM_Event_Management3_eth_eurd"
	FORM_Event_Management3_carr="$FORM_Event_Management3_carr_eurd"
	FORM_Event_Management3_rad="$FORM_Event_Management3_rad_eurd"
	FORM_Event_Management3_com="$FORM_Event_Management3_com_eurd"
	FORM_Event_Management3_io="$FORM_Event_Management3_io_eurd"
	FORM_Event_Management3=$FORM_Event_Management3_eth$FORM_Event_Management3_carr$FORM_Event_Management3_rad$FORM_Event_Management3_com$FORM_Event_Management3_io"AAAAAAAAAAAAAAAA"


validate <<EOF
ip|FORM_Event_Remote_IP_address0_eurd|@TR<<Remote IP0>>||$FORM_Event_Remote_IP_address0
ip|FORM_Event_Remote_IP_address1_eurd|@TR<<Remote IP1>>||$FORM_Event_Remote_IP_address1
ip|FORM_Event_Remote_IP_address2_eurd|@TR<<Remote IP2>>||$FORM_Event_Remote_IP_address2
ip|FORM_Event_Remote_IP_address3_eurd|@TR<<Remote IP3>>||$FORM_Event_Remote_IP_address3
int|FORM_Event_Remote_PORT0_eurd|@TR<<Remote Port0>>||$FORM_Event_Remote_PORT0
int|FORM_Event_Remote_PORT1_eurd|@TR<<Remote Port1>>||$FORM_Event_Remote_PORT1
int|FORM_Event_Remote_PORT2_eurd|@TR<<Remote Port2>>||$FORM_Event_Remote_PORT2
int|FORM_Event_Remote_PORT3_eurd|@TR<<Remote Port3>>||$FORM_Event_Remote_PORT3
int|FORM_Event_Timer0_eurd|@TR<<Intervals Timer0>>||$FORM_Event_Timer0
int|FORM_Event_Timer1_eurd|@TR<<Intervals Timer1>>||$FORM_Event_Timer1
int|FORM_Event_Timer2_eurd|@TR<<Intervals Timer2>>||$FORM_Event_Timer2
int|FORM_Event_Timer3_eurd|@TR<<Intervals Timer3>>||$FORM_Event_Timer3
EOF


       [ "$?" = "0" ] && {

            uci_set "eurd" "$conf" "Event_Remote_Reporting_Status" "$FORM_Event_Remote_Reporting_Status"
            uci_set "eurd" "$conf" "Event_Remote_IP_address0" "$FORM_Event_Remote_IP_address0"
            uci_set "eurd" "$conf" "Event_Remote_IP_address1" "$FORM_Event_Remote_IP_address1"
            uci_set "eurd" "$conf" "Event_Remote_IP_address2" "$FORM_Event_Remote_IP_address2"
            uci_set "eurd" "$conf" "Event_Remote_PORT0" "$FORM_Event_Remote_PORT0"
            uci_set "eurd" "$conf" "Event_Remote_PORT1" "$FORM_Event_Remote_PORT1"
            uci_set "eurd" "$conf" "Event_Remote_PORT2" "$FORM_Event_Remote_PORT2"
            uci_set "eurd" "$conf" "Event_Timer0" "$FORM_Event_Timer0"
            uci_set "eurd" "$conf" "Event_Timer1" "$FORM_Event_Timer1"
            uci_set "eurd" "$conf" "Event_Timer2" "$FORM_Event_Timer2"
            uci_set "eurd" "$conf" "Event_Message_type0" "$FORM_Event_Message_type0"
            uci_set "eurd" "$conf" "Event_Message_type1" "$FORM_Event_Message_type1"
            uci_set "eurd" "$conf" "Event_Message_type2" "$FORM_Event_Message_type2"
            uci_set "eurd" "$conf" "Event_Management0" "$FORM_Event_Management0"
            uci_set "eurd" "$conf" "Event_Management1" "$FORM_Event_Management1"
            uci_set "eurd" "$conf" "Event_Management2" "$FORM_Event_Management2"

	    uci_remove "eurd" "eur_conf_vpn0"
	    uci_remove "eurd" "eur_conf_vpn1"
	    uci_remove "eurd" "eur_conf_vpn2"
	    uci_add "eurd" "udprpt" "eur_conf_vpn0"
	    uci_add "eurd" "udprpt" "eur_conf_vpn1"
	    uci_add "eurd" "udprpt" "eur_conf_vpn2"
		for vpn_i in 0 1 2 ; do
			for s2s in $s2s_cfgs; do
				eval vpn_chk="\$FORM_Event_VPN${vpn_i}s_${s2s}_eurd"
				if [ "$vpn_chk" = "B" ]; then
				vpn_chk="B"
				else
				vpn_chk="A"
				fi    
			    	uci_set "eurd" "eur_conf_vpn${vpn_i}" "Event_VPN${vpn_i}s_${s2s}" "$vpn_chk"
			done
			for x2c in $x2c_cfgs; do
				eval vpn_chk="\$FORM_Event_VPN${vpn_i}c_${x2c}_eurd"
				if [ "$vpn_chk" = "B" ]; then
				vpn_chk="B"
				else
				vpn_chk="A"
				fi    
			    	uci_set "eurd" "eur_conf_vpn${vpn_i}" "Event_VPN${vpn_i}c_${x2c}" "$vpn_chk"
			done
		done

         }
fi



uci_load "eurd"
       config_get FORM_Event_Remote_Reporting_Status $conf Event_Remote_Reporting_Status
       config_get FORM_Event_Remote_IP_address0 $conf Event_Remote_IP_address0 
       config_get FORM_Event_Remote_IP_address1 $conf Event_Remote_IP_address1 
       config_get FORM_Event_Remote_IP_address2 $conf Event_Remote_IP_address2 
       config_get FORM_Event_Remote_IP_address3 $conf Event_Remote_IP_address3 
       config_get FORM_Event_Remote_PORT0 $conf Event_Remote_PORT0 
       config_get FORM_Event_Remote_PORT1 $conf Event_Remote_PORT1 
       config_get FORM_Event_Remote_PORT2 $conf Event_Remote_PORT2 
       config_get FORM_Event_Remote_PORT3 $conf Event_Remote_PORT3 
       config_get FORM_Event_Timer0 $conf Event_Timer0
       config_get FORM_Event_Timer1 $conf Event_Timer1
       config_get FORM_Event_Timer2 $conf Event_Timer2
       config_get FORM_Event_Timer3 $conf Event_Timer3
       config_get FORM_Event_Message_type0 $conf Event_Message_type0
       config_get FORM_Event_Message_type1 $conf Event_Message_type1
       config_get FORM_Event_Message_type2 $conf Event_Message_type2
       config_get FORM_Event_Management0 $conf Event_Management0
       config_get FORM_Event_Management1 $conf Event_Management1
       config_get FORM_Event_Management2 $conf Event_Management2
       config_get FORM_Event_Management3 $conf Event_Management3

FORM_Event_Remote_Reporting_Status_0=$(echo $FORM_Event_Remote_Reporting_Status | cut -c1)
FORM_Event_Remote_Reporting_Status_1=$(echo $FORM_Event_Remote_Reporting_Status | cut -c2)
FORM_Event_Remote_Reporting_Status_2=$(echo $FORM_Event_Remote_Reporting_Status | cut -c3)
FORM_Event_Remote_Reporting_Status_3=$(echo $FORM_Event_Remote_Reporting_Status | cut -c4)
FORM_Event_Message_type0_0=$(echo $FORM_Event_Message_type0 | cut -c1)
FORM_Event_Message_type0_1=$(echo $FORM_Event_Message_type0 | cut -c2)
FORM_Event_Message_type0_2=$(echo $FORM_Event_Message_type0 | cut -c3)
FORM_Event_Message_type0_3=$(echo $FORM_Event_Message_type0 | cut -c4)
FORM_Event_Message_type1_0=$(echo $FORM_Event_Message_type1 | cut -c1)
FORM_Event_Message_type1_1=$(echo $FORM_Event_Message_type1 | cut -c2)
FORM_Event_Message_type1_2=$(echo $FORM_Event_Message_type1 | cut -c3)
FORM_Event_Message_type1_3=$(echo $FORM_Event_Message_type1 | cut -c4)
FORM_Event_Message_type2_0=$(echo $FORM_Event_Message_type2 | cut -c1)
FORM_Event_Message_type2_1=$(echo $FORM_Event_Message_type2 | cut -c2)
FORM_Event_Message_type2_2=$(echo $FORM_Event_Message_type2 | cut -c3)
FORM_Event_Message_type2_3=$(echo $FORM_Event_Message_type2 | cut -c4)
FORM_Event_Management0_eth=$(echo $FORM_Event_Management0 | cut -c1)
FORM_Event_Management0_carr=$(echo $FORM_Event_Management0 | cut -c2)
FORM_Event_Management0_rad=$(echo $FORM_Event_Management0 | cut -c3)
FORM_Event_Management0_com=$(echo $FORM_Event_Management0 | cut -c4)
FORM_Event_Management0_io=$(echo $FORM_Event_Management0 | cut -c5)

FORM_Event_Management1_eth=$(echo $FORM_Event_Management1 | cut -c1)
FORM_Event_Management1_carr=$(echo $FORM_Event_Management1 | cut -c2)
FORM_Event_Management1_rad=$(echo $FORM_Event_Management1 | cut -c3)
FORM_Event_Management1_com=$(echo $FORM_Event_Management1 | cut -c4)
FORM_Event_Management1_io=$(echo $FORM_Event_Management1 | cut -c5)

FORM_Event_Management2_eth=$(echo $FORM_Event_Management2 | cut -c1)
FORM_Event_Management2_carr=$(echo $FORM_Event_Management2 | cut -c2)
FORM_Event_Management2_rad=$(echo $FORM_Event_Management2 | cut -c3)
FORM_Event_Management2_com=$(echo $FORM_Event_Management2 | cut -c4)
FORM_Event_Management2_io=$(echo $FORM_Event_Management2 | cut -c5)

FORM_Event_Management3_eth=$(echo $FORM_Event_Management3 | cut -c1)
FORM_Event_Management3_carr=$(echo $FORM_Event_Management3 | cut -c2)
FORM_Event_Management3_rad=$(echo $FORM_Event_Management3 | cut -c3)
FORM_Event_Management3_com=$(echo $FORM_Event_Management3 | cut -c4)
FORM_Event_Management3_io=$(echo $FORM_Event_Management3 | cut -c5)


config_options="start_form| @TR<<Report Configuration No.1>>
field|@TR<< <strong>Event Type</strong> >>|Event_Remote_Reporting_Status_0_eurd_field|1
select|Event_Remote_Reporting_Status_0_eurd|$FORM_Event_Remote_Reporting_Status_0
option|A|@TR<<Disable>>
option|B|@TR<<Modem_Event>>
option|C|@TR<<SDP_Event>>
option|D|@TR<<Management>>

field|@TR<<&nbsp;&nbsp;Remote IP>>|Event_Remote_IP_address0_eurd_field|hidden
text|Event_Remote_IP_address0_eurd|$FORM_Event_Remote_IP_address0|@TR<<0.0.0.0>>

field|@TR<<&nbsp;&nbsp;Remote PORT>>|Event_Remote_PORT0_eurd_field|hidden
text|Event_Remote_PORT0_eurd|$FORM_Event_Remote_PORT0|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Interval Time(s)>>|Event_Timer0_eurd_field|hidden
text|Event_Timer0_eurd|$FORM_Event_Timer0|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Message Info Type>>|Event_Message_type_0_eurd_field|hidden
select|Event_Message_type0_0_eurd|$FORM_Event_Message_type0_0
option|A|@TR<<None>>
option|B|@TR<<Modem>>
option|C|@TR<<Carrier>>
option|D|@TR<<Wan>>

select|Event_Message_type1_0_eurd|$FORM_Event_Message_type1_0
option|A|@TR<<None>>
option|B|@TR<<Modem>>
option|C|@TR<<Carrier>>
option|D|@TR<<Wan>>

select|Event_Message_type2_0_eurd|$FORM_Event_Message_type2_0
option|A|@TR<<None>>
option|B|@TR<<Modem>>
option|C|@TR<<Carrier>>
option|D|@TR<<Wan>>

field|@TR<<&nbsp;&nbsp;Interface Selection>>|Event_Management0_field|hidden
field|@TR<<&nbsp;&nbsp;Ethernet:>>|Event_Management0_field1|hidden
radio|Event_Management0_eth_eurd|$FORM_Event_Management0_eth|A|@TR<<Disable>>
radio|Event_Management0_eth_eurd|$FORM_Event_Management0_eth|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;Carrier:>>|Event_Management0_field2|hidden
radio|Event_Management0_carr_eurd|$FORM_Event_Management0_carr|A|@TR<<Disable>>
radio|Event_Management0_carr_eurd|$FORM_Event_Management0_carr|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;Radio:>>|Event_Management0_field3|hidden
radio|Event_Management0_rad_eurd|$FORM_Event_Management0_rad|A|@TR<<Disable>>
radio|Event_Management0_rad_eurd|$FORM_Event_Management0_rad|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;Com:>>|Event_Management0_field4|hidden
radio|Event_Management0_com_eurd|$FORM_Event_Management0_com|A|@TR<<Disable>>
radio|Event_Management0_com_eurd|$FORM_Event_Management0_com|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;DI/DO:>>|Event_Management0_field5|hidden
radio|Event_Management0_io_eurd|$FORM_Event_Management0_io|A|@TR<<Disable>>
radio|Event_Management0_io_eurd|$FORM_Event_Management0_io|B|@TR<<Enable>>

"

vpn_i=0
check_vpn=""
for s2s in $s2s_cfgs; do
       	config_get check_vpn "eur_conf_vpn${vpn_i}" "Event_VPN${vpn_i}s_${s2s}"
	if [ "$check_vpn" = "B" ];then
		check_vpn="B"
       	else
	 	check_vpn="A"
        fi    
config_options=$config_options"
field|@TR<<&nbsp;&nbsp;VPN S-S:&nbsp;${s2s}>>|Event_VPN${vpn_i}s_${s2s}_field|1
radio|Event_VPN${vpn_i}s_${s2s}_eurd|$check_vpn|A|@TR<<Disable>>
radio|Event_VPN${vpn_i}s_${s2s}_eurd|$check_vpn|B|@TR<<Enable>>
"
done

check_vpn=""
for x2c in $x2c_cfgs; do
       	config_get check_vpn "eur_conf_vpn${vpn_i}" "Event_VPN${vpn_i}c_${x2c}"
	if [ "$check_vpn" = "B" ];then
		check_vpn="B"
       	else
	 	check_vpn="A"
        fi    
config_options=$config_options"
field|@TR<<&nbsp;&nbsp;VPN C-S:&nbsp;${x2c}>>|Event_VPN${vpn_i}c_${x2c}_field|1
radio|Event_VPN${vpn_i}c_${x2c}_eurd|$check_vpn|A|@TR<<Disable>>
radio|Event_VPN${vpn_i}c_${x2c}_eurd|$check_vpn|B|@TR<<Enable>>
"
done


config_options=$config_options"
end_form

start_form| @TR<<Report Configuration No.2>>
field|@TR<< <strong>Event Type</strong> >>|Event_Remote_Reporting_Status_1_eurd_field|1
select|Event_Remote_Reporting_Status_1_eurd|$FORM_Event_Remote_Reporting_Status_1
option|A|@TR<<Disable>>
option|B|@TR<<Modem_Event>>
option|C|@TR<<SDP_Event>>
option|D|@TR<<Management>>

field|@TR<<&nbsp;&nbsp;Remote IP>>|Event_Remote_IP_address1_eurd_field|hidden
text|Event_Remote_IP_address1_eurd|$FORM_Event_Remote_IP_address1|@TR<<0.0.0.0>>

field|@TR<<&nbsp;&nbsp;Remote PORT>>|Event_Remote_PORT1_eurd_field|hidden
text|Event_Remote_PORT1_eurd|$FORM_Event_Remote_PORT1|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Interval Time(s)>>|Event_Timer1_eurd_field|hidden
text|Event_Timer1_eurd|$FORM_Event_Timer1|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Message Info Type>>|Event_Message_type_1_eurd_field|hidden
select|Event_Message_type0_1_eurd|$FORM_Event_Message_type0_1
option|A|@TR<<None>>
option|B|@TR<<Modem>>
option|C|@TR<<Carrier>>
option|D|@TR<<Wan>>

select|Event_Message_type1_1_eurd|$FORM_Event_Message_type1_1
option|A|@TR<<None>>
option|B|@TR<<Modem>>
option|C|@TR<<Carrier>>
option|D|@TR<<Wan>>

select|Event_Message_type2_1_eurd|$FORM_Event_Message_type2_1
option|A|@TR<<None>>
option|B|@TR<<Modem>>
option|C|@TR<<Carrier>>
option|D|@TR<<Wan>>

field|@TR<<&nbsp;&nbsp;Interface Selection>>|Event_Management1_field|hidden
field|@TR<<&nbsp;&nbsp;Ethernet:>>|Event_Management1_field1|hidden
radio|Event_Management1_eth_eurd|$FORM_Event_Management1_eth|A|@TR<<Disable>>
radio|Event_Management1_eth_eurd|$FORM_Event_Management1_eth|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;Carrier:>>|Event_Management1_field2|hidden
radio|Event_Management1_carr_eurd|$FORM_Event_Management1_carr|A|@TR<<Disable>>
radio|Event_Management1_carr_eurd|$FORM_Event_Management1_carr|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;Radio:>>|Event_Management1_field3|hidden
radio|Event_Management1_rad_eurd|$FORM_Event_Management1_rad|A|@TR<<Disable>>
radio|Event_Management1_rad_eurd|$FORM_Event_Management1_rad|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;Com:>>|Event_Management1_field4|hidden
radio|Event_Management1_com_eurd|$FORM_Event_Management1_com|A|@TR<<Disable>>
radio|Event_Management1_com_eurd|$FORM_Event_Management1_com|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;DI/DO:>>|Event_Management1_field5|hidden
radio|Event_Management1_io_eurd|$FORM_Event_Management1_io|A|@TR<<Disable>>
radio|Event_Management1_io_eurd|$FORM_Event_Management1_io|B|@TR<<Enable>>

"

vpn_i=1
check_vpn=""
for s2s in $s2s_cfgs; do
       	config_get check_vpn "eur_conf_vpn${vpn_i}" "Event_VPN${vpn_i}s_${s2s}"
	if [ "$check_vpn" = "B" ];then
		check_vpn="B"
       	else
	 	check_vpn="A"
        fi    
config_options=$config_options"
field|@TR<<&nbsp;&nbsp;VPN S-S:&nbsp;${s2s}>>|Event_VPN${vpn_i}s_${s2s}_field|1
radio|Event_VPN${vpn_i}s_${s2s}_eurd|$check_vpn|A|@TR<<Disable>>
radio|Event_VPN${vpn_i}s_${s2s}_eurd|$check_vpn|B|@TR<<Enable>>
"
done

check_vpn=""
for x2c in $x2c_cfgs; do
       	config_get check_vpn "eur_conf_vpn${vpn_i}" "Event_VPN${vpn_i}c_${x2c}"
	if [ "$check_vpn" = "B" ];then
		check_vpn="B"
       	else
	 	check_vpn="A"
        fi    
config_options=$config_options"
field|@TR<<&nbsp;&nbsp;VPN C-S:&nbsp;${x2c}>>|Event_VPN${vpn_i}c_${x2c}_field|1
radio|Event_VPN${vpn_i}c_${x2c}_eurd|$check_vpn|A|@TR<<Disable>>
radio|Event_VPN${vpn_i}c_${x2c}_eurd|$check_vpn|B|@TR<<Enable>>
"
done

config_options=$config_options"
end_form

start_form| @TR<<Report Configuration No.3>>
field|@TR<< <strong>Event Type</strong> >>|Event_Remote_Reporting_Status_2_eurd_field|1
select|Event_Remote_Reporting_Status_2_eurd|$FORM_Event_Remote_Reporting_Status_2
option|A|@TR<<Disable>>
option|B|@TR<<Modem_Event>>
option|C|@TR<<SDP_Event>>
option|D|@TR<<Management>>

field|@TR<<&nbsp;&nbsp;Remote IP>>|Event_Remote_IP_address2_eurd_field|hidden
text|Event_Remote_IP_address2_eurd|$FORM_Event_Remote_IP_address2|@TR<<0.0.0.0>>

field|@TR<<&nbsp;&nbsp;Remote PORT>>|Event_Remote_PORT2_eurd_field|hidden
text|Event_Remote_PORT2_eurd|$FORM_Event_Remote_PORT2|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Interval Time(s)>>|Event_Timer2_eurd_field|hidden
text|Event_Timer2_eurd|$FORM_Event_Timer2|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Message Info Type>>|Event_Message_type_2_eurd_field|hidden
select|Event_Message_type0_2_eurd|$FORM_Event_Message_type0_2
option|A|@TR<<None>>
option|B|@TR<<Modem>>
option|C|@TR<<Carrier>>
option|D|@TR<<Wan>>

select|Event_Message_type1_2_eurd|$FORM_Event_Message_type1_2
option|A|@TR<<None>>
option|B|@TR<<Modem>>
option|C|@TR<<Carrier>>
option|D|@TR<<Wan>>

select|Event_Message_type2_2_eurd|$FORM_Event_Message_type2_2
option|A|@TR<<None>>
option|B|@TR<<Modem>>
option|C|@TR<<Carrier>>
option|D|@TR<<Wan>>

field|@TR<<&nbsp;&nbsp;Interface Selection>>|Event_Management2_field|hidden
field|@TR<<&nbsp;&nbsp;Ethernet:>>|Event_Management2_field1|hidden
radio|Event_Management2_eth_eurd|$FORM_Event_Management2_eth|A|@TR<<Disable>>
radio|Event_Management2_eth_eurd|$FORM_Event_Management2_eth|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;Carrier:>>|Event_Management2_field2|hidden
radio|Event_Management2_carr_eurd|$FORM_Event_Management2_carr|A|@TR<<Disable>>
radio|Event_Management2_carr_eurd|$FORM_Event_Management2_carr|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;Radio:>>|Event_Management2_field3|hidden
radio|Event_Management2_rad_eurd|$FORM_Event_Management2_rad|A|@TR<<Disable>>
radio|Event_Management2_rad_eurd|$FORM_Event_Management2_rad|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;Com:>>|Event_Management2_field4|hidden
radio|Event_Management2_com_eurd|$FORM_Event_Management2_com|A|@TR<<Disable>>
radio|Event_Management2_com_eurd|$FORM_Event_Management2_com|B|@TR<<Enable>>
field|@TR<<&nbsp;&nbsp;DI/DO:>>|Event_Management2_field5|hidden
radio|Event_Management2_io_eurd|$FORM_Event_Management2_io|A|@TR<<Disable>>
radio|Event_Management2_io_eurd|$FORM_Event_Management2_io|B|@TR<<Enable>>

"

vpn_i=2
check_vpn=""
for s2s in $s2s_cfgs; do
       	config_get check_vpn "eur_conf_vpn${vpn_i}" "Event_VPN${vpn_i}s_${s2s}"
	if [ "$check_vpn" = "B" ];then
		check_vpn="B"
       	else
	 	check_vpn="A"
        fi    
config_options=$config_options"
field|@TR<<&nbsp;&nbsp;VPN S-S:&nbsp;${s2s}>>|Event_VPN${vpn_i}s_${s2s}_field|1
radio|Event_VPN${vpn_i}s_${s2s}_eurd|$check_vpn|A|@TR<<Disable>>
radio|Event_VPN${vpn_i}s_${s2s}_eurd|$check_vpn|B|@TR<<Enable>>
"
done

check_vpn=""
for x2c in $x2c_cfgs; do
       	config_get check_vpn "eur_conf_vpn${vpn_i}" "Event_VPN${vpn_i}c_${x2c}"
	if [ "$check_vpn" = "B" ];then
		check_vpn="B"
       	else
	 	check_vpn="A"
        fi    
config_options=$config_options"
field|@TR<<&nbsp;&nbsp;VPN C-S:&nbsp;${x2c}>>|Event_VPN${vpn_i}c_${x2c}_field|1
radio|Event_VPN${vpn_i}c_${x2c}_eurd|$check_vpn|A|@TR<<Disable>>
radio|Event_VPN${vpn_i}c_${x2c}_eurd|$check_vpn|B|@TR<<Enable>>
"
done

#end_form
#start_form| @TR<<Report Configuration No.4>>

config_options=$config_options"
field|@TR<< <strong>Event Type</strong> >>|Event_Remote_Reporting_Status_3_eurd_field|hidden
select|Event_Remote_Reporting_Status_3_eurd|$FORM_Event_Remote_Reporting_Status_3
option|A|@TR<<Disable>>
option|B|@TR<<Modem_Event>>
option|C|@TR<<SDP_Event>>
option|D|@TR<<Management>>

field|@TR<<&nbsp;&nbsp;Remote IP>>|Event_Remote_IP_address3_eurd_field|hidden
text|Event_Remote_IP_address3_eurd|$FORM_Event_Remote_IP_address3|@TR<<0.0.0.0>>

field|@TR<<&nbsp;&nbsp;Remote PORT>>|Event_Remote_PORT3_eurd_field|hidden
text|Event_Remote_PORT3_eurd|$FORM_Event_Remote_PORT3|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Interval Time(s)>>|Event_Timer3_eurd_field|hidden
text|Event_Timer3_eurd|$FORM_Event_Timer3|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Message Info Type>>|Event_Message_type_3_eurd_field|hidden
select|Event_Message_type0_3_eurd|$FORM_Event_Message_type0_3
option|A|@TR<<None>>
option|B|@TR<<Modem>>
option|C|@TR<<Carrier>>
option|D|@TR<<Wan>>

select|Event_Message_type1_3_eurd|$FORM_Event_Message_type1_3
option|A|@TR<<None>>
option|B|@TR<<Modem>>
option|C|@TR<<Carrier>>
option|D|@TR<<Wan>>

select|Event_Message_type2_3_eurd|$FORM_Event_Message_type2_3
option|A|@TR<<None>>
option|B|@TR<<Modem>>
option|C|@TR<<Carrier>>
option|D|@TR<<Wan>>

field|@TR<<&nbsp;&nbsp;Interface Selection>>|Event_Management3_field|hidden
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
check_vpn=""
for s2s in $s2s_cfgs; do
       	config_get check_vpn "eur_conf_vpn${vpn_i}" "Event_VPN${vpn_i}s_${s2s}"
	if [ "$check_vpn" = "B" ];then
		check_vpn="B"
       	else
	 	check_vpn="A"
        fi    
config_options=$config_options"
field|@TR<<&nbsp;&nbsp;VPN S-S:&nbsp;${s2s}>>|Event_VPN${vpn_i}s_${s2s}_field|hidden
radio|Event_VPN${vpn_i}s_${s2s}_eurd|$check_vpn|A|@TR<<Disable>>
radio|Event_VPN${vpn_i}s_${s2s}_eurd|$check_vpn|B|@TR<<Enable>>
"
done

check_vpn=""
for x2c in $x2c_cfgs; do
       	config_get check_vpn "eur_conf_vpn${vpn_i}" "Event_VPN${vpn_i}c_${x2c}"
	if [ "$check_vpn" = "B" ];then
		check_vpn="B"
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
end_form"

append forms "$config_options" "$N"


######################################################################################
# set JavaScript
javascript_forms="
    v01 = isset('Event_Remote_Reporting_Status_0_eurd', 'A');
    set_visible('Event_Remote_IP_address0_eurd_field', !v01);
    set_visible('Event_Remote_PORT0_eurd_field', !v01);
    set_visible('Event_Timer0_eurd_field', !v01);
    v011 = isset('Event_Remote_Reporting_Status_0_eurd', 'B');
    set_visible('Event_Message_type_0_eurd_field', v011);
    v012 = isset('Event_Remote_Reporting_Status_0_eurd', 'D');
    set_visible('Event_Management0_field', v012);
    set_visible('Event_Management0_field1', v012);
    set_visible('Event_Management0_field2', v012);
    set_visible('Event_Management0_field3', v012);
    set_visible('Event_Management0_field4', v012);
    set_visible('Event_Management0_field5', v012);



    v11 = isset('Event_Remote_Reporting_Status_1_eurd', 'A');
    set_visible('Event_Remote_IP_address1_eurd_field', !v11);
    set_visible('Event_Remote_PORT1_eurd_field', !v11);
    set_visible('Event_Timer1_eurd_field', !v11);
    v111 = isset('Event_Remote_Reporting_Status_1_eurd', 'B');
    set_visible('Event_Message_type_1_eurd_field', v111);
    v112 = isset('Event_Remote_Reporting_Status_1_eurd', 'D');
    set_visible('Event_Management1_field', v112);
    set_visible('Event_Management1_field1', v112);
    set_visible('Event_Management1_field2', v112);
    set_visible('Event_Management1_field3', v112);
    set_visible('Event_Management1_field4', v112);
    set_visible('Event_Management1_field5', v112);



    v21 = isset('Event_Remote_Reporting_Status_2_eurd', 'A');
    set_visible('Event_Remote_IP_address2_eurd_field', !v21);
    set_visible('Event_Remote_PORT2_eurd_field', !v21);
    set_visible('Event_Timer2_eurd_field', !v21);
    v211 = isset('Event_Remote_Reporting_Status_2_eurd', 'B');
    set_visible('Event_Message_type_2_eurd_field', v211);
    v212 = isset('Event_Remote_Reporting_Status_2_eurd', 'D');
    set_visible('Event_Management2_field', v212);
    set_visible('Event_Management2_field1', v212);
    set_visible('Event_Management2_field2', v212);
    set_visible('Event_Management2_field3', v212);
    set_visible('Event_Management2_field4', v212);
    set_visible('Event_Management2_field5', v212);


"
for vpn_i in 0 1 2 ; do
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
header "Tools" "Event Report" "@TR<<Event Report>>"  ' onload="modechange()" ' "$SCRIPT_NAME"
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


#####################################################################


footer ?>
<!--
##WEBIF:name:Tools:300:Event Report
-->
