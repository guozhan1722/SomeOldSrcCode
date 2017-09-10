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
uci_load iperf

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

if empty "$FORM_iperf_button"; then
    stop_iperf
    echo "page_refresh=30" > /var/run/iperf_refresh
    config_get FORM_serveraddr $client serveraddr
    config_get FORM_transmit $client transmit
    config_get FORM_time $client time
    config_get FORM_num $client num
    config_get FORM_len $client len
    config_get FORM_interval $client interval
    config_get FORM_format $client format
    config_get FORM_testmode $client testmode
    config_get FORM_port $client port
    config_get FORM_proto $client proto
    config_get FORM_mss $client mss
    config_get FORM_window $client window
    config_get FORM_bandwidth $client bandwidth
    config_get FORM_nodelay $client nodelay
else
    eval FORM_serveraddr="\$FORM_serveraddr_$client"
    eval FORM_transmit="\$FORM_transmit_$client"
    eval FORM_time="\$FORM_time_$client"
    eval FORM_num="\$FORM_num_$client"
    eval FORM_len="\$FORM_len_$client"
    eval FORM_interval="\$FORM_interval_$client"
    eval FORM_format="\$FORM_format_$client"
    eval FORM_testmode="\$FORM_testmode_$client"
    eval FORM_port="\$FORM_port_$client"
    eval FORM_proto="\$FORM_proto_$client"
    eval FORM_mss="\$FORM_mss_$client"
    eval FORM_window="\$FORM_window_$client"
    eval FORM_bandwidth="\$FORM_bandwidth_$client"
    eval FORM_nodelay="\$FORM_nodelay_$client"
    validate <<EOF
ip|FORM_serveraddr_$client|$client @TR<<Server IP Address>>||$FORM_serveraddr
int|FORM_transmit_$client|$client @TR<<Transmit>>||$FORM_transmit
int|FORM_len_$client|$client @TR<<Length>>||$FORM_len
int|FORM_interval_$client|$client @TR<<Test Interval>>||$FORM_interval
int|FORM_port_$client|$client @TR<<Port>>||$FORM_port
int|FORM_window_$client|$client @TR<<Window>>||$FORM_window
int|FORM_bandwidth_$client|$client @TR<<Bandwidth>>||$FORM_bandwidth
EOF
    [ "$?" = "0" ] && {

        sanitized=$(echo "$FORM_serveraddr" | awk -f "/usr/lib/webif/sanitize.awk")	
        ! empty "$sanitized" && ! empty "$FORM_transmit" &&  {
            diag_command="iperf -c $sanitized"
    
            [ "$FORM_time" = "1" ] && {
                diag_command="$diag_command -t $FORM_transmit" 
            } || {
                diag_command="$diag_command -n $FORM_transmit" 
            }
    
            empty "$FORM_interval" || diag_command="$diag_command -i $FORM_interval" 
    
            [ "$FORM_testmode" = "none" ] || {
                [ "$FORM_testmode" = "dual" ] && {
                    diag_command="$diag_command -d"
                }||{
                    diag_command="$diag_command -r"
                }
            }
    
            empty "$FORM_format" || diag_command="$diag_command -f $FORM_format" 
            [ "$FORM_proto" = "tcp" ] && {
                empty "$FORM_len" || diag_command="$diag_command -l $FORM_len"
                empty "$FORM_window" || diag_command="$diag_command -w $FORM_window"
                empty "$FORM_mss" || diag_command="$diag_command -M $FORM_mss"
                empty "$FORM_nodelay" || diag_command="$diag_command -N"
            }|| {
                diag_command="$diag_command -u" 
                empty "$FORM_bandwidth" || diag_command="$diag_command -b ${FORM_bandwidth}m"   
            }
        }

        stop_iperf
        echo "Please wait for output of \"$diag_command \"...<br/>" > "$iperftmpfile"

        $diag_command 2>&1 | awk '{printf("%s <br/>",$0);}' >> "$iperftmpfile" &
        #max_result=0
        #current_result=$(cat /tmp/run/iperf_run |grep -v Please|awk -F "<br/>" '{print $(NF-1)}'|awk '{print $7}') 
        #current_unit=$(cat /tmp/run/iperf_run |grep -v Please|awk -F "<br/>" '{print $(NF-1)}'|awk '{print $8}') 
        #echo "Current result = $current_result $current_unit  <br/>" > /var/run/iperf_run_result
        #echo "Max result = $max_result $current_unit  <br/>" >> /var/run/iperf_run_result

        uci_set "iperf" "$client" "serveraddr" "$FORM_serveraddr"
        uci_set "iperf" "$client" "transmit" "$FORM_transmit"
        uci_set "iperf" "$client" "time" "$FORM_time"
        uci_set "iperf" "$client" "num" "$FORM_num"
        uci_set "iperf" "$client" "len" "$FORM_len"
        uci_set "iperf" "$client" "interval" "$FORM_interval"
        uci_set "iperf" "$client" "format" "$FORM_format"
        uci_set "iperf" "$client" "testmode" "$FORM_testmode"
        uci_set "iperf" "$client" "port" "$FORM_port"
        uci_set "iperf" "$client" "proto" "$FORM_proto"
        uci_set "iperf" "$client" "mss" "$FORM_mss"
        uci_set "iperf" "$client" "window" "$FORM_window"
        uci_set "iperf" "$client" "bandwidth" "$FORM_bandwidth"
        uci_set "iperf" "$client" "nodelay" "$FORM_nodelay"
        echo "unit=$FORM_format" >/var/run/iperf_unit
        uci_commit "iperf"
        echo "page_refresh=1" > /var/run/iperf_refresh

    }
fi

app="start_form|@TR<<Iperf-Application Layer>>
        field|@TR<<Server Address>>
        text|serveraddr_$client|$FORM_serveraddr

        field|@TR<<Transmit>>
        text|transmit_$client|$FORM_transmit
        radio|time_$client|$FORM_time|1|@TR<<Seconds>>
        radio|time_$client|$FORM_time|0|@TR<<Bytes>>

        field|@TR<<Report Interval (s)>>
        text|interval_$client|$FORM_interval

        field|@TR<<Configuration>>
        select|conf_$client|$FORM_conf
        option|default|@TR<<Default>>
        option|advanced|@TR<<Advanced>>

        end_form"

append forms "$app" "$N"

tran="start_form|@TR<<Iperf-Transport Layer>>
        field|@TR<<Output Format>>|outputformat_field|hidden
        select|format_$client|$FORM_format
        option|k|@TR<<KBits>>
        option|m|@TR<<MBits>>
        option|K|@TR<<KBytes>>
        option|M|@TR<<MBytes>>

        field|@TR<<Testing Mode>>|testmode_field|hidden
        radio|testmode_$client|$FORM_testmode|none|@TR<<None>>
        radio|testmode_$client|$FORM_testmode|dual|@TR<<Dual>>
        radio|testmode_$client|$FORM_testmode|trade|@TR<<Trade>>


        field|@TR<<Port>>|port_field|hidden
        text|port_$client|$FORM_port

        field|@TR<<Protocal>>|protocal_field|hidden
        select|proto_$client|$FORM_proto
        option|tcp|@TR<<TCP>>
        option|udp|@TR<<UDP>>


        field|@TR<<Buffer Length (KM)>>|bufferlength_field|hidden
        text|len_$client|$FORM_len

        field|@TR<<Window Size (KM)>>|windowsize_field|hidden
        text|window_$client|$FORM_window

        field|@TR<<Max Segment Size (bytes)>>|mss_field|hidden
        text|mss_$client|$FORM_mss

        field|@TR<<TCP No Delay>>|nodelay_field|hidden
        checkbox|nodelay_$client|$FORM_nodelay|1

        field|@TR<<UDP Bandwidth (Mbits/sec)>>|bandwidth_field|hidden
        text|bandwidth_$client|$FORM_bandwidth
        end_form"

append forms "$tran" "$N"

#testbutton="start_form|@TR<<Test command>>
#        submit|iperf_button|@TR<<Start Test >>
#        submit|iperf_stop_button|@TR<<Stop Test >>
#        end_form"
#append forms "$testbutton" "$N"

###################################################################
# set JavaScript
java_forms="
    if (document.getElementById('conf_$client').value == 'default' )
    {
        document.getElementById('outputformat_field').style.display = 'none';
        document.getElementById('testmode_field').style.display = 'none';
        document.getElementById('port_field').style.display = 'none';
        document.getElementById('protocal_field').style.display = 'none';
        document.getElementById('bufferlength_field').style.display = 'none';
        document.getElementById('windowsize_field').style.display = 'none';
        document.getElementById('mss_field').style.display = 'none';
        document.getElementById('nodelay_field').style.display = 'none';
        document.getElementById('bandwidth_field').style.display = 'none';

    }

    else 
    {
        document.getElementById('outputformat_field').style.display = '';
        document.getElementById('testmode_field').style.display = '';
        document.getElementById('port_field').style.display = '';
        document.getElementById('protocal_field').style.display = '';

        if (document.getElementById('proto_$client').value == 'udp' )
        {
            document.getElementById('bandwidth_field').style.display = '';
            document.getElementById('bufferlength_field').style.display = 'none';
            document.getElementById('windowsize_field').style.display = 'none';
            document.getElementById('mss_field').style.display = 'none';
            document.getElementById('nodelay_field').style.display = 'none';
        } else {
            document.getElementById('bandwidth_field').style.display = 'none';
            document.getElementById('bufferlength_field').style.display = '';
            document.getElementById('windowsize_field').style.display = '';
            document.getElementById('mss_field').style.display = '';
            document.getElementById('nodelay_field').style.display = '';
        }
    }
"
append js "$java_forms" "$N"

header "Tools" "Iperf" "@TR<<Iperf - Network performance measurement tool>>" 'onload="modechange()"' "$SCRIPT_NAME"

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
$forms
EOF

cat <<EOF
<div class="settings">
<h3><strong>@TR<<Test Command>></strong></h3>
<input style="margin-left: 3%;" type="submit" name="iperf_button" value=" Run IPerf !" />
<input style="margin-left: 6%;" type="submit" name="iperf_stop_button" value=" Stop IPerf ! "/>
<input style="margin-left: 9%;" type="submit" name="iperf_clear_button" value=" Clear the result! "/>
</div>
<br />
EOF

cat <<EOF
<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
<td width="45%">
<div>
<iframe src="/cgi-bin/webif/tools-iperf_graph.sh" height="300" width="95%"></iframe>
</div>
</td>
<br/>
<td width="45%">
<div>
<iframe src="/cgi-bin/webif/tools-iperf_data.sh" height="300" width="95%"></iframe>
</div>
</td>
</tr>
</table>
EOF

footer ?>
<!--
###WEBIF:name:Tools:995:Iperf
-->

