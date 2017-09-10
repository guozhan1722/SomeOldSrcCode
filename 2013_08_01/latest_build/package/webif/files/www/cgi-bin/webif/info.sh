#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
. /etc/version

#header "Info" "System" "<img src=\"/images/blkbox.jpg\" alt=\"@TR<<System Information>>\"/>@TR<<System Information>>"
header "Info" "System" "@TR<<System Information>>"

[ -z "$_device" ] && _device="unidentified"
_kversion=$(uname -srv 2>/dev/null)
_mac=$(/sbin/ifconfig eth0 2>/dev/null | grep HWaddr | cut -b39-)
board_type=$(cat /proc/cpuinfo 2>/dev/null | sed 2,20d | cut -c16-)
[ -z "$board_type" ] && board_type=$(uname -m 2>/dev/null)
user_string="$REMOTE_USER"
[ -z "$user_string" ] && user_string="not logged in"
machinfo=$(uname -a 2>/dev/null)

package_filename="webif_latest.ipk"
if $(echo "$_firmware_version" | grep -q "r[[:digit:]]*"); then
	svn_path="trunk"
else
	svn_path="tags/kamikaze_$_firmware_version"
fi
version_url=$(sed '/^src[[:space:]]*X-Wrt[[:space:]]*/!d; s/^src[[:space:]]*X-Wrt[[:space:]]*//g; s/\/packages.*$/\//g' /etc/opkg.conf 2>/dev/null)
revision_text=" $SOFTWARE build $BUILD"
this_revision="$_webif_rev"
version_file=".version"
upgrade_button=""
_hostname=$(cat /proc/sys/kernel/hostname)
_date="`date +%F`"
_time="`date +%T`"


config_get_bool show_banner general show_banner 0
[ 1 -eq "$show_banner" ] && {
	echo "<pre>"
	cat /etc/banner 2>/dev/null
	echo "</pre><br />"
}

cat <<EOF
<table summary="System Information">
<tbody>
	<tr>
		<td width="100"><strong>@TR<<Firmware>></strong></td><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>
		<td>$_firmware_name </td>
	</tr>

	<tr>
		<td><strong>@TR<<Hardware>></strong></td><td>&nbsp;</td>
		<td>$HARDWARE</td>
	</tr>

	<tr>
		<td><strong>@TR<<User>></strong></td><td>&nbsp;</td>
		<td>$user_string</td>
	</tr>

	<tr>
		<td><strong>@TR<<Date>></strong></td><td>&nbsp;</td>
		<td>$_date</td>
	</tr>

	<tr>
		<td><strong>@TR<<Time>></strong></td><td>&nbsp;</td>
		<td>$_time</td>
	</tr>

	<tr>
		<td><strong>@TR<<Version>></strong></td><td>&nbsp;</td>
                <td>$revision_text</td>
	</tr>

	<tr>
		<td><strong>@TR<<Copyright>></strong></td><td>&nbsp;</td>
                <td>$VENDOR</td>
	</tr>

</tbody>
</table>
</table>

EOF

footer

?>
<!--
##WEBIF:name:Info:001:System
-->
