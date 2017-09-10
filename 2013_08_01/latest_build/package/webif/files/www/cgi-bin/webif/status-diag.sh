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
#logarea pre {
	margin: 0.2em auto 1em auto;
	padding: 3px;
	width: 98%;
	margin: auto;
	height: 300px;
	overflow: scroll;
	border: 1px solid;
	}
// -->
</style>

EOF
)
		
header "Status" "Diagnostics" "@TR<<Diagnostics>>" '' "$SCRIPT_NAME"

FORM_ping_hostname=${FORM_ping_hostname:-google.com}
FORM_tracert_hostname=${FORM_tracert_hostname:-google.com}
FORM_ping6_hostname=${FORM_ping6_hostname:-ipv6.google.com}
FORM_tracert6_hostname=${FORM_tracert6_hostname:-ipv6.google.com}
FORM_ping_count=${FORM_ping_count:-4}
FORM_ping_size=${FORM_ping_size:-56}


is_package_installed kmod-ipv6
if [ "$?" = "0" ]; then
	ipv6_forms="field|
text|ping6_hostname|$FORM_ping6_hostname
submit|ping6_button|@TR<<Ping6>>
field|
text|tracert6_hostname|$FORM_tracert6_hostname
submit|tracert6_button|@TR<<TraceRoute6>>"
fi

OUTPUT_CHECK_DELAY=1  # secs in pseudo-tail check
diag_command_output=""
diag_command=""

display_form <<EOF
start_form|@TR<<Ping Network Utilities>>
field|@TR<<Ping Host Name>>
text|ping_hostname|$FORM_ping_hostname
field|@TR<<Ping Count>>
text|ping_count|$FORM_ping_count
field|@TR<<Ping Size>>
text|ping_size|$FORM_ping_size
submit|ping_button|@TR<< Ping >>
end_form
EOF

display_form <<EOF
start_form|@TR<<Trace Route Network Utilities>>
field|@TR<<Trace Host Name>>                                  
text|tracert_hostname|$FORM_tracert_hostname
submit|tracert_button|@TR<<TraceRoute>>
$ipv6_forms
end_form
EOF

cat <<EOF  
<div class="settings">    
<h3><strong>@TR<<Diagnostics Table>></strong></h3>
<div id="logarea">
<pre> 
EOF

# determine if a process exists, by PID
does_process_exist() {
	# $1=PID
	ps | cut -c 1-6 | grep -q "$1 "
}

! empty "$FORM_ping_button" || ! empty "$FORM_ping6_button" || ! empty "$FORM_tracert_button" || ! empty "$FORM_tracert6_button"&& {
	! empty "$FORM_ping_button" && {		
		sanitized=$(echo "$FORM_ping_hostname" | awk -f "/usr/lib/webif/sanitize.awk")	
		! empty "$sanitized" && {
			diag_command="ping -c $FORM_ping_count -s $FORM_ping_size $sanitized"	
		}
	}

	! empty "$FORM_ping6_button" && {
		sanitized=$(echo "$FORM_ping6_hostname" | awk -f "/usr/lib/webif/sanitize.awk")
		! empty "$sanitized" && {
			diag_command="ping6 -c 4 $sanitized"
		}
	}

	! empty "$FORM_tracert_button" && {
		echo "$please_wait_msg"
		sanitized=$(echo "$FORM_tracert_hostname" | awk -f "/usr/lib/webif/sanitize.awk")	
		! empty "$sanitized" && {
			diag_command="traceroute $sanitized"		
		}
	}

	! empty "$FORM_tracert6_button" && {
		echo "$please_wait_msg"
		sanitized=$(echo "$FORM_tracert6_hostname" | awk -f "/usr/lib/webif/sanitize.awk")
		! empty "$sanitized" && {
			diag_command="traceroute6 $sanitized"
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
}

cat <<EOF
</pre>
</div>
<div class="clearfix">&nbsp;</div></div> 
EOF

 footer ?>
#<!--
###WEBIF:name:Status:990:Diagnostics
#-->
