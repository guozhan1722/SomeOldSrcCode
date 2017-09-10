
#!/bin/sh

. /usr/lib/webif/webif.sh
. /etc/version

_firmware_version="$CONFIG_general_firmware_version"
_firmware_name="$CONFIG_general_firmware_name"
_firmware_subtitle="$CONFIG_general_firmware_subtitle"

[ -z "$_device" ] && _device="unidentified"
_kversion=$(uname -srv 2>/dev/null)
_mac=$(/sbin/ifconfig eth0 2>/dev/null | grep HWaddr | cut -b39-)
board_type=$(cat /proc/cpuinfo 2>/dev/null | sed 2,20d | cut -c16-)
[ -z "$board_type" ] && board_type=$(uname -m 2>/dev/null)
user_string="admin"
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

echo "Firmware :  $_firmware_name - $_firmware_subtitle $_firmware_version"
echo "Hardware :  $HARDWARE"
echo "User     :  $user_string"
echo "Date     :  $_date"
echo "Time     :  $_time"
echo "Version  :  $revision_text"
echo "Copyright:  $VENDOR"

