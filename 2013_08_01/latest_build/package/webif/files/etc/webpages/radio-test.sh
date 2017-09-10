#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
. /etc/version
###################################################################
# Diagnostics
#
# Description:
#	Provide some diagnostic tools.
#
# Author(s) [in order of work date]:
#	Jeremy Collake <jeremy.collake@gmail.com>
#
# Major revisions:
#
# NVRAM variables referenced:
#  none
#
# Configuration files referenced:
#   none
#
iperftmpfile="/var/run/iperf_run"
client="iperfclient"

stop_iperf()
{
    iperf_id=`ps|grep "iperf -c" | grep -v "grep"| awk '{print $1}'`
    empty "$iperf_id" || kill -s 2 $iperf_id 
}

! empty "$FORM_iperf_stop_button" && {		
    stop_iperf
    rm -f /var/run/iperf_unit &
    echo "The Tool for Iperf is Stopped ">>"$iperftmpfile"
    echo "page_refresh=30" > /var/run/iperf_refresh
}

! empty "$FORM_iperf_clear_button" && {		
    stop_iperf
    echo "The Output Result Clear " > "$iperftmpfile"
    echo "page_refresh=30" > /var/run/iperf_refresh
}

if ! empty "$FORM_iperf_button"; then
    stop_iperf
    echo "page_refresh=30" > /var/run/iperf_refresh
    radiomode=$(uci get wireless.@wifi-iface[0].mode)
    if [ "$radiomode" = "mesh" ];then
         stop_iperf
         echo "Please wait for output of \"iperf -c 224.0.0.1 -u -b 10M -T 32 -t 864000 -i 1\"...<br/>" > "$iperftmpfile"
         route add default dev br-lan 2>/dev/null  
         iperf -c 224.0.0.1 -u -b 10M -T 32 -t 864000 -i 1 | awk '{printf("%s <br/>",$0);}' >> "$iperftmpfile" &
        
         echo "unit=$FORM_format" >/var/run/iperf_unit
         echo "page_refresh=1" > /var/run/iperf_refresh
    else
        stop_iperf
        echo "The Output Result Clear " > "$iperftmpfile"
        echo "page_refresh=30" > /var/run/iperf_refresh
        append ERROR "Error: Radio mode need to be mesh mode <br />" "$N"        
    fi
fi


header "Wireless" "Radio Test" "@TR<<Radio Test tool>>" 'onload="modechange()"' "$SCRIPT_NAME"

cat <<EOF
<script type="text/javascript" src="/webif.js"></script>
<script type="text/javascript">
<!--
var tick=1;
function scroldown()
{
    //alert (document.getElementById('myframe').name);
    document.getElementById('myframe').contentWindow.scrollTo(100,tick*1000);
    tick=tick+1;
}

function stopscrolldown()
{
    clearInterval(rid);
}
function modechange()
{
	var v;
	$js

	hide('save');
	show('save');
        var rid=setInterval("scroldown()",1000);
}
-->
</script>

EOF

display_form <<EOF
onchange|modechange
EOF

cat <<EOF
<div class="settings">
<h3><strong>@TR<<Test Command>></strong></h3>
<input style="margin-left: 3%;" type="submit" name="iperf_button" value="Continuous transmit" />
<input style="margin-left: 6%;" type="submit" onclick="clearInterval(rid)" name="iperf_stop_button" value=" Stop transmit"/>
<input style="margin-left: 9%;" type="submit" name="iperf_clear_button" value=" Clear the result! "/>
</div>
<br />
EOF

cat <<EOF
<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
<td width="95%">
<div>
<iframe id="myframe" name="FRAME" src="/cgi-bin/webif/tools-iperf_data.sh" height="300" width="95%"></iframe>
</div>
</td>
</tr>
</table>
EOF

footer_nosubmit ?>
<!--
##WEBIF:name:Wireless:600:Radio Test
-->

