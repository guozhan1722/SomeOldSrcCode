#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
	[ -n "$1" ] && eval "$1_cfg=\"$2\""
}

uci_load "syslog"

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

filter_temp="/tmp/.webif.log-read.tmp"
if [ -n "$FORM_clearfilter" ]; then
	rm -f "$filter_temp" 2>/dev/null
	unset FORM_filtext FORM_filtmode
fi
if [ -n "$FORM_newfilter" ]; then
	echo "# this file is automatically generated by webi^2 page" > "$filter_temp" 2>/dev/null
	echo "# for temporary processing; you are free to delete it" >> "$filter_temp" 2>/dev/null
	echo "filtext=$FORM_filtext" >> "$filter_temp" 2>/dev/null
	echo "filtmode=$FORM_filtmode" >> "$filter_temp" 2>/dev/null
else 
	if [ -e "$filter_temp" ]; then
		FORM_filtext=$(sed '/^filtext=/!d; s/^filtext=//' "$filter_temp" 2>/dev/null)
		FORM_filtmode=$(sed '/^filtmode=/!d; s/^filtmode=//' "$filter_temp" 2>/dev/null)
	fi
fi
if [ "$FORM_filtmode" != "include" -a "$FORM_filtmode" != "exclude" ]; then
	FORM_filtmode="include"
fi
[ -n "$FORM_filtext" ] && filtered_title=" (@TR<<log_filter_filtered#filtered>>)"

header "Log" "Syslog" "@TR<<Syslog View>>"

cat <<EOF
<div class="settings">
<h3><strong>@TR<<log_read_Syslog_Messages#Syslog Messages>>$filtered_title</strong></h3>
<div id="logarea">
<pre>
EOF
eval logtype="\$CONFIG_${syslogd_cfg}_type"
eval logfile="\$CONFIG_${syslogd_cfg}_file"
if [ "$logtype" = "file" ]; then
	syslog_cmd="cat \"$logfile\""
else
	syslog_cmd="logread"
fi
eval $syslog_cmd 2>/dev/null | awk -v "filtmode=$FORM_filtmode" -v "filtext=$FORM_filtext" '
BEGIN {
	msgcntr = 0
}
function print_sanitize(msg) {
	gsub(/&/, "\\&amp;", msg)
	gsub(/</, "\\&lt;", msg)
	gsub(/>/, "\\&gt;", msg)
	print msg
}
{
	if (filtmode == "include") {
		if ($0 ~ filtext) {
			print_sanitize($0)
			msgcntr++
		}
	} else {
		if ($0 !~ filtext) {
			print_sanitize($0)
			msgcntr++
		}
	}
}
END {
	if (msgcntr == 0) print "@TR<<log_read_no_messages#There are no syslog messages.>>"
}'
echo " </pre>"
echo "</div>"
echo "<div class=\"clearfix\">&nbsp;</div></div>"

FORM_filtext=$(echo "$FORM_filtext" | sed 's/&/\&amp;/; s/"/\&#34;/; s/'\''/\&#39;/; s/\$/\&#36;/; s/</\&lt;/; s/>/\&gt;/; s/\\/\&#92;/; s/|/\&#124;/;')

display_form <<EOF
formtag_begin|filterform|$SCRIPT_NAME
start_form|@TR<<log_filter_Text_Filter#Text Filter>>
field|@TR<<log_filter_Text_to_Filter#Text to Filter>>
text|filtext|$FORM_filtext
field|@TR<<log_filter_Filter_Mode#Filter Mode>>
select|filtmode|$FORM_filtmode
option|include|@TR<<log_filter_Include#Include>>
option|exclude|@TR<<log_filter_Exclude#Exclude>>
string|</td></tr><tr><td>
submit|clearfilter|@TR<<log_filter_Remove_Filter#Remove Filter>>
string|</td><td>
submit|newfilter|@TR<<log_filter_Filter_Messages#Filter Messages>>
helpitem|log_filter_Text_to_Filter#Text to Filter
helptext|log_filter_Text_to_Filter_helptext#Insert a string that covers what you would like to see or exclude. In fact you can use the reqular expression constants like: <code>00:[[:digit:]]{2}:[[:digit:]]{2}</code> or <code>.debug&#124;.err</code>.
helpitem|log_filter_Filter_Mode#Filter Mode
helptext|log_filter_Filter_Mode_helptext#You will see only messages containing the text in the Include mode while you will not see them in the Exclude mode.
end_form
formtag_end
EOF

footer ?>
#<!--
###WEBIF:name:Log:002:Syslog
#-->
