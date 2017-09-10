#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

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

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	#case "$cfg_type" in
	#	general)
	#		append general_cfgs "$cfg_name" "$N"
	#	;;
	#esac
}

################################################################

FORM_status_input1=$(cat /sys/class/button/INPUT1/status)
case "$FORM_status_input1" in
    1)     FORM_status_input1="<FONT COLOR=red>Close</FONT>" ;;	# button pressed		     
    0)     FORM_status_input1="Open"  ;;
esac
FORM_status_input2=$(cat /sys/class/button/INPUT2/status)
case "$FORM_status_input2" in
    1)     FORM_status_input2="<FONT COLOR=red>Close</FONT>" ;;	# button pressed		     
    0)     FORM_status_input2="Open"  ;;
esac
FORM_status_input3=$(cat /sys/class/button/INPUT3/status)
case "$FORM_status_input3" in
    1)     FORM_status_input3="<FONT COLOR=red>Close</FONT>" ;;	# button pressed		     
    0)     FORM_status_input3="Open"  ;;
esac
FORM_status_input4=$(cat /sys/class/button/INPUT4/status)
case "$FORM_status_input4" in
    1)     FORM_status_input4="<FONT COLOR=red>Close</FONT>" ;;	# button pressed		     
    0)     FORM_status_input4="Open"  ;;
esac

#################################################################

FORM_status_output1=$(cat /sys/class/leds/OUTPUT1/brightness)
case "$FORM_status_output1" in
    0)     FORM_status_output1="Open" ;;			     
    *)     FORM_status_output1="<FONT COLOR=red>Close</FONT>"  ;;
esac
FORM_status_output2=$(cat /sys/class/leds/OUTPUT2/brightness)
case "$FORM_status_output2" in
    0)     FORM_status_output2="Open" ;;			     
    *)     FORM_status_output2="<FONT COLOR=red>Close</FONT>"  ;;
esac
FORM_status_output3=$(cat /sys/class/leds/OUTPUT3/brightness)
case "$FORM_status_output3" in
    0)     FORM_status_output3="Open" ;;			     
    *)     FORM_status_output3="<FONT COLOR=red>Close</FONT>"  ;;
esac
FORM_status_output4=$(cat /sys/class/leds/OUTPUT4/brightness)
case "$FORM_status_output4" in
    0)     FORM_status_output4="Open" ;;			     
    *)     FORM_status_output4="<FONT COLOR=red>Close</FONT>"  ;;
esac







########### Rendering the input status #############################

append forms1 "start_form|$device @TR<< INPUT STATUS>>" "$N"

input1="field|@TR<< INPUT 1 >>|input1_status|
	caption|$FORM_status_input1"
append forms1 "$input1" "$N"

input2="field|@TR<< INPUT 2 >>|input2_status|
	caption|$FORM_status_input2"
append forms1 "$input2" "$N"

input3="field|@TR<< INPUT 3 >>|input3_status|
	caption|$FORM_status_input3"
append forms1 "$input3" "$N"

input4="field|@TR<< INPUT 4 >>|input4_status|
	caption|$FORM_status_input4"
append forms1 "$input4" "$N"

append forms1 "end_form" "$N"

########### Rendering the output status #############################

append forms2 "start_form|$device @TR<< OUTPUT STATUS>>" "$N"

output1="field|@TR<< OUTPUT 1 >>|output1_status|
	caption|$FORM_status_output1"
append forms2 "$output1" "$N"

output2="field|@TR<< OUTPUT 2 >>|output2_status|
	caption|$FORM_status_output2"
append forms2 "$output2" "$N"

output3="field|@TR<< OUTPUT 3 >>|output3_status|
	caption|$FORM_status_output3"
append forms2 "$output3" "$N"

output4="field|@TR<< OUTPUT 4 >>|output4_status|
	caption|$FORM_status_output4"
append forms2 "$output4" "$N"

append forms2 "end_form" "$N"

header "I\/O" "Status" "I/O Status" '' "$SCRIPT_NAME"

display_form <<EOF
$forms1
$forms2
EOF

echo "<div class=\"settings\" style=\"float: right;\">"
echo "<form name=\"refresh\" method=\"post\" action=\"$SCRIPT_NAME\">"

[ "$FORM_interval" -gt 0 ] >/dev/null 2>&1 && {
	cat <<EOF
<input  type="submit" value=" @TR<<system_info_Stop_Refreshing#Stop Refreshing>> " name="refreshstop" />
@TR<<system_info_Interval#Interval>>: $FORM_interval(@TR<<system_info_in_seconds#s>>)
EOF
} || {
	cat <<EOF
<input  type="submit" value=" @TR<<system_info_Auto_Refresh#Auto Refresh>> " name="refreshstart" />
@TR<<system_info_Interval#Interval>>:

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
(@TR<<system_info_in_seconds#s>>)
EOF
}
echo "</form></div>"

footer_nosubmit ?>
<!--
##WEBIF:name:I/O:100:Status
-->
