#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"


uci_load gpsd
conf="gpsd_conf"
config_get FORM_status $conf status
if [ "$FORM_status" = "1" ]; then

uci_load "gpsrecorderd"
conf="gpsrecorder_conf"

if empty "$FORM_submit"; then

    conf="gpsrecorder_conf"

else

 	FORM_Pos_Record_EN="$FORM_Pos_Record_EN_rec"
# 	FORM_Pos_Record_NUMS="$FORM_Pos_Record_NUMS_rec"
 	FORM_Pos_Record_INTERVAL="$FORM_Pos_Record_INTERVAL_rec"
 	FORM_Pos_Record_IO="$FORM_Pos_Record_IO_rec"
 	FORM_Pos_Record_ALT="$FORM_Pos_Record_ALT_rec"
 	FORM_Pos_Record_SPEED="$FORM_Pos_Record_SPEED_rec"
 	FORM_Pos_Record_OVERSPEED="$FORM_Pos_Record_OVERSPEED_rec"
 	FORM_Pos_Record_ORIENT="$FORM_Pos_Record_ORIENT_rec"
 	FORM_Pos_Record_ORIENTCHG="$FORM_Pos_Record_ORIENTCHG_rec"
 	FORM_Pos_Record_RSSI="$FORM_Pos_Record_RSSI_rec"

validate <<EOF
int|$FORM_Pos_Record_INTERVAL_rec|@TR<<Record Interval>>||$FORM_Pos_Record_INTERVAL
int|$FORM_Pos_Record_OVERSPEED_rec|@TR<<Record Interval>>||$FORM_Pos_Record_INTERVAL
int|$FORM_Pos_Record_ORIENTCHG_rec|@TR<<Record Interval>>||$FORM_Pos_Record_INTERVAL
EOF

       [ "$?" = "0" ] && {

            uci_set "gpsrecorderd" "$conf" "Pos_Record_EN" "$FORM_Pos_Record_EN"
#            uci_set "gpsrecorderd" "$conf" "Pos_Record_NUMS" "$FORM_Pos_Record_NUMS"
            uci_set "gpsrecorderd" "$conf" "Pos_Record_INTERVAL" "$FORM_Pos_Record_INTERVAL"
            uci_set "gpsrecorderd" "$conf" "Pos_Record_ALT" "$FORM_Pos_Record_ALT"
            uci_set "gpsrecorderd" "$conf" "Pos_Record_IO" "$FORM_Pos_Record_IO"
            uci_set "gpsrecorderd" "$conf" "Pos_Record_SPEED" "$FORM_Pos_Record_SPEED"
            uci_set "gpsrecorderd" "$conf" "Pos_Record_OVERSPEED" "$FORM_Pos_Record_OVERSPEED"
            uci_set "gpsrecorderd" "$conf" "Pos_Record_ORIENT" "$FORM_Pos_Record_ORIENT"
            uci_set "gpsrecorderd" "$conf" "Pos_Record_ORIENTCHG" "$FORM_Pos_Record_ORIENTCHG"
            uci_set "gpsrecorderd" "$conf" "Pos_Record_RSSI" "$FORM_Pos_Record_RSSI"

         }
fi


uci_load "gpsrecorderd"
       config_get FORM_Pos_Record_EN $conf Pos_Record_EN
#       config_get FORM_Pos_Record_NUMS $conf Pos_Record_NUMS
       config_get FORM_Pos_Record_INTERVAL $conf Pos_Record_INTERVAL
       config_get FORM_Pos_Record_ALT $conf Pos_Record_ALT
       config_get FORM_Pos_Record_IO $conf Pos_Record_IO
       config_get FORM_Pos_Record_SPEED $conf Pos_Record_SPEED
       config_get FORM_Pos_Record_OVERSPEED $conf Pos_Record_OVERSPEED
       config_get FORM_Pos_Record_ORIENT $conf Pos_Record_ORIENT
       config_get FORM_Pos_Record_ORIENTCHG $conf Pos_Record_ORIENTCHG
       config_get FORM_Pos_Record_RSSI $conf Pos_Record_RSSI

       if  empty "$FORM_Pos_Record_INTERVAL"; then
           FORM_Pos_Record_INTERVAL="600"
       fi

       if  empty "$FORM_Pos_Record_OVERSPEED"; then
           FORM_Pos_Record_OVERSPEED="120"
       fi

       if  empty "$FORM_Pos_Record_ORIENTCHG"; then
           FORM_Pos_Record_ORIENTCHG="60"
       fi

#field|@TR<<&nbsp;&nbsp;Position Items>>|Pos_Record_NUMS_rec_field|hidden
#select|Pos_Record_NUMS_rec|$FORM_Pos_Record_NUMS
#option|20000|@TR<<Max 20000 Items>>

config_options="
start_form| @TR<<GPS Recorder Setting>>
field|@TR<< <strong>Status<strong> >>|Pos_Record_EN_rec_field|1
select|Pos_Record_EN_rec|$FORM_Pos_Record_EN
option|A|@TR<<Disable>>
option|B|@TR<<Enable GPS Recorder>>

field|@TR<< &nbsp;&nbsp;Record Feature Selections:>>|Pos_Record_Select_rec_field|hidden
string| (Record items among 16,000~36,000.) 

field|@TR<<&nbsp;&nbsp;Time Interval>>|Pos_Record_INTERVAL_rec_field|hidden
text|Pos_Record_INTERVAL_rec|$FORM_Pos_Record_INTERVAL|@TR<<[30~65535](s)>>

field|@TR<<&nbsp;&nbsp;DI/DO Changed>>|Pos_Record_IO_rec_field|hidden
select|Pos_Record_IO_rec|$FORM_Pos_Record_IO
option|A|@TR<<Don't Record>>
option|B|@TR<<Record>>

field|@TR<<&nbsp;&nbsp;Speed>>|Pos_Record_SPEED_rec_field|hidden
select|Pos_Record_SPEED_rec|$FORM_Pos_Record_SPEED
option|A|@TR<<Don't Record>>
option|B|@TR<<Record>>

field|@TR<<&nbsp;&nbsp;Over Speed>>|Pos_Record_OVERSPEED_rec_field|hidden
text|Pos_Record_OVERSPEED_rec|$FORM_Pos_Record_OVERSPEED|@TR<<[Min 30](Km/h)>>

field|@TR<<&nbsp;&nbsp;Orientation>>|Pos_Record_ORIENT_rec_field|hidden
select|Pos_Record_ORIENT_rec|$FORM_Pos_Record_ORIENT
option|A|@TR<<Don't Record>>
option|B|@TR<<Record>>

field|@TR<<&nbsp;&nbsp;Orientation Changed>>|Pos_Record_ORIENTCHG_rec_field|hidden
text|Pos_Record_ORIENTCHG_rec|$FORM_Pos_Record_ORIENTCHG|@TR<<[5~180](180:Disable)>>

field|@TR<<&nbsp;&nbsp;Carrier RSSI Level>>|Pos_Record_RSSI_rec_field|hidden
select|Pos_Record_RSSI_rec|$FORM_Pos_Record_RSSI
option|A|@TR<<Don't Record>>
option|B|@TR<<Record>>

field|@TR<<&nbsp;&nbsp;Altitude>>|Pos_Record_ALT_rec_field|hidden
select|Pos_Record_ALT_rec|$FORM_Pos_Record_ALT
option|A|@TR<<Don't Record>>
option|B|@TR<<Record>>

end_form
"

append forms "$config_options" "$N"


######################################################################################
# set JavaScript
# set_visible('Pos_Record_NUMS_rec_field', 0);

javascript_forms="
    v_en = isset('Pos_Record_EN_rec','B');

    set_visible('Pos_Record_Select_rec_field', v_en);
    set_visible('Pos_Record_INTERVAL_rec_field', v_en);
    set_visible('Pos_Record_ALT_rec_field', v_en);
    set_visible('Pos_Record_IO_rec_field', v_en);

    set_visible('Pos_Record_SPEED_rec_field', v_en);
    v_speed=isset('Pos_Record_SPEED_rec','B');
    if(v_speed&&v_en)v_speed=1;
    else v_speed=0;
    set_visible('Pos_Record_OVERSPEED_rec_field', v_speed);

    set_visible('Pos_Record_ORIENT_rec_field', v_en);
    v_orient=isset('Pos_Record_ORIENT_rec','B');
    if(v_orient&&v_en)v_orient=1;
    else v_orient=0;
    set_visible('Pos_Record_ORIENTCHG_rec_field', v_orient);

    set_visible('Pos_Record_RSSI_rec_field', v_en);
"

append js "$javascript_forms" "$N"


#####################################################################
header "GPS" "Recorder" "@TR<<GPS Recorder Service>>" ' onload="modechange()" ' "$SCRIPT_NAME"
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

<div class="settings">
<h3><strong> Current GPS Infomation</strong></h3>
<div class="settings-content">
<table width="100%" summary="Settings">
<tr id="fielden">
<td width="40%">Local Time:</td><td width="60%">
$(date)
</td></tr>
EOF

/bin/gpsrecorder readgps

cat /var/run/gpsstatus <<EOF
EOF
cat <<EOF
</table>
</div>
<div class="clearfix"></div></div>
EOF


display_form <<EOF
onchange|modechange
$validate_error
$forms
EOF

#####################################################################
else  
#####################################################################
header "GPS" "Recorder" "@TR<<GPS Recorder Service>>"  "$SCRIPT_NAME"
#####################################################################
cat <<EOF
Sorry, GPS service is disable.
<br>Please config GPS Settings firstly.
EOF

fi


footer ?>
<!--
##WEBIF:name:GPS:500:Recorder
-->
