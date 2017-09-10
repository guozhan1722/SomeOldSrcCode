#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
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
header_inject_head=$(cat <<EOF
<style type="text/css">
<!--

#logarea {
	width:1000px;

}
#logarea pre {
	//margin: 0.2em auto 1em auto;
        //position: relative;
	padding: 3px;
	width: 70%;
	//margin: auto;
	height: 250px;
	overflow: scroll;
	border: 1px solid;
	//right: 10px;
        left: 1em;
        //margin-right: 10em;
        margin-left: 10em;
}

#my_id {
	position: relative;
	width: 85%;
        left: 5em;
	//text-align: right;
	//z-index: 100;
}

// -->
</style>

EOF
)

uci_load iperf
client="iperfclient"

! empty "$FORM_iperf_stop_button" && {		
    iperf_pid=`ps |grep "iperf -c" |awk '{print $1}'`
    empty "$iperf_pid" || kill $iperf_pid 
    rm -f /var/run/iperf_run &
    rm -f /var/run/iperf_unit &
}

if empty "$FORM_iperf_button"; then
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

    iperf_pid=`ps |grep "iperf -c" |awk '{print $1}'`
    empty "$iperf_pid" || kill $iperf_pid 
    rm -f /var/run/iperf_run &
    rm -f /var/run/iperf_unit &

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

header "Status" "Iperf" "@TR<<Iperf - Network performance measurement tool>>" 'onload="modechange()"' "$SCRIPT_NAME"

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

if empty "$FORM_iperf_button"; then
cat <<EOF
<div class="settings">
<h3><strong>@TR<<Test Command>></strong></h3>
<input style="margin-left: 3em;" type="submit" name="iperf_button" value=" Run IPerf !" />
<input style="margin-left: 20em;" type="submit" name="iperf_stop_button" value=" Stop IPerf ! " disabled="disabled" />
</div>
<br />
EOF
else
cat <<EOF
<div class="settings">
<h3><strong>@TR<<Test Command>></strong></h3>
<input style="margin-left: 3em;" type="submit" name="iperf_button" value=" Run IPerf !" disabled="disabled"/>
<input style="margin-left: 20em;" type="submit" name="iperf_stop_button" value=" Stop IPerf ! " />
</div>
<br />
EOF
fi

OUTPUT_CHECK_DELAY=1  # secs in pseudo-tail check
diag_command_output=""
diag_command=""

cat <<EOF
<table id="my_id">
<tbody>
<tr>
<div class="settings">    
<h3><strong>@TR<<Diagnostics Table>></strong></h3>
<td>
EOF
?>
        <object type="image/svg+xml" data="/cgi-bin/webif/graph_iperf_svg.sh?if=iperf"
		width="500" height="250">
		<param name="src" value="/cgi-bin/webif/graph_if_svg.sh?if=<? echo -n wlan0 ?>" />
		<a href="http://www.adobe.com/svg/viewer/install/main.html">If the graph is not fuctioning download the viewer here.</a>
	</object>

<?
cat <<EOF
</td>
<td>
<div id="logarea">
<pre> 
EOF

# determine if a process exists, by PID
does_process_exist() {
	# $1=PID
	ps | cut -c 1-6 | grep -q "$1 "
}

! empty "$FORM_iperf_button" && {
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
	#
	# every one second take a snapshot of the output file and output new lines since last snapshot.	
	# we force synchronization by stopping the outputting process while taking a snapshot
	# of its output file.
	#
	# TODO: Bug.. occasisionally lines can get skipped with this method, look into.
	#	
	echo "@TR<<Please wait for output of>> \"$diag_command\" ...<br />"
	tmpfile=$(mktemp /tmp/.webif-diag-XXXXXX)
	tmpfile2=$(mktemp /tmp/.webif-diag-XXXXXX)
	tmpfile3=$(mktemp /tmp/.webif-diag-XXXXXX)
	$diag_command 2>&1 > "$tmpfile" &
	ps_search=$(echo "$diag_command" | cut -c 1-15) # todo: limitation, X char match resolution	
	ps_results=$(ps | grep "$ps_search" | grep -v "grep")
	_pid=$(echo $ps_results | cut -d ' ' -f 1 | sed 2,99d)    # older busybox
#	equal $_pid "0" && _pid=$(echo $ps_results | cut -d ' ' -f 1 | sed 2,99d)  # newer busybox

	output_snapshot_file() {
		# output file
		# tmpfile2
		# PID		
		# stop process..	
		kill -s STOP $3 
		exists "$1" && {
			linecount_1=$(cat "$1" | wc -l | tr -d ' ') # current snapshot size
			linecount_2=$(cat "$2" | wc -l | tr -d ' ') # last snapshot size
			let new_lines=$linecount_1-$linecount_2

			! equal "$new_lines" "0" && {

				tail -n 1 "$1" > "$tmpfile3"
				lastline_size=$(cat "$tmpfile3" | wc -c)
				if [ "$lastline_size" -gt "2" ];then
					#echo -n "<pre>"
					tail -n $new_lines "$1"
					tail -n $new_lines "$1" > /var/run/iperf_run
					#echo "</pre>"
					cp "$1" "$2"
				else
					sed '$d' < "$1" > "$tmpfile3"	
					#echo -n "<pre>"
					tail -n $new_lines "$tmpfile3"
					#echo "</pre>"
					cp "$tmpfile3" "$2" 	
				fi
			}
		}
		# continue process..	
		kill -s CONT $3
	}

	if empty "$_pid" || equal "$_pid" "0"; then
		# exited before we could get PID
		echo "Error: Utility terminated too quick."			
	else		
		touch "$tmpfile2" # force to exist first iter
		while does_process_exist "$_pid"; do
			output_snapshot_file "$tmpfile" "$tmpfile2" "$_pid"
			sleep $OUTPUT_CHECK_DELAY
		done
		output_snapshot_file "$tmpfile" "$tmpfile2" 
		echo "<br />"
	fi	
	rm -f "$tmpfile2"	
	rm -f "$tmpfile3"	
	rm -f "$tmpfile"
        rm -f /var/run/iperf_run 
        rm -f /var/run/iperf_unit 
}

cat <<EOF
</pre>
</div>
</td>
</div>
</tr>
</tbody>
</table>
<div class="clearfix">&nbsp;</div>
EOF

 footer ?>
#<!--
###WEBIF:name:Status:995:Iperf
#-->
