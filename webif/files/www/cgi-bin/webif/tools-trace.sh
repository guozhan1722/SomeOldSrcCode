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

echo "page_refresh=1" > /var/run/trace_refresh

stop_trace()
{
    trace_pid=`ps|grep "traceroute" | grep -v "grep"| awk '{print $1}'`
    empty "$trace_pid" || kill -s 2 $trace_pid 
}
		
uci_load trace
[ "$?" != "0" ] && {
	uci_set_default trace <<EOF
config 'tool' 'trace'
	option 'trace_hostname' 'google.com'
EOF
	uci_load trace
}
		
tool="trace"
tmpfile="/var/run/trace_result_tmp"
		
if empty "$FORM_trace_button"; then
       config_get FORM_trace_hostname $tool trace_hostname google.com
       echo "page_refresh=30" > /var/run/trace_refresh
else
       FORM_trace_hostname="$FORM_hostname"
       sanitized=$(echo "$FORM_trace_hostname" | awk -f "/usr/lib/webif/sanitize.awk")	
       ! empty "$sanitized" && {
            stop_trace 
            echo "Please wait for output \"traceroute $sanitized\"...<br/>" > "$tmpfile"
            traceroute $sanitized 2>&1 | awk '{printf("%s <br/>",$0);}' >> "$tmpfile" &
            uci_set "trace" "$tool" "trace_hostname" "$FORM_trace_hostname"
            echo "page_refresh=1" > /var/run/trace_refresh
        }
fi
		
! empty "$FORM_stop_button" && {		
    stop_trace
    echo "The Tool for trace is Stopped " >> "$tmpfile"
    echo "page_refresh=30" > /var/run/trace_refresh
}

! empty "$FORM_clear_button" && {		
    stop_trace
    echo "The Output Result Clear " > "$tmpfile"
    echo "page_refresh=30" > /var/run/trace_refresh

}
		
		
header "Tools" "TraceRoute" "@TR<<Network TraceRoute>>" '' "$SCRIPT_NAME"

display_form <<EOF
start_form|@TR<<TraceRoute Network Utilities>>
field|@TR<<Tracerout Host Name>>
text|hostname|$FORM_trace_hostname
submit|trace_button|@TR<< Run TraceRoute >>
submit|stop_button|@TR<< Stop TraceRoute>>
submit|clear_button|@TR<< Clear Result>>
end_form
EOF

cat <<EOF  
<div>
<iframe src="/cgi-bin/webif/tools-trace_data.sh" height="300" width="95%"></iframe>
</div>
EOF

 footer ?>
<!--
##WEBIF:name:Tools:992:TraceRoute
-->
