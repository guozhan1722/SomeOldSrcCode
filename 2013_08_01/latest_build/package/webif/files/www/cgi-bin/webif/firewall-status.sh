#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

TEMPFILE="/var/run/iptables_status"

parse_target() {
	equal "$1" "" && return
	equal "$2" "" && return

	echo "$1" | awk -v "heading=$2" '
	BEGIN {
		print "	<tr>"
		print "		<th colspan=\"11\"><h3>" heading "</h3></th>"
		print "	</tr>"
		rulecntr=-1
	}
	function blankline()
	{
		print "	<tr>"
		print "		<td colspan=\"11\"><br /></td>"
		print "	</tr>"
	}
	function translatechain()
	{
		gsub(/^Chain /, "@TR<<status_iptables_Chain#Chain>> ")
		gsub(/\(policy /, "(@TR<<status_iptables_policy#policy>> ")
		gsub(/ packets,/, " @TR<<status_iptables_packets#packets>>,")
		gsub(/ bytes\)/, " @TR<<status_iptables_bytes#bytes>>)")
		gsub(/ references\)/, " @TR<<status_iptables_references#references>>)")
	}
	/^(#.*)?$/ {next}
	$1 == "Chain" {
		if (rulecntr >= 0 ) blankline()
		translatechain()
		print "	<tr>"
		print "		<td colspan=\"11\"><h4>" $0 "</h4></td>"
		print "	</tr>"
	}
	$1 == "num" {
		print "	<tr>"
		for (i=1; i<=10; i++)
			printf "%s%s%s%s%s\n", "		<th>@TR<<status_iptables_col_", $i, "#", $i, ">></th>"
		print "		<th>@TR<<status_iptables_col_options#options>></th>"
		print "	</tr>"
		rulecntr=0
		odd=1
	}
	$1 ~ /[[:digit:]]{1,4}/ {
		if (odd == 1) {
			print "	<tr>"
			odd--
		} else {
			print "	<tr class=\"odd\">"
			odd++
		}
		print "		<td align=\"left\">" $1 "</td>"
		print "		<td align=\"left\">" $2 "</td>"
		print "		<td align=\"left\">" $3 "</td>"
		if ($10 !~ /^[[:digit:]]{1,3}\.[[:digit:]]{1,3}\.[[:digit:]]{1,3}\.[[:digit:]]{1,3}/) {
			NF = NF + 1
			for (i = NF; i >= 5; i--) $(i) = $(i-1)
			$4 = ""
		}
		print "		<td>" $4 "</td>"
		print "		<td>" $5 "</td>"
		print "		<td>" $6 "</td>"
		print "		<td>" $7 "</td>"
		print "		<td>" $8 "</td>"
		print "		<td>" $9 "</td>"
		print "		<td>" $10 "</td>"
		lastjoin=$11
		for (i=12; i <= NF; i++)
			lastjoin = lastjoin " " $i
		print "		<td>" lastjoin "</td>"
		print "	</tr>"
		rulecntr++
	}
	END {
		blankline()
	}'
}

filter_status="$(iptables -L -nv --line-numbers -t filter 2>/dev/null)"
nat_status="$(iptables -L -nv --line-numbers -t nat 2>/dev/null)"
mangle_status="$(iptables -L -nv --line-numbers -t mangle 2>/dev/null)"
raw_status="$(iptables -L -nv --line-numbers -t raw 2>/dev/null)"

config_load firewall
config_get_bool disabled_firewall normal disabled_firewall 0

if empty "$FORM_gettable"; then
    [ -f /var/run/iptables_tmp_001 ] && {
        FORM_firewall=$(cat /var/run/iptables_tmp_001)
    } || {
        FORM_firewall="all"
    }
else
    FORM_firewall=$FORM_firewall_tables    
    echo "$FORM_firewall_tables" > /var/run/iptables_tmp_001
    if [ "$disabled_firewall" = "1" ];then
        echo "Firewall is disabled !!!" 
    fi 
fi

mode_display="start_form|
field|@TR<<Status and Rules>>
select|firewall_tables|$FORM_firewall
option|all|@TR<<All>>
option|filter|@TR<<Filter>>
option|nat|@TR<<Nat>>
option|raw|@TR<<Raw>>
option|mangle|@TR<<Mangle>>
submit|gettable|@TR<< Check >>
end_form"

append forms "$mode_display" "$N"

header "Firewall" "Status" "@TR<<Firewall Status>>" 'onload="modechange()"' "$SCRIPT_NAME"

display_form <<EOF
$forms
EOF

?>
<div class="settings">
<table style="width: 96%; text-align: left; font-size: 0.8em;" border="0" cellpadding="3" cellspacing="3" align="center">
<tbody>
<?
    FORM_firewall=$FORM_firewall_tables    
    if [ "$FORM_firewall" = "all" ];then
#        iptables-save -n > $TEMPFILE
        parse_target "$filter_status" "@TR<<status_iptables_Target_Filter#Target Filter>>"
        parse_target "$nat_status)" "@TR<<status_iptables_Target_NAT#Target NAT>>"
        parse_target "$mangle_status" "@TR<<status_iptables_Target_Mangle#Target Mangle>>"
        parse_target "$raw_status" "@TR<<status_iptables_Target_Raw#Target Raw>>"
    else
#        iptables-save -t $FORM_firewall > $TEMPFILE
        eval target="\${${FORM_firewall}_status}"
        parse_target "$target"  "Target $FORM_firewall"
    fi 

?>
</tbody>
</table>
</div>
<br />
<?

footer_nosubmit ?>
<!--
##WEBIF:name:Firewall:100:Status
-->
