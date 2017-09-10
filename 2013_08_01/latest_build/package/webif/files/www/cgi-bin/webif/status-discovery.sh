#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

header "Status" "Discovery" "@TR<<Network Discovery>>"
discoveryfile="/var/run/netdiscovery"

bridge_name="br-lan"
br_ip="$(ifconfig "${bridge_name}" | grep inet | awk -F'[: ]+' '{print $4}')"

parse_target() {

   awk -v "heading=$2" -v "br_ip=$br_ip" '
   BEGIN {
      rulecntr=0
      odd=1
      print "   <tr>"
      print "      <td width=\"20%\">@TR<<MAC Address>></td>"
      print "      <td width=\"10%\">@TR<<IP Address>></td>"
      print "      <td width=\"10%\">@TR<<Discription>></td>"
      print "   </tr>"

      }
   function blankline()
   {
      print "	<tr>"
      print "		<td colspan=\"11\"><br /></td>"
      print "	</tr>"
   }
   /^(#.*)?$/ {next}
   {
      if (odd == 1) {
         print "	<tr>"
         odd--
      } else {
         print "	<tr class=\"odd\">"
	 odd++
      }

      print "		<td>" $1 "</td>"
      printf "		<td><a href='http://%s' target='_blank'>"$2"</a></td>", $2
      print "		<td>" $3 "</td>"
      print "	</tr>"
      rulecntr++
   }

   END {
      blankline()
      print "</tbody>"
      }' "$discoveryfile"
}

DISCOVERY_BUTTON='@TR<<Start discovery network now>>'
if [ "$FORM_disbutton" != "" ]; then
   DISCOVERY_BUTTON='@TR<<Start discovery network again>>'
   sdpClient br-lan 1 0   

fi
display_form <<EOF
start_form|@TR<<Network Discovery>>
<table style="width: 30%; margin-left: 2.5em; text-align: left; font-size: 0.8em;" border="1" cellpadding="3" cellspacing="2"> <tbody>
EOF
parse_target "$discoveryfile"
display_form <<EOF
end_form
EOF

cat <<-EOF
<div class="settings">
<form enctype="multipart/form-data" method="post" action="$SCRIPT_NAME">
<input style="margin-left: 12em;" type="submit" name="disbutton" value=" $DISCOVERY_BUTTON " />
</form>
<div class="clearfix">&nbsp;</div>
</div>
<br />
EOF



footer ?>
#<!--
###WEBIF:name:Status:750:Discovery
#-->
