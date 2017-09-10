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

echo "page_refresh=1" > /var/run/ping_refresh

stop_ping()
{
    ping_id=`ps|grep "ping -c" | grep -v "grep"| awk '{print $1}'`
    empty "$ping_id" || kill -s 2 $ping_id 
}

uci_load ping
[ "$?" != "0" ] && {
	uci_set_default ping <<EOF
config 'tool' 'ping'
	option 'ping_hostname' 'google.com'
	option 'ping_count' '4'
	option 'ping_size' '56'
EOF
	uci_load ping
}

tool="ping"
tmpfile="/var/run/ping_result_tmp"

if empty "$FORM_ping_button"; then
       config_get FORM_ping_hostname $tool ping_hostname google.com
       config_get FORM_ping_count $tool  ping_count 4
       config_get FORM_ping_size $tool ping_size 56
       echo "page_refresh=30" > /var/run/ping_refresh
else
       FORM_ping_hostname="$FORM_hostname"
       FORM_ping_count="$FORM_count"
       FORM_ping_size="$FORM_size"
       validate <<EOF
int|FORM_count|$tool @TR<<Count>>||$FORM_ping_count
int|FORM_size|$tool @TR<<Size>>||$FORM_ping_size
EOF
       [ "$?" = "0" ] && {
            sanitized=$(echo "$FORM_ping_hostname" | awk -f "/usr/lib/webif/sanitize.awk")	
            ! empty "$sanitized" && {
                stop_ping
                echo "Please wait for output of \"ping -c $FORM_ping_count -s $FORM_ping_size $sanitized\"..." > "$tmpfile"
                ping -c $FORM_ping_count -s $FORM_ping_size $sanitized 2>&1 | awk '{printf("%s <br/>",$0);}' >> "$tmpfile" &
	        #ping -c $FORM_ping_count -s $FORM_ping_size $sanitized 2>&1 >> "$tmpfile" &	
                uci_set "ping" "$tool" "ping_hostname" "$FORM_ping_hostname"
                uci_set "ping" "$tool" "ping_count" "$FORM_ping_count"
                uci_set "ping" "$tool" "ping_size" "$FORM_ping_size"
                echo "page_refresh=1" > /var/run/ping_refresh
            }
        }
fi
		
! empty "$FORM_stop_button" && {		
    stop_ping
    echo "The Tool for Ping is Stopped " >> "$tmpfile"
    echo "page_refresh=30" > /var/run/ping_refresh
}

! empty "$FORM_clear_button" && {		
    stop_ping
    echo "The Output Result Clear " > "$tmpfile"
    echo "page_refresh=30" > /var/run/ping_refresh

}

header "Tools" "Ping" "@TR<<Network Tools Ping>>" '' "$SCRIPT_NAME"

display_form <<EOF
start_form|@TR<<Ping Network Utilities>>
field|@TR<<Ping Host Name>>
text|hostname|$FORM_ping_hostname
field|@TR<<Ping Count>>
text|count|$FORM_ping_count
field|@TR<<Ping Size>>
text|size|$FORM_ping_size
submit|ping_button|@TR<< Ping >>
submit|stop_button|@TR<< Stop >>
submit|clear_button|@TR<< Clear >>
end_form
EOF

cat <<EOF  
<div>
<iframe src="/cgi-bin/webif/tools-ping_data.sh" height="300" width="95%"></iframe>
</div>
EOF

 footer ?>
<!--
##WEBIF:name:Tools:990:Ping
-->
