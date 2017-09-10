#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		accesscontrol)
			append acl_cfgs "$cfg_name" "$N"
		;;
	esac
}
############### password Changing part ########################
if ! empty "$FORM_passwdchange" ; then
	SAVED=1
	validate <<EOF
string|FORM_pw1|@TR<<Password>>|required min=5|$FORM_pw1
EOF
	equal "$FORM_pw1" "$FORM_pw2" || {
		[ -n "$ERROR" ] && ERROR="${ERROR}<br />"
		ERROR="${ERROR}@TR<<Passwords do not match>><br />"
	}
       empty "$ERROR" && {
	       RES=$(
		       (
			       echo "$FORM_pw1"
			       sleep 1
			       echo "$FORM_pw2"
		       ) | passwd $REMOTE_USER 2>&1
	       )
	       equal "$?" 0 && {
                    ROOTFALSE=$(cat /etc/passwd |grep "root:"|grep "/bin/false")
                    ROOTTRUE=$(cat /etc/passwd |grep "root:"|grep "/bin/ash")
                    
                    empty "$ROOTFALSE"  &&  ROOTFALSE=$(echo $ROOTTRUE | sed '//s/\/bin\/ash/\/bin\/false/g')
                    empty "$ROOTTRUE"  &&  ROOTTRUE=$(echo $ROOTFALSE | sed '//s/\/bin\/false/\/bin\/ash/g')

                    [  "$FORM_pw2" = "admin" ] && {
                        echo "$ROOTTRUE" > /tmp/passwd_tmp
                        `cat /etc/passwd |grep -v "root:" >> /tmp/passwd_tmp` 
                        `cp /tmp/passwd_tmp /etc/passwd`
                        `rm /tmp/passwd_tmp`
                    }|| {
                        echo "$ROOTFALSE" > /tmp/passwd_tmp
                        `cat /etc/passwd |grep -v "root:" >> /tmp/passwd_tmp` 
                        `cp /tmp/passwd_tmp /etc/passwd`
                        `rm /tmp/passwd_tmp`
                    }
               } || ERROR="<pre>$RES</pre>"
       }

fi

password_form="
start_form|@TR<<Password Change>>
field|<strong>@TR<<User Name >>: $REMOTE_USER </strong>      
field|@TR<<New Password >>:
password|pw1
string|(min 5 characters)
field|@TR<<Confirm Password>>:
password|pw2
submit|passwdchange| @TR<<Change Passwd>> |
end_form"

###########################end of password change ###################

############# added user part #####################################
#get_menu=$(cat /tmp/.webcache/subcat_admin |awk -F ":" '{print $1":"$2":"$3":"}'|sed 's/ //g'|sed 's/-//g'|grep -v Logout)
#Logout_page=$(cat /tmp/.webcache/subcat_admin |awk -F ":" '{print $1":"$2":"$3":"}'|sed 's/ //g'|sed 's/-//g'|grep Logout)
generate_menulist(){
   get_menu=$(cat /tmp/.webcache/subcat_admin |sed 's/[ -]//g'|grep -v Logout|grep -v "systeminfo.sh")
   Logout_page=$(cat /tmp/.webcache/subcat_admin |sed 's/[ -]//g'|grep Logout)
   sysinfo_page=$(cat /tmp/.webcache/subcat_admin |sed 's/[ -]//g'|grep "systeminfo.sh")

   [ -z "$get_menu" ] && {
      get_menu="  Carrier:100:Status:/cgibin/webif/carrierstatus.sh
               Carrier:200:Settings:/cgibin/webif/carrier.sh
               Carrier:300:Keepalive:/cgibin/webif/carrierkeepalive.sh
               Carrier:400:TrafficWatchdog:/cgibin/webif/carriertrafficwatchdog.sh
               Carrier:500:DynamicDNS:/cgibin/webif/updatedd.sh
               Carrier:600:SMSConfig:/cgibin/webif/carriersmsconfig.sh
               Carrier:700:SMS:/cgibin/webif/carriersms.sh
               Comport:100:Status:/cgibin/webif/statuscomport.sh
               Comport:101:Com0:/cgibin/webif/comport.sh
               Comport:201:Com1:/cgibin/webif/comportcom2.sh
               Firewall:100:Status:/cgibin/webif/firewallstatus.sh
               Firewall:200:General:/cgibin/webif/firewallgeneral.sh
               Firewall:300:Rules:/cgibin/webif/firewallrules.sh
               Firewall:400:PortForwarding:/cgibin/webif/firewallportforwarding.sh
               Firewall:500:MACIPList:/cgibin/webif/firewallmaclist.sh
               GPS:100:Location:/cgibin/webif/gpslocation.sh  
               GPS:200:Settings:/cgibin/webif/gpssettings.sh
               GPS:300:Report:/cgibin/webif/gpsreport.sh
               GPS:400:GpsGate:/cgibin/webif/gpsgpsgate.sh
               GPS:500:Recorder:/cgibin/webif/gpsrecorder.sh
               GPS:600:LoadRecord:/cgibin/webif/gpsloadrecord.sh
               I/O:100:Status:/cgibin/webif/iostatus.sh
               I/O:200:OUTPUT:/cgibin/webif/io.sh
               Network:100:Status:/cgibin/webif/statusnetwork.sh
               Network:101:LAN:/cgibin/webif/networklan.sh
               Network:500:Routes:/cgibin/webif/networksroutes.sh
               Network:600:GRE:/cgibin/webif/gresummary.sh
               Network:700:SNMP:/cgibin/webif/networksnmp.sh
               Network:800:sdpServer:/cgibin/webif/networksdpserver.sh
               System:010:Settings:/cgibin/webif/systemsettings.sh
               System:160:AccessControl:/cgibin/webif/systemacl.sh
               System:200:Services:/cgibin/webif/systemservices.sh
               System:900:Maintenance:/cgibin/webif/systemupgrade.sh
               System:920:Reboot:/cgibin/webif/reboot.sh
               Tools:001:Discovery:/cgibin/webif/toolsdiscovery.sh
               Tools:100:NetflowReport:/cgibin/webif/toolsfprobe.sh
               Tools:900:SiteSurvey:/cgibin/webif/toolswlansurvey.sh
               Tools:990:Ping:/cgibin/webif/toolsping.sh
               Tools:992:TraceRoute:/cgibin/webif/toolstrace.sh
               Tools:998:NetworkTraffic:/cgibin/webif/toolsvnstat.sh
               Tools:999:EventReport:/cgibin/webif/toolseventreport.sh
               Tools:999:Modbus:/cgibin/webif/toolsmodbus.sh
               Tools:999:NMSSettings:/cgibin/webif/toolsmanagement.sh
               VPN:100:Summary:/cgibin/webif/ipsecs2ssummary.sh
               VPN:200:GatewayToGateway:/cgibin/webif/ipsecs2s.sh
               VPN:300:ClientToGateway:/cgibin/webif/ipsecx2c.sh
               VPN:350:VPNClientAccess:/cgibin/webif/ipsecvca.sh
               VPN:500:CertificateManagement:/cgibin/webif/ipseccamanagement.sh
               Wireless:150:Status:/cgibin/webif/statuswlan.sh
               Wireless:400:Radio1:/cgibin/webif/wirelesswlan0.sh "
   }

   [ -z "$Logout_page" ] && {
      Logout_page="System:910:Logout:/cgibin/webif/logout.sh"   
   }

   [ -z "$sysinfo_page" ] && {
      sysinfo_page="System:001:Summary:/cgibin/webif/systeminfo.sh"
   }

   echo "get_menu=\"${get_menu}\"" > /usr/lib/webif/aclmenu.lst
   echo "Logout_page=\"${Logout_page}\"" >> /usr/lib/webif/aclmenu.lst
   echo "sysinfo_page=\"${sysinfo_page}\"" >> /usr/lib/webif/aclmenu.lst
}

mkdir /tmp/.webif/
exists /etc/config/webif_access_control || touch /etc/config/webif_access_control

if [ ! -f /usr/lib/webif/aclmenu.lst ]; then
   generate_menulist
fi

. /usr/lib/webif/aclmenu.lst

######### remove user ###############
if ! empty "$FORM_remove_vcfg"; then
   ######## remove user from /etc/config/webif_access_control ############
   if [ "$REMOTE_USER" = "root" -o "$REMOTE_USER" = "admin" ]; then
      uci_remove "webif_access_control" "$FORM_remove_vcfg"
      a=$(grep -v $FORM_remove_vcfg /etc/httpd.conf > /tmp/.webif/file-httpd.conf) 
   else
      ERROR="${ERROR}@TR<<Do not have permission to remove user>>.<br />"
   fi
   ####### remove user from /etc/httpd.conf file
fi

####### added user ##################
if ! empty "$FORM_mhadd_user"; then
validate <<EOF
string|FORM_user_add|@TR<<Username>>|required min=5 max=32|$FORM_user_add
string|FORM_password_add|@TR<<Password>>|required min=5|$FORM_password_add
EOF
   ##### password match check ###################
   equal "$FORM_password_add" "$FORM_password2_add" || {
      [ -n "$ERROR" ] && ERROR="${ERROR}<br />"
      ERROR="${ERROR}@TR<<password_mismatch#The passwords do not match!>><br />"
   }

   ######## user name check ####################
   if [ "${FORM_user_add}" = "root" -o "${FORM_user_add}" = "admin" -o "${FORM_user_add}" = "upgrade" -o "${FORM_user_add}" = "nobody" ]; then
      [ -n "$ERROR" ] && ERROR="${ERROR}<br />"
      ERROR="${ERROR}@TR<<root, admin, upgrade and nobody are already users>>.<br />"
   fi

   ##### permission check ##################
   if [ "$REMOTE_USER" != "admin" ]; then
      [ -n "$ERROR" ] && ERROR="${ERROR}<br />"
      ERROR="${ERROR}@TR<< Only admin can add users>>.<br />"
   fi

   ####### user check in httpd.conf ###########
   [ -e /tmp/.webif/file-httpd.conf ] && HTTPD_CONFIG_FILE=/tmp/.webif/file-httpd.conf
   [ -e /etc/httpd.conf ] && HTTPD_CONFIG_FILE="$HTTPD_CONFIG_FILE /etc/httpd.conf"
   [ -n "HTTPD_CONFIG_FILE" ] && grep -q "$FORM_user_add" $HTTPD_CONFIG_FILE
   if [ "$?" = "0" ]; then
      [ -n "$ERROR" ] && ERROR="${ERROR}<br />"
      ERROR="${ERROR}$FORM_user_add @TR<<already exists.>>.<br />"
   fi

   empty "$ERROR" && {
      ##########add user in /etc/password file ###############
      deluser $FORM_user_add
      delgroup $FORM_user_add
      RES=$(
               (
	          echo "$FORM_password_add"
		  sleep 1
		  echo "$FORM_password2_add"
	       ) | adduser -s /bin/false $FORM_user_add 
	    )

      equal "$?" 0 && {
         #password=$(uhttpd -m $FORM_password_add)
         ######## add user in /etc/httpd.conf file
       	[ -e /tmp/.webif/file-httpd.conf ] || cp /etc/httpd.conf /tmp/.webif/file-httpd.conf
         echo "/cgi-bin/webif/:${FORM_user_add}:\$p\$${FORM_user_add}" >>/tmp/.webif/file-httpd.conf

         ###### added user in /etc/config/webif_access_control ########
         uci_add "webif_access_control" "accesscontrol" "${FORM_user_add}"; add_user_cfg="$CONFIG_SECTION"

         for muser1 in $get_menu; do 
            firstp=$(echo $muser1 |cut -d ":" -f1)
            secondp=$(echo $muser1 |cut -d ":" -f2)
            thirdp=$(echo $muser1 |cut -d ":" -f3)
            eval FORM_sec_field="\$FORM_${firstp}_${thirdp}"
            if [ "$FORM_sec_field" = "1" ];then
               uci_set "webif_access_control" "$add_user_cfg" ${firstp}_${secondp} "$FORM_sec_field"
            fi
         done
         ### add logout page as a default page
         LOfirst=$(echo $Logout_page |cut -d ":" -f1)
         LOsecond=$(echo $Logout_page |cut -d ":" -f2)
         LOthird=$(echo $Logout_page |cut -d ":" -f3)
         uci_set "webif_access_control" "$add_user_cfg" ${LOfirst}_${LOsecond} 1
         ### add system info page as a default page
         SIfirst=$(echo $sysinfo_page |cut -d ":" -f1)
         SIsecond=$(echo $sysinfo_page |cut -d ":" -f2)
         SIthird=$(echo $sysinfo_page |cut -d ":" -f3)
         uci_set "webif_access_control" "$add_user_cfg" ${SIfirst}_${SIsecond} 1

         unset FORM_user_add  FORM_sec_field FORM_mhadd_user FORM_submit
      }|| ERROR="<pre>$RES</pre>"
   }
fi

########## add user part form ######################################
user_add_form=""
user1_add_form="start_form|@TR<<Add User:  ( Note: Changes will not take effect until the system is rebooted )>>
               field|@TR<<Username>> :
               text|user_add|$FORM_user_add
               string| (5-32 characters)
               field|@TR<<Password >>
               password|password_add
               string|(min 5 characters)
               field|@TR<<Confirm Password>>
               password|password2_add"

append user_add_form "$user1_add_form" "$N"
category=""
js=""
for muser1 in $get_menu; do
   firstp=$(echo $muser1 |cut -d ":" -f1)
   secondp=$(echo $muser1 |cut -d ":" -f2)
   thirdp=$(echo $muser1 |cut -d ":" -f3)
   [ -z "$FORM_enable_field" ] && FORM_enable_field=0
   if [ "$firstp" != "$category" ]; then
      eval FORM_enable_field="\$FORM_enable_$firstp"
      user1_add_form="field|$firstp
                   select|${firstp}_enable|$FORM_enable_field
                   option|0|@TR<<Hide Submuenu>>
                   option|1|@TR<<Show Submuenu>>"
      append user_add_form "$user1_add_form" "$N"
   fi
   category="$firstp"
   #eval FORM_sec_field="\$FORM_${firstp}_${thirdp}"
   user1_add_form="field|&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;$thirdp|${firstp}_${thirdp}_field|hidden
                   select|${firstp}_${thirdp}|0
                   option|0|@TR<<Disable>>
                   option|1|@TR<<Enable>>"
   append user_add_form "$user1_add_form" "$N"


   javascript_forms="v = isset('${firstp}_enable','1');
   		     set_visible('${firstp}_${thirdp}_field', v);"
   append js "$javascript_forms" "$N"
done
   user1_add_form="field|Add User
                  submit|mhadd_user|@TR<<Add User>>
                  end_form"
append user_add_form "$user1_add_form" "$N"

################ user list part #####################
acl_cfgs=""
user_lists=""
uci_load webif_access_control
get_submenu_name(){
   firstp="$1":"$2"
   thirdp=$(echo "$get_menu" | grep $firstp |cut -d ":" -f3)
   logger "fistp is $firstp and thirdp is $thirdp"
   echo $thirdp
}

user_list="start_form|@TR<<Users Summary>>"
append user_lists "$user_list" "$N"
if [ -n "$acl_cfgs" ]; then
   for usr1 in $acl_cfgs; do
      user_list="field|Username 
                 string|$usr1"
      append user_lists "$user_list" "$N"

      logoutnum=$(echo "$Logout_page" |cut -d ":" -f2 )
      systeminfonum=$(echo "$sysinfo_page" |cut -d ":" -f2 )
      showlists=$(uci show webif_access_control.$usr1 | grep -v accesscontrol |grep -v "$logoutnum" |grep -v "$systeminfonum")
      for item in $showlists; do
         mainmenu=$(echo $item | cut -d "." -f3 | cut -d "_" -f1 )
         subnum=$(echo $item |cut -d "." -f3 |cut -d "_" -f2 |cut -d "=" -f1 )
         submenu_name=$(get_submenu_name $mainmenu $subnum)
         user_list="field|&nbsp;
                    string|$mainmenu&nbsp;&nbsp;$submenu_name&nbsp;&nbsp;Enabled"
         append user_lists "$user_list" "$N"
      done

      user_list="field|&nbsp;&nbsp
                  string|<a href=\"$SCRIPT_NAME?remove_vcfg=$usr1\">@TR<<Remove User>></a>"
      append user_lists "$user_list" "$N"
   done
else
   user_list="string|</tr>
              string|No users defined."
   append user_lists "$user_list" "$N"
fi
append user_lists "end_form" "$N"


header "System" "Access Control" "@TR<<Access Control>>" 'onload="modechange()"' "$SCRIPT_NAME"
#####################################################################
# modechange script
#
cat <<EOF
<script type="text/javascript" src="/webif.js"></script>
<script type="text/javascript">
<!--
function modechange()
{
	var v;
	$js

	hide('save');
	show('save');
}
-->
</script>
EOF

display_form <<EOF
onchange|modechange
$password_form
$user_add_form
$user_lists
EOF

footer

?>

<!--
##WEBIF:name:System:160:Access Control
-->
