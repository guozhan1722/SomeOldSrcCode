#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
###################################################################
# Mesh status page
#
# Description:
#	Shows Mesh Status
#
# Author(s) [in order of work date]:
#	Original webif developers
#	Bob Sheng
#
# Major revisions:
#
# NVRAM variables referenced:
#	none
#
# Configuration files referenced:
#	none
#

header_inject_head=$(cat <<EOF
	<meta http-equiv="refresh" content="30" />
EOF
)

header "Status" "Mesh" "@TR<<Mesh Routing Table>>"

?>
<div class="settings">
<table style="width: 50%; text-align: left; font-size: 0.8em;" border="0" 
cellpadding="3" cellspacing="3" align="center"> <tbody>
<?
parse_target() {
	equal "$1" "" && return

	echo "$1" | awk '
	BEGIN {
		print "	<tr>"
                print "         <td>Destination</td>"
                print "         <td>Next Hop</td>"
                print "         <td>Interface</td>"
                print "	</tr>"
		rulecntr=-1
	}
	function blankline()
	{
		print "	<tr>"
		print "		<td colspan=\"11\"><br /></td>"
		print "	</tr>"
	}
	/^(#.*)?$/ {next}
	$1 == "DEST_ADDR" {
		print "	<tr>"
		for (i=1; i<=3; i++)
			printf "%s%s%s%s%s\n", "		<th>@TR<<status_mesh_col_", $i, "#", $i, ">></th>" 
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
		print "		<td>" $1 "</td>"
		print "		<td>" $2 "</td>"
		print "		<td>" $3 "</td>"
		print "	</tr>"
		rulecntr++
	}
	END {
		blankline()
	}'
}

iw dev wlan0 mpath dump 2>/dev/null > /tmp/mpath.txt
iw dev wlan1 mpath dump 2>/dev/null >> /tmp/mpath.txt
iw dev wlan2 mpath dump 2>/dev/null >> /tmp/mpath.txt                    
iw dev wlan3 mpath dump 2>/dev/null >> /tmp/mpath.txt  
                                                                                
parse_target "$(cat /tmp/mpath.txt)" 
?>
</tbody>
</table>
<div class="clearfix">&nbsp;</div></div>
<br />

<? footer ?>
<!--
##WEBIF:name:Status:410:Mesh
-->
