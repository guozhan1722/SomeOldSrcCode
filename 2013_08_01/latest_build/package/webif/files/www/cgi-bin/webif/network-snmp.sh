#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
. /etc/version
###################################################################
# SNMP daemon configuration page
#
# Kamikaze version
# 29/04/2007 Liran Tal <liran@enginx.com>
#
# Description:
#	Configures SNMP daemon.
#
# Author(s) [in order of work date]:
#	Liran Tal <liran at enginx.com>
#	Lubos Stanek <lubek at users.berlios.de>
#
# NVRAM variables referenced:
#	snmp_private_name
#	snmp_private_src
#	snmp_public_name
#	snmp_public_src
#
# Configuration files referenced:
#	none
#

DOWNLOAD()
{
cat <<EOF
&nbsp;&nbsp;&nbsp;@TR<<confman_noauto_click#If downloading does not start automatically, click here>> ... <a href="/$1">$1</a><br /><br />
<script language="JavaScript" type="text/javascript">
setTimeout('top.location.href=\"/$1\"',"5000")
</script>
EOF
}

uci_load "snmpd"
uci_load "firewall"

#if ! empty "$FORM_remove_snmpd"; then
#	echo "@TR<<Removing SNMPd package>> ...<pre>"
#	/etc/init.d/S??snmpd stop 2> /dev/null
#	/etc/init.d/snmpd stop 2> /dev/null
#	rm -f "/etc/init.d/S??snmpd" 2> /dev/null
#	remove_package snmpd
#	remove_package libnetsnmp 2>/dev/null
#	remove_package libelf 2>/dev/null
#	echo "</pre>"
#fi

local cfg="snmpd"
if empty "$FORM_submit"; then
       config_get FORM_NetWork_SNMP_MODE $cfg NetWork_SNMP_MODE
       config_get FORM_NetWork_SNMP_Read_Community_Name $cfg NetWork_SNMP_Read_Community_Name
       config_get FORM_NetWork_SNMP_Write_Community_Name $cfg NetWork_SNMP_Write_Community_Name
       config_get FORM_NetWork_SNMP_V3_User_Name $cfg NetWork_SNMP_V3_User_Name
       config_get FORM_NetWork_SNMP_V3_User_ReadWrite_Limit $cfg NetWork_SNMP_V3_User_ReadWrite_Limit
       config_get FORM_NetWork_SNMP_V3_User_Auth_Level $cfg NetWork_SNMP_V3_User_Auth_Level
       config_get FORM_NetWork_SNMP_V3_Auth_Password $cfg NetWork_SNMP_V3_Auth_Password
       config_get FORM_NetWork_SNMP_V3_Privacy_Password $cfg NetWork_SNMP_V3_Privacy_Password
       config_get FORM_NetWork_SNMP_Trap_Version $cfg NetWork_SNMP_Trap_Version
       config_get FORM_NetWork_SNMP_Auth_Traps_Status $cfg NetWork_SNMP_Auth_Traps_Status
       config_get FORM_NetWork_SNMP_Trap_Community_Name $cfg NetWork_SNMP_Trap_Community_Name
       config_get FORM_NetWork_SNMP_Trap_Manage_Host $cfg NetWork_SNMP_Trap_Manage_Host
       config_get FORM_NetWork_SNMP_Listen_Protocol $cfg NetWork_SNMP_Listen_Protocol
       config_get FORM_NetWork_SNMP_Listen_Port $cfg NetWork_SNMP_Listen_Port
else
       eval FORM_NetWork_SNMP_MODE="\$FORM_${cfg}_NetWork_SNMP_MODE"
       eval FORM_NetWork_SNMP_Read_Community_Name="\$FORM_${cfg}_NetWork_SNMP_Read_Community_Name"
       eval FORM_NetWork_SNMP_Write_Community_Name="\$FORM_${cfg}_NetWork_SNMP_Write_Community_Name"
       eval FORM_NetWork_SNMP_V3_User_Name="\$FORM_${cfg}_NetWork_SNMP_V3_User_Name"
       eval FORM_NetWork_SNMP_V3_User_ReadWrite_Limit="\$FORM_${cfg}_NetWork_SNMP_V3_User_ReadWrite_Limit"
       eval FORM_NetWork_SNMP_V3_User_Auth_Level="\$FORM_${cfg}_NetWork_SNMP_V3_User_Auth_Level"
       eval FORM_NetWork_SNMP_V3_Auth_Password="\$FORM_${cfg}_NetWork_SNMP_V3_Auth_Password"
       eval FORM_NetWork_SNMP_V3_Privacy_Password="\$FORM_${cfg}_NetWork_SNMP_V3_Privacy_Password"
       eval FORM_NetWork_SNMP_Trap_Version="\$FORM_${cfg}_NetWork_SNMP_Trap_Version"
       eval FORM_NetWork_SNMP_Auth_Traps_Status="\$FORM_${cfg}_NetWork_SNMP_Auth_Traps_Status"
       eval FORM_NetWork_SNMP_Trap_Community_Name="\$FORM_${cfg}_NetWork_SNMP_Trap_Community_Name"
       eval FORM_NetWork_SNMP_Trap_Manage_Host="\$FORM_${cfg}_NetWork_SNMP_Trap_Manage_Host"
       eval FORM_NetWork_SNMP_Listen_Protocol="\$FORM_${cfg}_NetWork_SNMP_Listen_Protocol"
       eval FORM_NetWork_SNMP_Listen_Port="\$FORM_${cfg}_NetWork_SNMP_Listen_Port"

	validate <<EOF
string|FORM_NetWork_SNMP_Read_Community_Name|@TR<<Read Only Community Name>>||$FORM_NetWork_SNMP_Read_Community_Name
string|FORM_NetWork_SNMP_Write_Community_Name|@TR<<Read Write Community Name>>||$FORM_NetWork_SNMP_Write_Community_Name
string|FORM_NetWork_SNMP_V3_User_Name|@TR<<SNMP V3 User Name>>||$FORM_NetWork_SNMP_V3_User_Name
string|FORM_NetWork_SNMP_V3_Auth_Password|@TR<<V3 Authentication Password>>|required min=8|$FORM_NetWork_SNMP_V3_Auth_Password
string|FORM_NetWork_SNMP_V3_Privacy_Password|@TR<<V3 Privacy Password>>|required min=8|$FORM_NetWork_SNMP_V3_Privacy_Password
string|FORM_NetWork_SNMP_Trap_Community_Name|@TR<<Trap Community Name>>||$FORM_NetWork_SNMP_Trap_Community_Name
ip|FORM_NetWork_SNMP_Trap_Manage_Host|@TR<<Trap Manage Host IP>>||$FORM_NetWork_SNMP_Trap_Manage_Host
int|FORM_NetWork_SNMP_Listen_Port|@TR<<SNMP Listening Port>>||$FORM_NetWork_SNMP_Listen_Port
EOF
	equal "$?" 0 && {
         uci_set snmpd "$cfg" "NetWork_SNMP_MODE" "$FORM_NetWork_SNMP_MODE"
         uci_set snmpd "$cfg" "NetWork_SNMP_Read_Community_Name" "$FORM_NetWork_SNMP_Read_Community_Name"
         uci_set snmpd "$cfg" "NetWork_SNMP_Write_Community_Name" "$FORM_NetWork_SNMP_Write_Community_Name"
         uci_set snmpd "$cfg" "NetWork_SNMP_V3_User_Name" "$FORM_NetWork_SNMP_V3_User_Name"
         uci_set snmpd "$cfg" "NetWork_SNMP_V3_User_ReadWrite_Limit" "$FORM_NetWork_SNMP_V3_User_ReadWrite_Limit"
         uci_set snmpd "$cfg" "NetWork_SNMP_V3_User_Auth_Level" "$FORM_NetWork_SNMP_V3_User_Auth_Level"
         uci_set snmpd "$cfg" "NetWork_SNMP_V3_Auth_Password" "$FORM_NetWork_SNMP_V3_Auth_Password"
         uci_set snmpd "$cfg" "NetWork_SNMP_V3_Privacy_Password" "$FORM_NetWork_SNMP_V3_Privacy_Password"
         uci_set snmpd "$cfg" "NetWork_SNMP_Trap_Version" "$FORM_NetWork_SNMP_Trap_Version"
         uci_set snmpd "$cfg" "NetWork_SNMP_Auth_Traps_Status" "$FORM_NetWork_SNMP_Auth_Traps_Status"
         uci_set snmpd "$cfg" "NetWork_SNMP_Trap_Community_Name" "$FORM_NetWork_SNMP_Trap_Community_Name"
         uci_set snmpd "$cfg" "NetWork_SNMP_Trap_Manage_Host" "$FORM_NetWork_SNMP_Trap_Manage_Host"
         uci_set snmpd "$cfg" "NetWork_SNMP_Listen_Protocol" "$FORM_NetWork_SNMP_Listen_Protocol"
         uci_set snmpd "$cfg" "NetWork_SNMP_Listen_Port" "$FORM_NetWork_SNMP_Listen_Port"
	}
fi

snmpd_options="start_form|@TR<<SNMP Settings>>

field|@TR<<SNMP Operation Mode>>
radio|${cfg}_NetWork_SNMP_MODE|$FORM_NetWork_SNMP_MODE|A|@TR<<Disable>>
radio|${cfg}_NetWork_SNMP_MODE|$FORM_NetWork_SNMP_MODE|B|@TR<<V1&V2c&V3>>

field|@TR<<Read Only Community Name>>
text|${cfg}_NetWork_SNMP_Read_Community_Name|$FORM_NetWork_SNMP_Read_Community_Name
#helpitem|SNMP Community Name
#helptext|Helptext SNMP Community Name#The SNMP community name identifies a group of devices and management systems that share authentication, access control of this group. Although PUBLIC and PRIVATE are commonly used, it is strongly suggested to use hard to guess names. The only worse thing than PUBLIC and PRIVATE, is to leave the community name blank! The community name can be considered a group password.

field|@TR<<Read Write Community Name>>
text|${cfg}_NetWork_SNMP_Write_Community_Name|$FORM_NetWork_SNMP_Write_Community_Name

field|@TR<<SNMP V3 User Name>>
text|${cfg}_NetWork_SNMP_V3_User_Name|$FORM_NetWork_SNMP_V3_User_Name

field|@TR<<V3 User Read Write Limit>>
radio|${cfg}_NetWork_SNMP_V3_User_ReadWrite_Limit|$FORM_NetWork_SNMP_V3_User_ReadWrite_Limit|A|@TR<<Read Only>>
radio|${cfg}_NetWork_SNMP_V3_User_ReadWrite_Limit|$FORM_NetWork_SNMP_V3_User_ReadWrite_Limit|B|@TR<<Read Write>>

field|@TR<<V3 User Authentication Level>>
select|${cfg}_NetWork_SNMP_V3_User_Auth_Level|$FORM_NetWork_SNMP_V3_User_Auth_Level
option|A|@TR<<NoAuthNoPriv>>
option|B|@TR<<AuthNoPriv>>
option|C|@TR<<AuthPriv>>

field|@TR<<V3 Authentication Password>>
text|${cfg}_NetWork_SNMP_V3_Auth_Password|$FORM_NetWork_SNMP_V3_Auth_Password

field|@TR<<V3 Privacy Password>>
text|${cfg}_NetWork_SNMP_V3_Privacy_Password|$FORM_NetWork_SNMP_V3_Privacy_Password

field|@TR<<SNMP Trap Version>>
select|${cfg}_NetWork_SNMP_Trap_Version|$FORM_NetWork_SNMP_Trap_Version
option|A|@TR<<V1 Traps>>
option|B|@TR<<V2 Traps>>
option|C|@TR<<V3 Traps>>
option|D|@TR<<V1&V2 Traps>>
option|E|@TR<<V1&V2&V3 Traps>>
              
field|@TR<<Auth Failure Traps>>
radio|${cfg}_NetWork_SNMP_Auth_Traps_Status|$FORM_NetWork_SNMP_Auth_Traps_Status|A|@TR<<Disable>>
radio|${cfg}_NetWork_SNMP_Auth_Traps_Status|$FORM_NetWork_SNMP_Auth_Traps_Status|B|@TR<<Enable>>

field|@TR<<Trap Community Name>>
text|${cfg}_NetWork_SNMP_Trap_Community_Name|$FORM_NetWork_SNMP_Trap_Community_Name

field|@TR<<Trap Manage Host IP>>
text|${cfg}_NetWork_SNMP_Trap_Manage_Host|$FORM_NetWork_SNMP_Trap_Manage_Host

field|@TR<<SNMP Listening Protocol>>
radio|${cfg}_NetWork_SNMP_Listen_Protocol|$FORM_NetWork_SNMP_Listen_Protocol|UDP|@TR<<UDP>>
radio|${cfg}_NetWork_SNMP_Listen_Protocol|$FORM_NetWork_SNMP_Listen_Protocol|TCP|@TR<<TCP>>

field|@TR<<SNMP Listening Port>>
text|${cfg}_NetWork_SNMP_Listen_Port|$FORM_NetWork_SNMP_Listen_Port

end_form"

append forms "$snmpd_options" "$N"

#	primary_snmpd_form="field|@TR<<SNMP Public Community Name>>|snmp_public_name
#	text|snmp_public_name|$FORM_snmp_public_name
#	helpitem|SNMP Community Name
#	helptext|Helptext SNMP Community Name#The SNMP community name identifies a group of devices and management systems that share authentication, access control of this group. Although PUBLIC and PRIVATE are commonly used, it is strongly suggested to use hard to guess names. The only worse thing than PUBLIC and PRIVATE, is to leave the community name blank! The community name can be considered a group password.
#	field|@TR<<SNMP Public Source>>|snmp_public_src
#	text|snmp_public_src|$FORM_snmp_public_src
#	helpitem|SNMP Source
#	helptext|Helptext SNMP Source#SNMP source defines the IP address, hostname or network mask for management systems that can read information from this 'public' community device or control this 'private' comunity device.
#	field|@TR<<SNMP Private Community Name>>|snmp_private_name
#	text|snmp_private_name|$FORM_snmp_private_name
#	field|@TR<<SNMP Private Source>>|snmp_private_src
#	text|snmp_private_src|$FORM_snmp_private_src
#	$remove_snmpd_button"


header "Network" "SNMP" "@TR<<SNMP Settings>>" '' "$SCRIPT_NAME"

if ! equal $FORM_download "" ; then

        echo "<br />Downloading MIB File, please wait ... <br />"

        if [ "$PRODUCT" = "VIP4G" ]; then
           /bin/mibparse /lib/snmp_item /www/Microhard-VIP4G.MIB
           DOWNLOAD Microhard-VIP4G.MIB
        elif [ "$PRODUCT" = "IPn4G" ]; then
           /bin/mibparse /lib/snmp_item /www/Microhard-IPn4G.MIB
           DOWNLOAD Microhard-IPn4G.MIB
        fi
        footer_nosubmit
        sleep 25 
        #rm $tgz
else
display_form <<EOF
$forms
EOF
cat <<EOF
<form method="post" name="download" action="$SCRIPT_NAME" enctype="multipart/form-data">
EOF

display_form <<EOF
start_form|@TR<<Download MIB File>>
EOF
cat <<EOF
<td><input class="buttons" type="submit" name="download" value="@TR<<Get MIB File>>" /></td>
EOF

display_form <<EOF
end_form
EOF

footer 
fi

?>
<!--
##WEBIF:name:Network:700:SNMP
-->
