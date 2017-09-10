#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

p_fresh() {
    [ -n "$FORM_interval" ] || FORM_interval=20
    
    ! empty "$FORM_refreshstop" && {
    	FORM_interval=0
    }
    
    [ "$FORM_interval" -gt 0 ] >/dev/null 2>&1 && {
    	meta_refresh="<meta http-equiv=\"refresh\" content=\"$FORM_interval;url=$SCRIPT_NAME?interval=$FORM_interval\" />"
    }
header_inject_head=$(cat <<EOF
$meta_refresh
EOF
)

}

b_fresh(){
    echo "<div class=\"settings\" style=\"float: right;\">"
    echo "<form name=\"refresh\" method=\"post\" action=\"$SCRIPT_NAME\">"
    
    [ "$FORM_interval" -gt 0 ] >/dev/null 2>&1 && {
cat <<EOF
<input type="submit" value=" @TR<<status_comports_Stop_Refreshing#Stop Refreshing>> " name="refreshstop" />
@TR<<status_comports_Interval#Interval>>: $FORM_interval (@TR<<status_comports_in_seconds#in seconds>>)
EOF
    } || {
cat <<EOF
<input type="submit" value=" @TR<<status_comports_Auto_Refresh#Auto Refresh>> " name="refreshstart" />
@TR<<status_comports_Interval#Interval>>:
    
<select name="interval">
EOF
    	for sec in $(seq 3 59); do
    		[ "$sec" -eq 20 ] && {
    			echo "<option value=\"$sec\" selected=\"selected\">$sec</option>"
    		} || {
    			echo "<option value=\"$sec\">$sec</option>"
    		}
    	done
cat <<EOF
</select>
@TR<<status_comports_in_seconds#in seconds>>
EOF
    }
    echo "</form></div><br/>"
}

p_fresh

header "GPS" "Location" "@TR<<#Location Map>>" ''

uci_load "gpsd"
config_get check_status "gpsd_conf" status
config_get check_location "gpsd_conf" mylocation
if [ "$check_status" = "1" ];then
    if [ "$check_location" = "1" ];then

        G_LAT=$(cat /tmp/run/GPS_position |grep 'latitude=' |awk -F '\"' '{print $2}' )
        G_LON=$(cat /tmp/run/GPS_position |grep 'longitude=' |awk -F '\"' '{print $2}' )
        G_ELEV=$(cat /tmp/run/GPS_position |grep 'elevation=' |awk -F '\"' '{print $2}' )
        G_TIME=$(cat /tmp/run/GPS_position |grep 'updatetime=' |awk -F '\"' '{print $2}' )
        G_RAD=5

        if empty "$G_LAT" ; then
            echo "<strong>Waiting for valid GPS data... Getting for carrier's online location:</strong><br>"
            /bin/gps_webget update
            G_LAT=$(cat /tmp/run/GPS_position_carr |grep 'latitude=' |awk -F '\"' '{print $2}' )
            G_LON=$(cat /tmp/run/GPS_position_carr |grep 'longitude=' |awk -F '\"' '{print $2}' )
            G_RAD=$(cat /tmp/run/GPS_position_carr |grep 'radius=' |awk -F '\"' '{print $2}' )
            G_Source=$(cat /tmp/run/GPS_position_carr |grep 'source=' |awk -F '\"' '{print $2}' )
            G_TIME=$(cat /tmp/run/GPS_position_carr |grep 'updatetime=' |awk -F '\"' '{print $2}' )
            if empty "$G_LAT" ; then
                echo "<strong>Can't get carrier's network Location information.</strong><br>"
            else
                if [ "$G_Source" = "GPS" ]; then
                    G_RAD="2000"
                fi

cat <<EOF
Last Carrier's Latitude:${G_LAT}, Longitude:${G_LON}, Radius:${G_RAD}m  <I> &nbsp;Update:$G_TIME<I>
<div id='map_canvas' style='width: 100%%; height: 640px;'></div>   
<script type='text/javascript' language='JavaScript' src='http://maps.google.com/maps/api/js?sensor=false&language=en'></script>
<script type='text/javascript' language='JavaScript' src='/js/location.js'></script>
<script language='JavaScript'>
var lat=$G_LAT;
var lon=$G_LON;
var rad=$G_RAD;
initialize(lat,lon,rad);
</script>  
EOF


            fi



        else
cat <<EOF
<tr><td>Latitude:${G_LAT}, Longitude:${G_LON}, Altitude:${G_ELEV}m  <I> &nbsp;Update:$G_TIME<I></td></tr>
<div id='map_canvas' style='width: 100%%; height: 640px;'></div>   
<script type='text/javascript' language='JavaScript' src='http://maps.google.com/maps/api/js?sensor=false&language=en'></script>
<script type='text/javascript' language='JavaScript' src='/js/location.js'></script>
<script language='JavaScript'>
var lat=$G_LAT;
var lon=$G_LON;
var rad=$G_RAD;
initialize(lat,lon,rad);
</script>  
EOF
       fi

    else
        echo "<tr><td><strong>Sorry, Mylocation Update is disable. No data.</strong></td></tr>"
    fi    
else
        echo "<tr><td><strong>Sorry, GPS Service is disable. No data.</strong></td></tr>"
fi

b_fresh
footer_nosubmit ?>
<!--
##WEBIF:name:GPS:100:Location
-->
