#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
. /etc/version
header "Tools" "Discovery" "@TR<<Network Discovery>>"
discoveryfile="/var/run/netdiscovery"

bridge_name="br-lan"
br_ip="$(ifconfig "${bridge_name}" | grep inet | awk -F'[: ]+' '{print $4}')"

parse_target() {

   awk -v "heading=$2" -v "br_ip=$br_ip" '
   BEGIN {
      FS="\t";
      rulecntr=0
      odd=1
      }
   function blankline()
   {
      print "	<tr>"
      print "		<td colspan=\"11\"><br /></td>"
      print "	</tr>"
   }
   /^(#.*)?$/ {next}
   {
      rulecntr++
      if (odd == 1) {
         print "	<tr>"
         odd--
      } else {
         print "	<tr class=\"odd\">"
	 odd++
      }
      macaddr[rulecntr]= $1
      ipaddr[rulecntr] = $2
      desc[rulecntr] = $3
      pdname[rulecntr] = $5
      fmver[rulecntr] = $6
      mode[rulecntr] = $7
      ssid[rulecntr] = $8
      
   }

   END {
      for (i=1; i <= rulecntr; i++) {
            if (i % 2 == 0)
                print("<tr>");
            else
                print("<tr class=\"odd\">");
	    printf("<td>%s</td>",macaddr[i]);
	    printf("<td><a href='http://%s' target='_blank'>%s</a></td>",ipaddr[i],ipaddr[i]);
	    printf("<td>%s</td>",desc[i]);
	    printf("<td>%s</td>",pdname[i]);
	    printf("<td>%s</td>",fmver[i]);
	    printf("<td>%s</td>",mode[i]);
	    printf("<td>%s</td>",ssid[i]);
            print("</tr>");
        }
    }' "$1"
}

DISCOVERY_BUTTON='@TR<<Start discovery network now>>'
if [ "$FORM_disbutton" != "" ]; then
   DISCOVERY_BUTTON='@TR<<Start discovery network again>>'
   sdpClient br-lan 1 0   

fi

if [ "$PRODUCT" = "IPnDDL" ];then
    network_id="Network ID"
else
    network_id="SSID"
fi

cat <<EOF
<div class="settings">
<table style="width: 100%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3><strong>Network Discovery</strong></h3></th>
</table>

<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td><strong>MAC Address</td>
	<td><strong>IP Address</td>
	<td><strong>Description</td>
	<td><strong>Product Name</td>
	<td><strong>Firmware Ver</td>
	<td><strong>Mode</td>
	<td><strong>$network_id</td>
</tr>

EOF

parse_target "$discoveryfile"
cat <<EOF
</table></div>
<br/>
EOF

cat <<-EOF
<div class="settings">
<form enctype="multipart/form-data" method="post" action="$SCRIPT_NAME">
<input style="margin-left: 2em;" type="submit" name="disbutton" value=" $DISCOVERY_BUTTON " />
</form>
<div class="clearfix">&nbsp;</div>
</div>
<br />
EOF



footer_nosubmit ?>
<!--
##WEBIF:name:Tools:001:Discovery
-->
