#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"

###################################################################
# system configuration page
#
# Description:
#	Configures general system settings.
#
# Author(s) [in order of work date]:
#   	Original webif developers -- todo
#	Markus Wigge <markus@freewrt.org>
#   	Jeremy Collake <jeremy.collake@gmail.com>
#	Travis Kemen <kemen04@gmail.com>
#
# Major revisions:
#
# Configuration files referenced:
#   none
#

# ntpcliconf variable left here incase name changes again.
ntpcliconf="ntpclient"

# vip board revision
vip_revision="`vip -rev get|awk '{print $4}'`"
. /etc/version

# Add NTP Server
if ! empty "$FORM_add_ntpcfg_number"; then
	uci_add "$ntpcliconf" "ntpserver"
	uci_set "$ntpcliconf" "$CONFIG_SECTION" "hostname" ""
	uci_set "$ntpcliconf" "$CONFIG_SECTION" "port" "123"
	FORM_add_ntpcfg=""
fi

# Remove NTP Server
if ! empty "$FORM_remove_ntpcfg"; then
	uci_remove "$ntpcliconf" "$FORM_remove_ntpcfg"
fi

unset SERVER
unset PORT
unset INTERVAL
unset COUNT
unset INTERFACE_GLOBAL

NTPC=`which ntpclient`

check_ntpserver() {
	local hostname
	local port
	local interface
	[ -n "$SERVER" ] && return
	config_get hostname $1 hostname
	config_get port $1 port
	config_get interface $1 interface

	[ -z "$interface" ] && interface=$INTERFACE_GLOBAL

	[ -n "$interface" ] && {
		# $INTERFACE is passed from hotplug event
		[ "$interface" = "$INTERFACE" ] || return
	}

	[ -z "$hostname" ] && return
	$NTPC -c 1 -p ${port:-123} -i 2 -h $hostname > /dev/null && { SERVER=$hostname; PORT=${port:-123}; }
}

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		system)
			hostname_cfg="$cfg_name"
		;;
		timezone)
			timezone_cfg="$cfg_name"
		;;
	        wifi-device)
	                echo "$cfg_name" |grep -q "0"
			if [ "$?" = "0" ]; then
   			   primary_wlan_phy="$cfg_name"
			fi

	                echo "$cfg_name" |grep -q "1"
			if [ "$?" = "0" ]; then
   			   secondary_wlan_phy="$cfg_name"
			fi
	        ;;
	        wifi-iface)
	                append vface "$cfg_name" "$N"
	        ;;
	        interface)
			if [ "wan" = "$cfg_name" ]; then
			   wan_cfg="$cfg_name" 
		        fi
	        ;;
		ntpserver)
			append ntpservers "$cfg_name" "$N"
		;;

		zone)
			append gzones "$cfg_name" "$N"
		;;
		ntpclient)
			append ntpclient_cfgs "$cfg_name" "$N"
	esac
}

uci_load "webif"
uci_load "system"
#We have to load the system host name setting here because ntp_client also uses the hostname setting.
eval CONFIG_systemhostname="\$CONFIG_${hostname_cfg}_hostname"
FORM_hostname="${FORM_hostname:-$CONFIG_systemhostname}"
eval CONFIG_systemsystemmode="\$CONFIG_${hostname_cfg}_systemmode"
#FORM_system_mode="${FORM_system_mode:-$CONFIG_systemsystemmode}"
#FORM_system_mode="${FORM_system_mode:-bridge}"

config_clear "$hostname_cfg"
uci_load "network"

uci_load timezone
[ "$?" != "0" ] && {
	uci_set_default timezone <<EOF
config 'timezone'
	option 'posixtz' 'MST7MDT,M3.2.0,M11.1.0'
	option 'zoneinfo' 'America/Denver'
EOF
	uci_load timezone

}
uci_load "$ntpcliconf"
uci_load "firewall"

#FIXME: uci_load bug
#uci_load will pass the same config twice when there is a section to be added by using uci_add before a uci_commit happens
#we will use uniq so we don't try to parse the same config section twice.
ntpservers=$(echo "$ntpservers" |uniq)

ntpcfg_number=$(echo "$ntpservers" |wc -l)
let "ntpcfg_number+=1"

# install NTP client if asked
if ! empty "$FORM_install_ntpclient"; then
	tmpfile=$(mktemp "/tmp/.webif_ntp-XXXXXX")
	echo "@TR<<system_settings_Installing_NTPCLIENT_package#Installing NTPCLIENT package>> ...<pre>"
	install_package "ntpclient"
	ACTION=ifup
	. /etc/hotplug.d/iface/20-ntpclient
	echo "</pre>"
fi

generate_ssl_key() {
	local inst_packages inst_links llib llink libsymlinks
	is_package_installed "zlib"
	[ "$?" != "0" ] && {
		inst_packages="zlib $inst_packages"
		inst_links="$inst_links /tmp/usr/lib/libz.so.*"
	}
	is_package_installed "libopenssl"
	[ "$?" != "0" ] && {
		inst_packages="libopenssl $inst_packages"
		inst_links="$inst_links /tmp/usr/lib/libssl.so.* /tmp/usr/lib/libcrypto.so.*"
	}
	is_package_installed "openssl-util"
	[ "$?" != "0" ] && {
		inst_packages="openssl-util $inst_packages"
		inst_links="$inst_links /tmp/usr/bin/openssl"
	}
	[ -n "$inst_packages" ] && opkg -force-overwrite -d ram install $inst_packages
	is_package_installed "openssl-util"
	if [ "$?" = "0" ]; then
		for llib in $inst_links; do
			llink=$(echo "$llib" | sed 's/\/tmp//')
			ln -s $llib $llink
			[ "$?" = "0" ] && libsymlinks="$libsymlinks $llink"
		done
		if [  -z "$(ps | grep "[n]tpd\>")" ]; then
			is_package_installed "ntpclient"
			[ "$?" != "0" ] && {
				echo "@TR<<system_settings_Updating_time#Updating time>> ..."
				rdate -s 0.openwrt.pool.ntp.org
			}
		fi
		export RANDFILE="/tmp/.rnd"
		dd if=/dev/urandom of="$RANDFILE" count=1 bs=512 2>/dev/null
		(openssl genrsa -out /etc/stunnel/stunnel.key 2048; openssl req -new -batch -nodes -key /etc/stunnel/stunnel.key -out /etc/stunnel/stunnel.csr; openssl x509 -req -days 365 -in /etc/stunnel/stunnel.csr -signkey /etc/stunnel/stunnel.key -out /etc/stunnel/stunnel.cert)
		rm -f "$RANDFILE" 2>/dev/null
		unset RANDFILE
	fi
	[ -n "$libsymlinks" ] && rm -f $libsymlinks
	[ -n "$inst_packages" ] && opkg remove $inst_packages
}

if ! empty "$FORM_install_stunnel"; then
	echo "@TR<<system_settings_Installing_STunnel_package#Installing STunnel package>> ...<pre>"
	install_package "stunnel"
	is_package_installed "stunnel"
	[ "$?" = "0" ] && [ ! -e /etc/stunnel/stunnel.key -o ! -e /etc/stunnel/stunnel.cert ] && {
		echo "@TR<<system_settings_Generating_SSL_certificate#Generating SSL certificate>> ..."
		generate_ssl_key
	}
	echo "</pre><br />"
fi
if ! empty "$FORM_generate_certificate"; then
	echo "@TR<<system_settings_Generating_SSL_certificate#Generating SSL certificate>> ...<pre>"
	generate_ssl_key
	echo "</pre><br />"
fi

#####################################################################
# initialize forms
if empty "$FORM_submit"; then
	# initialize all defaults
	eval time_zone_part="\$CONFIG_${timezone_cfg}_posixtz"
	eval time_zoneinfo_part="\$CONFIG_${timezone_cfg}_zoneinfo"
	time_zone_part="${time_zone_part:-"UTC+0"}"
	time_zoneinfo_part="${time_zoneinfo_part:-"-"}"
	FORM_system_timezone="${time_zoneinfo_part}@${time_zone_part}"

	# webif settings
	#FORM_effect="${CONFIG_general_use_progressbar}"		# -- effects checkbox
	#if equal $FORM_effect "1" ; then FORM_effect="checked" ; fi	# -- effects checkbox
	FORM_language="${CONFIG_general_lang:-en}"	
	FORM_theme=${CONFIG_theme_id:-xwrt}
	FORM_ssl_enable="${CONFIG_general_ssl:-0}"
	FORM_ssl_port="${CONFIG_general_ssl_port:-443}"
        FORM_usr_date="`date +%F | sed 's/-/./g'`"
        FORM_usr_time="`date +%T`"
        config_get FORM_usr_mode datetime mode
        [ -z $FORM_usr_mode ] && FORM_usr_mode="local" 
else
#####################################################################
# save forms
	SAVED=1
#when hostname has space , it will crash webpage
#validate <<EOF
#hostname|FORM_hostname|@TR<<Host Name>>||$FORM_hostname
#EOF
validate <<EOF
ports|FORM_ssl_port|@TR<<SSL port>>||$FORM_ssl_port
EOF

	if equal "$?" 0 ; then
		time_zone_part="${FORM_system_timezone#*@}"
		time_zoneinfo_part="${FORM_system_timezone%@*}"
		empty "$hostname_cfg" && {
			uci_add system system
			hostname_cfg="$CONFIG_SECTION"
		}
		uci_set "system" "$hostname_cfg" "hostname" "$FORM_hostname"
		#uci_set "system" "$hostname_cfg" "systemmode" "$FORM_system_mode" 
		uci_set "system" "$hostname_cfg" "timezone" "$time_zone_part"


		empty "$timezone_cfg" && {
			uci_add timezone timezone
			timezone_cfg="$CONFIG_SECTION"
		}
		uci_set timezone "$timezone_cfg" posixtz "$time_zone_part"
		uci_set timezone "$timezone_cfg" zoneinfo "$time_zoneinfo_part"
		for server in $ntpservers; do
			eval FORM_ntp_server="\$FORM_ntp_server_$server"
			eval FORM_ntp_port="\$FORM_ntp_port_$server"
			uci_set "$ntpcliconf" "$server" hostname "$FORM_ntp_server"
			uci_set "$ntpcliconf" "$server" port "$FORM_ntp_port"
		done

                [ "$FORM_datetime_mode" = "sync" ]  || {
                    sysdatetime=$(date -s "${FORM_datetime_date}-${FORM_datetime_time}")
                    [ -z "$sysdatetime" ] && {
                        ERROR="Invalid Date or Time Formats"
                    } || {
                        ERROR=""
                    uci_set "system" "datetime" "newloc" "1"
                    uci_set "system" "datetime" "diffsec" "0"
                    }
                }
                FORM_usr_mode="$FORM_datetime_mode"
                sleep 1
                FORM_usr_date="`date +%F | sed 's/-/./g'`"
                FORM_usr_time="`date +%T`"
                FORM_datetime_date="`date +%F | sed 's/-/./g'`"
                FORM_datetime_time="`date +%T`"

                uci_set "system" "datetime" "date" "$FORM_datetime_date"
                uci_set "system" "datetime" "time" "$FORM_datetime_time"
                uci_set "system" "datetime" "mode" "$FORM_usr_mode"
		# webif settings
		# fix emptying the field when not present
		FORM_ssl_enable="${FORM_ssl_enable:-$CONFIG_general_ssl}"
		FORM_ssl_enable="${FORM_ssl_enable:-0}"
                FORM_ssl_port="${FORM_ssl_port:-443}"
		uci_set "webif" "general" "ssl" "$FORM_ssl_enable"
		uci_set "webif" "general" "ssl_port" "$FORM_ssl_port"
		uci_set "firewall" "DEFRule_remote_https_man" "dest_port" "$FORM_ssl_port"
		uci_set "firewall" "DEFRule_remote_https_man2" "dest_port" "$FORM_ssl_port"
		#uci_set "webif" "theme" "id" "$FORM_theme"
		uci_set "webif" "general" "lang" "$FORM_language"
		#uci_set "webif" "general" "use_progressbar" "$FORM_effect_enable"
		#FORM_effect=$FORM_effect_enable ; if equal $FORM_effect "1" ; then FORM_effect="checked" ; fi
	#else
	#	echo "<br /><div class=\"warning\">@TR<<Warning>>: @TR<<system_settings_Hostname_failed_validation#Hostname failed validation. Can not be saved.>></div><br />"
	fi
fi

WEBIF_SSL="field|@TR<<system_settings_Webif_SSL#HTTP SSL>>"
#is_package_installed "stunnel"
#if [ "$?" != "0" ]; then
#	WEBIF_SSL="$WEBIF_SSL
#string|<div class=\"warning\">@TR<<system_settings_Feature_requires_stunnel#STunnel package is not installed. You need to install it for ssl support>>:</div>
#submit|install_stunnel| @TR<<system_settings_Install_stunnel#Install STunnel>> |"
#else
	if [ -e /etc/stunnel/stunnel.key -a -e /etc/stunnel/stunnel.cert ]; then
		WEBIF_SSL="$WEBIF_SSL
select|ssl_enable|$FORM_ssl_enable
option|0|@TR<<system_settings_webifssl_Off#Off>>
option|1|@TR<<system_settings_webifssl_On#On>>
field|@TR<<HTTP SSL PORT>>|http_ssl_port_field|hidden
text|ssl_port|$FORM_ssl_port"

javascript_forms="
        v = isset('ssl_enable','1');
        set_visible('http_ssl_port_field', v);
        "
append js "$javascript_forms" "$N"

	else
		WEBIF_SSL="$WEBIF_SSL
string|<div class=\"warning\">@TR<<system_settings_Feature_requires_certificate#The SSL certificate is missing. You need to generate it for ssl support>>:</div>
submit|generate_certificate| @TR<<system_settings_Generate_SSL_Certificate#Generate SSL Certificate>> |"
	fi
#fi

#	effect_field=$(cat <<EOF
#field| 
#string|<input type="checkbox" name="effect_enable" value="1" $FORM_effect />&nbsp;@TR<<Enable visual effects>><br/><br/>
#EOF
#)

#####################################################################
# Initialize THEMES form
#
#
# start with list of available installable theme packages
#
! exists "/etc/themes.lst" && {
	# create list if it doesn't exist ..
	/usr/lib/webif/webif-mkthemelist.sh	
}
THEMES=$(cat "/etc/themes.lst")
for str in $temp_t; do
	THEME="$THEME
		option|$str"
done

# enumerate installed themes by finding all subdirectories of /www/theme
# this lets users install themes not built into packages.
#
for curtheme in /www/themes/*; do
	curtheme=$(echo "$curtheme" | sed s/'\/www\/themes\/'//g)
	if exists "/www/themes/$curtheme/name"; then
		theme_name=$(cat "/www/themes/$curtheme/name")
	else
		theme_name="$curtheme"
	fi
	! equal "$curtheme" "active" && {
		THEMES="$THEMES
option|$curtheme|$theme_name"
	}
done
#
# sort list and remove dupes
#
THEMES=$(echo "$THEMES" | sort -u)

#####################################################################
# Initialize LANGUAGES form
# create list if it doesn't exist ..
/usr/lib/webif/webif-mklanglist.sh
LANGUAGES=$(cat "/etc/languages.lst")

if [ "$HARDWARE" = "v2.0.0" ]; then
    system_mode_form=""
else
{
	system_mode_form="field|@TR<<Default System Mode>>
	select|system_mode|$FORM_system_mode
	option|bridge|@TR<<Bridge>>
	option|router|@TR<<Router>>
	option|gateway|@TR<<Gateway>>"
}
fi
#####################################################################
# ntp form
for server in $ntpservers; do
	if empty "$FORM_submit"; then
		config_get FORM_ntp_server $server hostname
		config_get FORM_ntp_port $server port
	else
		eval FORM_ntp_server="\$FORM_ntp_server_$server"
		eval FORM_ntp_port="\$FORM_ntp_port_$server"
	fi
	#add check for blank config, the only time it will be seen is when config section is waitings to be removed
	if [ "$FORM_ntp_port" != "" -o "$FORM_ntp_server" != "" ]; then
		if [ "$FORM_ntp_port" = "" ]; then
			FORM_ntp_port=123
		fi
		ntp_form="field|@TR<<NTP Server>>|server_field_$server|hidden
		text|ntp_server_$server|$FORM_ntp_server
		field|@TR<<NTP Server Port>>|server_port_$server|hidden
		text|ntp_port_$server|$FORM_ntp_port
		field||remove_$server|hidden
		string|<a href=\"$SCRIPT_NAME?remove_ntpcfg=$server\">@TR<<Remove NTP Server>></a>"
		append NTP "$ntp_form" "$N"
	fi

javascript_forms="
        v = isset('datetime_mode','local');
        set_visible('server_field_$server', !v);
        set_visible('server_port_$server', !v);
        set_visible('remove_$server', !v);
        "
append js "$javascript_forms" "$N"

done

add_ntpcfg="field||add_ntpcfg_field|hidden
string|<a href=\"$SCRIPT_NAME?add_ntpcfg_number=$ntpcfg_number\">@TR<<Add NTP Server>></a>"
append NTP "$add_ntpcfg" "$N"

javascript_forms="
        v = isset('datetime_mode','local');
        set_visible('add_ntpcfg_field', !v);
        "
append js "$javascript_forms" "$N"

#if [ -n "$(has_pkgs ntpclient)" -a -n "$(has_pkgs openntpd)" ]; then
#	NTPCLIENT_INSTALL_FORM="string|<div class=\"warning\">@TR<<Warning>>: @TR<<system_settings_feature_requires_ntpclient#No NTP client is installed. For correct time support you need to install one>>:</div>
#		submit|install_ntpclient| @TR<<system_settings_Install_NTP_Client#Install NTP Client>> |"
#fi

#####################################################################
# initialize time zones

TIMEZONE_OPTS=$(
	awk -v timezoneinfo="$FORM_system_timezone" '
		BEGIN {
			FS="	"
			last_group=""
			defined = 0
		}
		/^(#.*)?$/ {next}
		$1 != last_group {
			last_group=$1
			print "optgroup|" $1
		}
		{
			list_timezone = $4 "@" $3
			if (list_timezone == timezoneinfo)
				defined = defined + 1
			print "option|" list_timezone "|@TR<<" $2 ">>"
		}
		END {
			if (defined == 0) {
				split(timezoneinfo, oldtz, "@")
				print "optgroup|@TR<<system_settings_group_unknown_TZ#Unknown>>"
				if (oldtz[1] == "-") oldtz[1] = "@TR<<system_settings_User_or_old_TZ#User defined (or out of date)>>"
				print "option|" timezoneinfo "|" oldtz[1]
			}
		}' < /usr/lib/webif/timezones.csv 2>/dev/null

)
#######################################################
# Web Services Form
uci_load httpd
cfg=$CONFIG_SECTION

if empty "$FORM_submit"; then
    config_get FORM_port "$cfg" port "80"
else
        FORM_port="$FORM_port_v"
        uci_set "httpd" "$cfg" "port" "$FORM_port"
        uci_set "firewall" "DEFRule_remote_man" "dest_port" "$FORM_port" 
        uci_set "firewall" "DEFRule_remote_man2" "dest_port" "$FORM_port"
fi

javascript_forms="
        v = isset('datetime_mode','local');
        set_visible('date_field', v);
        set_visible('time_field', v);
        set_visible('timezone_field', 1);
        set_visible('view_tz_string', !v);"
append js "$javascript_forms" "$N"

#####################################################################
header "System" "Settings" "@TR<<System Settings>>" ' onload="modechange()" ' "$SCRIPT_NAME"

#####################################################################

cat <<EOF
<script type="text/javascript" src="/webif.js"></script>
<script type="text/javascript">
<!--
function modechange()
{
	var tz_info = value('system_timezone');
	if ((tz_info=='') || (tz_info==null)){
		set_value('show_TZ', tz_info);
	}
	else {
		var tz_split = tz_info.split('@');
		set_value('show_TZ', tz_split[1]);
	}

	var v;
	$js

	hide('save');
	show('save');
}
-->
</script>
EOF

#cat <<EOF
#<script language="JavaScript">
#<!--
#date=Date()
#document.write(date)
#//-->
#</script>
#EOF
#######################################################
# Show form
display_form <<EOF
onchange|modechange
start_form|@TR<<System Settings>>
field|@TR<<Host Name>>
text|hostname|$FORM_hostname
$system_mode_form
end_form
start_form|@TR<<Time Settings : Current Date(yyyy.mm.dd)>> `date +%F | sed 's/-/./g'`  Time(hh:mm:ss): `date +%T`
field|@TR<<Timezone>>|timezone_field|hidden
select|system_timezone|$FORM_system_timezone
$TIMEZONE_OPTS
field|@TR<<Date and Time Setting Mode>>
select|datetime_mode|$FORM_usr_mode
option|local|@TR<<Use Local Time Source >>
option|sync|@TR<<Synchronize Date And Time Over Network>>
field|@TR<<Date (yyyy.mm.dd)>>|date_field|hidden
text|datetime_date|$FORM_usr_date
field|@TR<<Time (hh:mm:ss)>>|time_field|hidden
text|datetime_time|$FORM_usr_time
field|@TR<<system_settings_POSIX_TZ_String#POSIX TZ String>>|view_tz_string|hidden
string|<input id="show_TZ" type="text" style="width: 96%; height: 1.2em; color: #2f2f2f; background: #ececec; " name="show_TZ" readonly="readonly" value="@TR<<system_settings_js_required#This field requires the JavaScript support.>>" />
$NTP
end_form
$NTPCLIENT_INSTALL_FORM

##########################
# webif settings
#start_form|@TR<<Webif&sup2; Settings>>
#$effect_field
#field|@TR<<Language>>
#select|language|$FORM_language
#$LANGUAGES
#field|@TR<<Theme>>
#select|theme|$FORM_theme
#$THEMES
#end_form

start_form|@TR<<Web Configuration Settings>>
field|@TR<<HTTP Port>>
text|port_v|$FORM_port
$WEBIF_SSL
end_form
EOF

footer

 ?>

<!--
##WEBIF:name:System:010:Settings
-->
