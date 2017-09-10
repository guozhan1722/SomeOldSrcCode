#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"


uci_load gpsd
conf="gpsd_conf"

if empty "$FORM_submit"; then

    conf="gpsd_conf"

else

        FORM_status="$FORM_status_gpsd"
        FORM_GPSD_TCP_Port="$FORM_GPSD_TCP_Port_gpsd"
#        FORM_Antenna_PowerV="$FORM_Antenna_PowerV_gpsd"
        FORM_mylocation="1"
        FORM_GPS_Source="$FORM_GPS_Source_gpsd"
#        FORM_acceler_set="$FORM_acceler_set_gpsd"

validate <<EOF
int|FORM_status_gpsd|@TR<<GPS Service Status>>||$FORM_status
int|FORM_GPSD_TCP_Port_gpsd|@TR<<GPS TCP Port>>||$FORM_GPSD_TCP_Port
int|$FORM_mylocation_gpsd|@TR<<Mylocation Update>>||$FORM_mylocation
EOF

       [ "$?" = "0" ] && {
            uci_set "gpsd" "$conf" "status" "$FORM_status"
            uci_set "gpsd" "$conf" "GPSD_TCP_Port" "$FORM_GPSD_TCP_Port"
            uci_set "gpsd" "$conf" "mylocation" "$FORM_mylocation"
	    uci_set "gpsd" "$conf" "GPS_Source" "$FORM_GPS_Source"
#	    uci_set "gpsd" "$conf" "acceler_set" "$FORM_acceler_set"
         }
fi



uci_load "gpsd"
       config_get FORM_status $conf status
       config_get FORM_GPSD_TCP_Port $conf GPSD_TCP_Port
       config_get FORM_mylocation $conf mylocation 
       config_get FORM_GPS_Source $conf GPS_Source 
       config_get FORM_GPS_Carrier $conf Carrier_GPS_Data 
       config_get FORM_GPS_STD $conf Standalone_GPS_Source 
#       config_get FORM_acceler_set $conf acceler_set
    if [ "$FORM_GPS_Carrier" ]; then
        conf="gpsd_conf"
    else
        FORM_GPS_Carrier="/dev/ttyUSB4"
        uci_set "gpsd" "$conf" "Carrier_GPS_Data" "$FORM_GPS_Carrier"
    fi


config_options="start_form| @TR<<Settings Option:>>
field|@TR<< <strong>GPS Status<strong> >>|status_gpsd_field|1
select|status_gpsd|$FORM_status
option|0|@TR<<Disable>>
option|1|@TR<<Enable>>

field|@TR<<&nbsp;&nbsp;GPS Source>>|Source_gpsd_field|hidden
select|GPS_Source_gpsd|$FORM_GPS_Source
option|$FORM_GPS_STD|@TR<< Standalone GPS >>
option|$FORM_GPS_Carrier|@TR<< Embeded Carrier GPS >>

field|@TR<<&nbsp;&nbsp;TCP Port>>|GPSD_TCP_Port_gpsd_field|hidden
text|GPSD_TCP_Port_gpsd|$FORM_GPSD_TCP_Port|@TR<<0~65535,default:2947>>

end_form"
#field|@TR<<&nbsp;&nbsp;Acceler Sensitivity>>|ACCELER_SET_field|hidden
#radio|acceler_set_gpsd|$FORM_acceler_set|0|@TR<<Off &nbsp;&nbsp;>>
#radio|acceler_set_gpsd|$FORM_acceler_set|1|@TR<<Low &nbsp;&nbsp;>>
#radio|acceler_set_gpsd|$FORM_acceler_set|2|@TR<<Medium &nbsp;&nbsp;>>
#radio|acceler_set_gpsd|$FORM_acceler_set|3|@TR<<High &nbsp;&nbsp;>>

#field|@TR<<&nbsp;&nbsp;Location For Other Application>>|mylocation_gpsd_field|hidden
#radio|mylocation_gpsd|$FORM_mylocation|0|@TR<<Disable>>
#radio|mylocation_gpsd|$FORM_mylocation|1|@TR<<Enable>>

append forms "$config_options" "$N"


######################################################################################
# set JavaScript
javascript_forms="
    v01 = isset('status_gpsd', '1')
    set_visible('Source_gpsd_field', v01);
    set_visible('GPSD_TCP_Port_gpsd_field', v01);
    set_visible('mylocation_gpsd_field', v01);
"
#    set_visible('ACCELER_SET_field', v01);

append js "$javascript_forms" "$N"


#####################################################################
header "GPS" "Settings" "@TR<<GPS Service Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"
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
##WEBIF:name:GPS:200:Settings
-->
