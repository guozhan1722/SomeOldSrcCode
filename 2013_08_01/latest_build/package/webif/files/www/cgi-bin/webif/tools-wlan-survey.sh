#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

#################################
#
# Wireless survey page
#
# Description:
#	Perform a wireless survey and display pretty results.
#
# Author(s) [in order of work date]:
#	Jeremy Collake <jeremy.collake@gmail.com>
#	Travis Kemen <thepeople@berlios.de>
#
# Completely rewritten for speed improvement into pure AWK by
#	Stefan Stapelberg <stefan@rent-a-guru.de>

MACLIST_URL="http://standards.ieee.org/cgi-bin/ouisearch?"

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
	        wifi-device) append devices "$cfg_name" ;;
	        wifi-iface)  append vface "$cfg_name" "$N" ;;
	esac
}

uci_load wireless

for device in $devices; do
	config_get type $device type
	[ "$type" = "broadcom" ] && {
		config_get disabled $device disabled
		[ "$disabled" = "0" ] && scan_iface=1
	}
	[ "$type" = "atheros" ] && {
		config_get disabled $device disabled
		[ "$disabled" = "0" ] && atheros_devices="$atheros_devices $device"
	}
	[ "$type" = "mac80211" ] && {
		config_get disabled $device disabled
		[ "$disabled" = "0" ] && {
			scan_iface=1
		}
	}
done

for ath_device in $atheros_devices; do
	for i in $vface; do
		config_get iface $i device
		if [ "$iface" = "$ath_device" ]; then
			config_get mode $i mode
#			[ "$mode" = "sta" ] && scan_iface=1
			scan_iface=1
		fi
	done
done

header "Tools" "Site Survey" "@TR<<Site Survey>>"

equal "$scan_iface" "1" || {
	echo '<div class="settings">'
	echo "@TR<<No wireless networks were found>>."
	echo '</div>'
	footer_nosubmit
	exit
}

SCAN_BUTTON='@TR<<Start the scan now>>'
tempfile=$(mktemp /tmp/.survtemp.XXXXXX)

if [ "$FORM_clientswitch" != "" ]; then
	SCAN_BUTTON='@TR<<Start the scan again>>'
	#echo "<p style=\"margin: 1em 0;\">@TR<<Please wait, the scan needs some time>> ...</p>"

	for device in $devices; do
		config_get type $device type
		[ "$type" = "broadcom" ] && {
			wifi down
			wlc stdin <<EOF
down
vif 0
enabled 0
vif 1
enabled 0
vif 2
enabled 0
vif 3
enabled 0

ap 1
mssid 1
apsta 0
infra 1

802.11d 0
802.11h 0
rxant 3
txant 3

radio 1
macfilter 0
maclist none
wds none

country IL0
channel 5
maxassoc 128
slottime -1

vif 0
closed 0
ap_isolate 0
up
vif 0
vlan_mode 0
ssid OpenWrt
enabled 1
EOF
			break
		}
		[ "$type" = "atheros" ] && {
			wifi down >/dev/null
			wlanconfig ath0 create wlandev wifi0 wlanmode sta >/dev/null
			ifconfig ath0 up
			sleep 3
		}
		[ "$type" = "mac80211" ] && {
                       for i in $vface; do                                     
                               config_get if_device $i device                  
                               if [ "$if_device" = "$device" ]; then
                                       config_get if_name $i ifname  
                                       config_get if_mode $i mode
                                       [ "$if_mode" = "repeater" ] && "$if_name"="$if_name"".rep"
                                       got_result=0
                                       iwlistfile="/var/run/iwlist$device" 
                                       for j in 1 2 3 4; do
                                           [ "$if_mode" = "repeater" ] || {
                                                ifconfig $if_name down  
                                                ifconfig $if_name up 
                                           }

	                                   iwlist "$if_name" scan 2>/dev/null | tee "$iwlistfile" | grep -qi 'Cell [0-9][0-9] - Address:' && {
		                                got_result=1
		                                break;
	                                   } || sleep 1
                                       done
                                fi                                   
                       done 
		}
	done
fi


cat <<-EOF
<div class="settings">
<form enctype="multipart/form-data" method="post" action="$SCRIPT_NAME">
<h3><strong>@TR<<Wireless Survey>></strong></h3>
<p style="margin-bottom: 1em;">
<span class="red">@TR<<wlan_survey_warning#<b>Note:</b> Your WLAN traffic will be interrupted during this brief period.>></span>
</p>
<input style="margin-left: 2em;" type="submit" name="clientswitch" value=" $SCAN_BUTTON " />
</form>
<div class="clearfix">&nbsp;</div>
</div>
<br />
EOF

scanresult=$(ls /var/run/iwlistradio*)
equal "$got_result" "1" || {
[ -z "$scanresult" ] && {
	SCAN_BUTTON="@TR<<Start survey>>"
	echo '<div class="settings" style="margin-top: 1em;">'

	equal "$FORM_clientswitch" "1" && {
		echo '<p>@TR<<status_wlan_noresults#Sorry, there are no scan results. Please do a search by clicking on>>'
	echo "<i>${SCAN_BUTTON}</i> @TR<<above>>.</p>"
	}
	echo '</div>'
	footer_nosubmit
	rm "$tempfile"
	exit
}
}

FORM_tables()
{
awk -v "maclist_url=$MACLIST_URL" '
BEGIN { nc = 0; }
	$1 == "Cell" && $4 == "Address:" {
	nc = nc + 1;	# strip leading zero
	macaddr[nc] = $5;
}
nc > 0 {
	if ($1 ~ /^ESSID:/) {
		sub(/^  *ESSID:"/, "", $0);
		sub(/"$/, "", $0);
		essid[nc] = $0;
		next;
	}
        if ($1 ~ /^Frequency:/) {                            
		split($1, arr, ":");
		freq[nc] = arr[2] $2;

                split($3, arr, "(");
                if(arr[2] == "Channel")
                {                                
                        split($4, arr, ")");
                        channel[nc] = arr[1];
                }                                            
		next;
        }   
	if ($1 ~ /^Quality/ && $2 ~ /^Signal$/) {
		split($1, arr, "=");
		quality[nc] = arr[2];
	
		split($3, arr, "=");
		siglevel[nc] = arr[2];
	
		split($6, arr, "=");
		noiselevel[nc] = arr[2];
		next;
	}

	if ($1 ~ /^Authentication/)  {
                auth[nc]=1
		next;
	}

	if ($1 ~ /^Encryption/) {
		split($2, arr, ":");
		enc[nc] = arr[2];
	}
}
END {	
    j=1;
    for (i in channel) {
        indx[j] = i;
        chtmp[j]=channel[i];
        j++;
    }

    for (i=1; i<= nc-1; i++) {
        chan=chtmp[i];
        for (j=i+1;j<=nc;j++)
        {
            if (chtmp[i] > chtmp[j]) {
                tmp=indx[i];
                indx[i]=indx[j];
                indx[j]=tmp;
                tmp=chtmp[i];
                chtmp[i]=chtmp[j];
                chtmp[j]=tmp;
            }
        }
    }

    for (i=1; i <= nc; i++) {

		split(macaddr[indx[i]], arr, ":");

		abbr_macaddr = arr[1] "-" arr[2] "-" arr[3];

                if (noiselevel[indx[i]] == "") noiselevel[indx[i]] = "-110";

		noise_delta = -95 - noiselevel[indx[i]];
		integrity = siglevel[indx[i]] + noise_delta;
                snr = siglevel[indx[i]] - noiselevel[indx[i]] ;
                RSSIDBM = siglevel[indx[i]] - noiselevel[indx[i]] - 95;
                RSSIPER = ( siglevel[indx[i]] - noiselevel[indx[i]] ) * 100 / 30
                if (RSSIPER < 0) RSSIPER=0;
                if (RSSIPER > 100) RSSIPER=100;
                if(RSSIPER <=10){                                 
                    backgcolor="#FF0000";                    
                    midbackgcolor="#FF0000";
                    per=0.5;
                }else if (RSSIPER>10 && RSSIPER < 35){           
                    backgcolor="#FFFF00";
                    midbackgcolor="#FF0000";
                    per=0.5*10/RSSIPER;
                    ffper2=70-RSSIPER;
                    ffper1=350/RSSIPER;
                }else{                                                       
                    backgcolor="#00FF00";
                    midbackgcolor="#FFFF00";
                    per=0.1*100/RSSIPER;
                    ffper2=235-RSSIPER;
                    ffper1=1000/RSSIPER;
                }                             
		img_url = "<img src=\"/images/";
		if (enc[indx[i]] == "on") {
			img_url = img_url "lock.png";
                        if (auth[indx[i]] == 1) enc[indx[i]] = "WPA/WPA2/PSK";
                        else enc[indx[i]] = "WEP"
                }    
		else	{
                    img_url = img_url "transmit.png";
                }
		img_url = img_url "\" width=\"16\" height=\"16\" border=\"0\" alt=\"\" />";
                if (i % 2 == 0)
                    print("<tr>");
                else
                    print("<tr class=\"odd\">");
	        printf("<td width=\"100\">%d</td>",channel[indx[i]]);
	        printf("<td width=\"170\">%s</td>",essid[indx[i]]);
	        printf("<td width=\"170\">%s</td>",macaddr[indx[i]]);
	        printf("<td width=\"170\">%s%s</td>",img_url,enc[indx[i]]);
	        printf("<td width=\"130\">%s</td>",freq[indx[i]]);
	        printf("<td width=\"130\">%s dBm</td>",RSSIDBM);
	        printf("<td width=\"130\">%d dB</td>",snr);
	        printf("<td width=\"130\">%d dBm</td>\n",noiselevel[indx[i]]);
                printf("<td><div class=\"progressbar\" id=\"RSSI\" style=\"width:200px\">");
                printf("<span class=\"progress\" style=\"background-color:%s;background: -webkit-gradient(linear, left top, right top, from(#FF0000), to(%s), color-stop(%s, %s)); background: -moz-linear-gradient(left,#FF0000 0%%,%s %s%%,%s %s%%);filter: progid:DXImageTransform.Microsoft.gradient(GradientType=1,startColorstr=%s, endColorstr=%s);width:%d%%\">%d%%</span>",backgcolor,backgcolor,per,midbackgcolor,midbackgcolor,ffper1,backgcolor,ffper2,midbackgcolor,backgcolor,RSSIPER,RSSIPER);
              	printf("</div></td>");
                printf("</td>");
                
                print("</tr>");

	}
}' "$1"
}


FORM_cells=$(awk -v "maclist_url=$MACLIST_URL" '
BEGIN { nc = 0; }
	$1 == "Cell" && $4 == "Address:" {
	nc = nc + 1;	# strip leading zero
	macaddr[nc] = $5;
}
nc > 0 {
	if ($1 ~ /^ESSID:/) {
		sub(/^  *ESSID:"/, "", $0);
		sub(/"$/, "", $0);
		essid[nc] = $0;
		next;
	}
        if ($1 ~ /^Frequency:/) {                            
                split($3, arr, "(");
                if(arr[2] == "Channel")
                {                                
                        split($4, arr, ")");
                        channel[nc] = arr[1];
                }                                            
                next;                            
        }   
	if ($1 ~ /^Quality/ && $2 ~ /^Signal$/) {
		split($1, arr, "=");
		quality[nc] = arr[2];
	
		split($3, arr, "=");
		siglevel[nc] = arr[2];
	
		split($6, arr, "=");
		noiselevel[nc] = arr[2];
		next;
	}
	if ($1 ~ /^Encryption/) {
		split($2, arr, ":");
		enc[nc] = arr[2];
	}
}
END {	for (i=1; i <= nc; i++) {
		split(macaddr[i], arr, ":");
		abbr_macaddr = arr[1] "-" arr[2] "-" arr[3];

                if (noiselevel[i] == "") noiselevel[i] = "-95";

		noise_delta = -95 - noiselevel[i];
		integrity = siglevel[i] + noise_delta;
		rssi = siglevel[i] + 95;
		psnr = rssi * 100 / 60;

		img_url = "<img src=\"/images/";
		if (enc[i] == "on")
			img_url = img_url "lock.png";
		else	img_url = img_url "transmit.png";
		img_url = img_url "\" width=\"16\" height=\"16\" border=\"0\" alt=\"\" />";

		printf("string|<tr><td class=\"wlscan\" valign=\"top\" nowrap=\"nowrap\">%s<span class=\"essid\">%s</span><br />\n",
			img_url, essid[i]);
		printf("string|<span class=\"wlinfo\"><a title=\"@TR<<Cell>> %d\" href=\"%s?%s\" target=\"_blank\">%s</a>\n",
			i, maclist_url, abbr_macaddr, macaddr[i]);
		printf("string| @TR<<on>> @TR<<Channel>> %d</span>", channel[i]);
		if (quality[i] != "0/0")
			printf("<br />\nstring|<span class=\"wlinfo\">@TR<<Quality>>: %s</span>", quality[i]);
		printf("</td>\n");
		printf("string|<td valign=\"top\">@TR<<Signal>> %d @TR<<dbm>> / @TR<<Noise>> %d @TR<<dbm>><br />\n",
			siglevel[i], noiselevel[i]);
		printf("progressbar|SNR|@TR<<SNR>> %d @TR<<dbm>>|200|%d|\"%d%%\"\n", integrity, psnr, psnr);
		printf("string|</td></tr><tr><td colspan=\"2\">&nbsp;</td></tr>\n");
	}
}' "$tempfile")

rm -f "$tempfile"

# Restore radio to its original state
empty "$FORM_clientswitch" || wifi >/dev/null 2>&1

count=1
for device in $devices; do

cat <<EOF
<div class="settings">
<table style="width: 100%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3><strong>Radio$count Survey Results</strong></h3></th>
</table>

<table style="width: 93%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<td width="100"><strong>Channel</td>
	<td width="170"><strong>SSID</td>
	<td width="170"><strong>MACDDR</td>
	<td width="170"><strong>Encryption</td>
	<td width="130"><strong>Frequency </td>
	<td width="130"><strong>RSSI</td>
	<td width="130"><strong>SNR</td>
	<td width="130"><strong>Noise</td>
	<td><strong>Signal Level</td>
</tr>
EOF
iwlistfile="/var/run/iwlist$device"
FORM_tables "$iwlistfile"
#FORM_tables "/etc/iwlist"
cat <<EOF
</table>
</div>
<br/>
EOF

count=$(( $count + 1 ))
done
waitprom=""
footer_nosubmit ?>
<!--
##WEBIF:name:Tools:900:Site Survey
-->
