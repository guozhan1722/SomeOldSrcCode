#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"

if [ "$FORM_view" ]; then

#####################################################################
header "GPS" "Load Record" "@TR<<GPS Record Review>>" "$SCRIPT_NAME"
#####################################################################

cat <<EOF
<div class="settings">
<table width="95%" summary="Settings">
<tr id="fielden">
<td width="20%">Record Time(UTC)</td><td width="12%">Latitude</td> <td width="12%">Longitude</td><td width="8%">Input</td><td width="8%">Output</td><td width="6%">Speed</td><td width="6%">Angle</td><td width="6%">RSSI</td><td>Altitude</td></tr>
EOF

/bin/gpsloadrecord view $FORM_view

cat /var/run/gpsloadrecview <<EOF
EOF
cat <<EOF
</table>
</div>
<div class="clearfix"></div>
EOF


elif [ "$FORM_trace" ]; then

#####################################################################
header "GPS" "Load Record" "@TR<<GPS Trace Map>>" "$SCRIPT_NAME"
#####################################################################
/bin/gpsloadrecord trace $FORM_trace

cat <<EOF
<div id='map_canvas' style='width: 100%%; height: 640px;'></div>   
<script type='text/javascript' language='JavaScript' src='http://maps.google.com/maps/api/js?sensor=false&language=en'></script>
<script language='JavaScript'>
      function initialize() {
EOF
cat /var/run/gpsloadrec0 <<EOF
EOF
#        var myLatLng = new google.maps.LatLng(51.142838,-114.075241);
#        var myLatLng_stop = new google.maps.LatLng(51.142838,-114.075241);
#        var mapOptions = { zoom: 11,
cat <<EOF
          center: myLatLng_center,
          mapTypeId: google.maps.MapTypeId.ROADMAP
        };

        var map = new google.maps.Map(document.getElementById('map_canvas'), mapOptions);
        var image_start = '/images/go_flag.gif';
        var image_stop = '/images/end_flag.gif';
        var beachMarker = new google.maps.Marker({
        position: myLatLng,
        map: map,
        icon: image_start
        });

        var beachMarker1 = new google.maps.Marker({
        position: myLatLng_stop,
        map: map,
        icon: image_stop
        });

        var flightPlanCoordinates = [
EOF
#            new google.maps.LatLng(21.291982, -157.821856),
cat /var/run/gpsloadrectrace <<EOF
EOF

cat <<EOF

        ];
        var flightPath = new google.maps.Polyline({
          path: flightPlanCoordinates,
          strokeColor: '#0000FF',
          strokeOpacity: 1.0,
          strokeWeight: 2
        });

        flightPath.setMap(map);
      }
    </script>

<script language='JavaScript'>
initialize();
</script>  
EOF




else #To End

#$FORM_dataselect1->? on


uci_load "gpsrecorderd"
conf="gpsrecorder_conf"
send_conf="A"

if empty "$FORM_submit"; then

    conf="gpsrecorder_conf"
    if [ "$FORM_submit_download" ]; then
        conf="gpsrecorder_conf"

    fi

    FORM_SERVER_Address="nms.microhardcorp.com"
    FORM_SERVER_Port="30175"
    FORM_Mode="E"

else

    FORM_SERVER_Address="$FORM_SERVER_Address_send"
    FORM_SERVER_Port="$FORM_SERVER_Port_send"
    FORM_Mode="$FORM_Mode_send"
    send_data="data-"
    index_file="0" 
    while [ "$index_file" -lt "51" ] ; do
        eval select_chk="\$FORM_dataselect${index_file}"
        if [ "$select_chk" = "on" ]; then
           send_data=$send_data"${index_file}-"
        fi
        index_file=`expr $index_file + 1`
    done


validate <<EOF
int|$FORM_SERVER_Port_send|@TR<<Send Port>>||$FORM_SERVER_Port
EOF

       [ "$?" = "0" ] && {
            send_conf="B"
            #uci_set "gpsrecorderd" "$conf" "Pos_Record_EN" "$FORM_Pos_Record_EN"
         }

       if [ "$send_data" = "data," ]; then
           send_conf="A"
       fi 
fi


config_options="
start_form| @TR<<Send Record To Server>>
field|@TR<< &nbsp;&nbsp;Record Time Range>>
string| Please Select Above Items 

field|@TR<< &nbsp;&nbsp;Send Mode/Protocol>>
select|Mode_send|$FORM_Mode
option|A|@TR<<NMEA via UDP>>
option|B|@TR<<NMEA via TCP>>
option|C|@TR<<GpsGate via UDP>>
option|D|@TR<<GpsGate via TCP>>
option|E|@TR<<Plain Text via UDP>>
option|F|@TR<<Plain Text via TCP>>

field|@TR<<&nbsp;&nbsp;Server Address/IP>>|SERVER_Address_send_field
text|SERVER_Address_send|$FORM_SERVER_Address

field|@TR<<&nbsp;&nbsp;Server Port>>|SERVER_Port_send_field
text|SERVER_Port_send|$FORM_SERVER_Port

end_form
"

append forms "$config_options" "$N"


######################################################################################
# set JavaScript
javascript_forms="
    v_en = isset('Mode_send','B');

"

append js "$javascript_forms" "$N"


#####################################################################
header "GPS" "Load Record" "@TR<<GPS Record Review and Load Service>>" ' onload="modechange()" ' "$SCRIPT_NAME"
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


function unselectall(form){
if(form.chkAll.checked){
form.chkAll.checked = form.chkAll.checked&0;
}
}
function CheckAll(form){
for (var i=0;i<form.elements.length;i++){
var e = form.elements[i];
if (e.name != 'chkAll'&&e.disabled==false&&e.type == 'checkbox')
e.checked = form.chkAll.checked;
}
}

-->
</script>

<div class="settings">
<h3><strong> Current Position Record</strong></h3>
<div class="settings-content">
<table width="100%" summary="Settings">
<tr id="fielden">
<td width="30%">&nbsp;&nbsp;Start Time(UTC)</td><td width="30%">&nbsp;&nbsp;End Time(UTC)</td><td width="6%">Select</td><td>&nbsp;&nbsp;&nbsp;&nbsp;Review/Operation</td></tr>
EOF

/bin/gpsloadrecord list
#box,from, to, view, trace, export  

cat /var/run/gpsloadreclist <<EOF
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

if [ "$FORM_submit" ]; then
if [ "$send_conf" = "B" ]; then
cat <<EOF
&nbsp;&nbsp;&nbsp;&nbsp;Begin to send,waiting...<br>&nbsp;&nbsp;&nbsp;&nbsp;
EOF
/bin/gpsloadrecord send $FORM_Mode $FORM_SERVER_Port $FORM_SERVER_Address $send_data
else
cat <<EOF
&nbsp;&nbsp;&nbsp;&nbsp;Please check above settings.
EOF
fi
fi

#other views
fi

footer ?>
<!--
##WEBIF:name:GPS:600:Load Record
-->
