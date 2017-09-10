#!/usr/bin/webif-page "-U /tmp -u 16384"
<?
. /usr/lib/webif/webif.sh
. /etc/version

COPY_FILES="/etc/firewall.config
/etc/firewall.user
/etc/httpd.conf
/etc/hosts
/etc/ethers
/etc/passwd"

COPY_DIRS="/etc/config
/etc/openvpn
/etc/crontabs
/etc/ssl
/etc/dropbear"

if [ "$HARDWARE" = "5.0.0" ]; then
	productname="IPMESH"
elif [ "$HARDWARE" = "3.0.0" ]; then
	productname="SVIP"
elif [ "$HARDWARE" = "v1.0.0" ]; then
	productname="NANOPCI"
        if [ "$PRODUCT" = "IPnVT" ];then
            productname="IPnVT"
        fi
else
	productname="VIP"
fi
[ "$PRODUCT" = "VIP421EXP49G" ] && PRODUCT="VIP421NOST"

board_info="${VENDOR}:${productname}-${PRODUCT}:${HARDWARE}"

. /usr/lib/webif/webif.sh
uci_load "system"

board_type=$(cat /proc/cpuinfo 2>/dev/null | sed 2,20d | cut -c16-)
machinfo=$(uname -a 2>/dev/null)
if $(echo "$machinfo" | grep -q "mips"); then
	if $(echo "$board_type" | grep -q "Atheros"); then
		target="atheros-2.6"
		full_flash="0"
	elif $(echo "$board_type" | grep -q "WP54"); then
		target="adm5120-2.6"
		full_flash="0"
	elif $(echo "$machinfo" | grep -q "2\.4"); then
		target="brcm"
		full_flash="1"
		mtd_path="/dev/mtdblock/1"
	elif $(echo "$machinfo" | grep -q "2\.6"); then
		target="brcm"
		full_flash="1"
		mtd_path="/dev/mtdblock1"
	fi
elif $(echo "$machinfo" | grep -q " i[0-9]86 "); then
	target="x86-2.6"
	full_flash="0"
elif $(echo "$machinfo" | grep -q " avr32 "); then
	target="avr32-2.6"
	full_flash="0"
elif $(cat /proc/cpuinfo 2>/dev/null | grep -q "IXP4"); then
	target="ixp4xx-2.6"
	full_flash="0"
fi

DOWNLOAD()
{
cat <<EOF
&nbsp;&nbsp;&nbsp;@TR<<confman_noauto_click#If downloading does not start automatically, click here>> ... <a href="/$1">$1</a><br /><br />
<script language="JavaScript" type="text/javascript">
setTimeout('top.location.href=\"/$1\"',"5000")
</script>
EOF
}

revision_text=" $SOFTWARE build $BUILD"
Built_date=$(cat /etc/banner|grep "Built time"|awk '{print $3}')
Built_time=$(cat /etc/banner|grep "Built time"|awk '{print $4}')

header "System" "Maintenance" "@TR<<System Maintenance>>" '' "$SCRIPT_NAME"


if [ "$target" = "x86-2.6" -o "$target" = "brcm" -o "$target" = "adm5120-2.6"  -o "$target" = "atheros-2.6" -o "$target" = "ixp4xx-2.6" ]; then

[ "$PRODUCT" = "VIP421ENCOM" ] && { 
cat <<EOF
<div class="settings">
<table style="width: 100%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3><strong>Version Information</strong></h3></th>
</table>

<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<th><strong>Product Name</th>
	<th><strong>Hardware Type</th>
	<th><strong>Build Version</th>
	<th><strong>Build Date</th>
	<th><strong>Build Time</th>
</tr>

<tr class="odd">
	<td>$productname-$PRODUCT</td>
	<td>$HARDWARE</td>
	<td>$revision_text</td>
	<td>$Built_date</td>
	<td>$Built_time</td>
</tr>
</table></div>
<br/>
EOF
} || {
if [ "$PRODUCT" = "IPnVT" ];then
    _firmware_name="IPnVT 1.00"

cat <<EOF
<div class="settings">
<table style="width: 100%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3><strong>Version Information</strong></h3></th>
</table>

<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<th><strong>Product Name</th>
	<th><strong>Firmware Version</th>
	<th><strong>Hardware Type</th>
	<th><strong>Build Version</th>
	<th><strong>Build Date</th>
	<th><strong>Build Time</th>
</tr>

<tr class="odd">
	<td>$productname</td>
	<td>$_firmware_name</td>
	<td>$HARDWARE</td>
	<td>$revision_text</td>
	<td>$Built_date</td>
	<td>$Built_time</td>
</tr>
</table></div>
<br/>
EOF
else
cat <<EOF
<div class="settings">
<table style="width: 100%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3><strong>Version Information</strong></h3></th>
</table>

<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<th><strong>Product Name</th>
	<th><strong>Firmware Version</th>
	<th><strong>Hardware Type</th>
	<th><strong>Build Version</th>
	<th><strong>Build Date</th>
	<th><strong>Build Time</th>
</tr>

<tr class="odd">
	<td>$productname-$PRODUCT</td>
	<td>$_firmware_name</td>
	<td>$HARDWARE</td>
	<td>$revision_text</td>
	<td>$Built_date</td>
	<td>$Built_time</td>
</tr>
</table></div>
<br/>
EOF
fi
}

	if empty "$FORM_upgrade"; then
display_form <<EOF
start_form|@TR<<Firmware Upgrade>>
field|@TR<<Erase Current Configuration>>
checkbox|nokeepconfig|$FORM_nokeepconfig|1
field|@TR<<Firmware Image>>
upload|upgradefile|$FORM_upgradefile_name
submit|upgrade| @TR<<Upgrade Firmware>> |
end_form
EOF
	else
                if empty "$FORM_upgradefile_name"; then
display_form <<EOF
start_form|@TR<<Firmware Upgrade>>
EOF
echo "<tr><td colspan=\"2\"><font color=\"red\">@TR<<big_warning#WARNING>></font>: @TR<<Upgrade Filename cannot be empty>>!</td></tr>"
display_form<<EOF
end_form
EOF
                else
display_form <<EOF
start_form|@TR<<Firmware Upgrade>>
EOF
		echo "<br />Upgrading firmware, please wait ... <br />"
		if [ "$FORM_nokeepconfig" = "1" ]; then
			sysupgrade -b -n $FORM_upgradefile
                        DEFIP="192.168.168.1"
                        echo "<br />@TR<<Rebooting now>>...<meta http-equiv=\"refresh\" content=\"4;url=reboot.sh?reboot=1&amp;defaultip=$DEFIP\">"
		else
			sysupgrade -b $FORM_upgradefile
		        echo "<br />@TR<<Rebooting now>>...<meta http-equiv=\"refresh\" content=\"4;url=reboot.sh?reboot=1\">"
		fi
		#echo "@TR<<done>>."
display_form<<EOF
end_form
EOF
                fi
	fi
else
	echo "<br />The ability to upgrade your platform has not been implemented.<br />"
fi

if ! equal $FORM_download "" ; then
	
	if equal $FORM_rdflash "1" ; then

		tmp=/tmp/flash_$FORM_name.trx
		tgz=/www/flash_$FORM_name.trx
		dd if=$mtd_path > $tmp 2>/dev/null
		ln -s $tmp $tgz 2>/dev/null
		DOWNLOAD flash_$FORM_name.trx
		sleep 25 ; rm $tmp ; rm $tgz
	else

		tmp=/tmp/config.$$
		tgz=/www/${FORM_name}.config
		rm -rf $tmp 2>/dev/null
		mkdir -p $tmp 2>/dev/null
		date > $tmp/config.date
		echo "$FORM_name" > $tmp/config.name
                echo "$board_info" >$tmp/config.boardinfo

	        echo $(dmesg | grep "CPU revision is:" | sed -e s/'CPU revision is: '//g) > $tmp/config.boardtype

		for file in $COPY_FILES; do
			[ -e $file ] && [ ! -h $file ] && {
			d=`dirname $file`; [ -d $tmp$d ] || mkdir -p $tmp$d
			cp -af $file $tmp$file 2>/dev/null
			}
		done
		for dir in $COPY_DIRS; do
			[ -e $dir ] && {
			mkdir -p $tmp$dir
			cp -afr $dir/* $tmp$dir/ 2>/dev/null
			}
		done
		[ -n "$tmp" ] && rm $tmp/etc/banner
		(cd $tmp; tar czf $tgz *)
		rm -rf $tmp 2>/dev/null
                DOWNLOAD ${FORM_name}.config
		sleep 25 
                #rm $tgz
	fi

elif ! equal $FORM_instconfig "" ; then

if equal $FORM_rdflash "1" ; then
	echo "<br />@TR<<confman_Restoring_firmware#Restoring firmware, please wait ...>> <br />"
	mtd -r write $FORM_file linux
else

dir=$FORM_dir
display_form <<EOF
start_form|@TR<<Restore Configuration>>
EOF
	if [ -n "$dir" ] && [ -d "$dir" ] && [ -e "$dir/config.name" ] && [ -e "$dir/config.boardtype" ] && [ -e "$dir/config.boardinfo" ]; then
			echo "<tr><td colspan=\"2\">@TR<<confman_restoring_conf#Restoring configuration.>><br /><pre>"
			cd $dir
			for file in $(find etc); do
				if [ -d $file ]; then
					[ -d /$file ] || mkdir /$file
				else
					[ -e /$file ] && rm /$file
					cp -af $file /$file
					#echo "@TR<<confman_restoring_file#restoring>> $file"
				fi
			done
                        #replace mac address in wireless config file
        		for phy in $(ls /sys/class/ieee80211 2>/dev/null); do
			    localmac=$(cat /sys/class/ieee80211/${phy}/macaddress)
                            devidx=$(echo $phy|cut -c 4) 
                            echo "local_macaddr is $localmac devidx is $devidx" >/tmp/aaa
                            uci set wireless.radio${devidx}.macaddr=$localmac 
        		done
                        sleep 1
                        uci commit wireless
                        sleep 1

		echo "<br />@TR<<Rebooting now>>...<meta http-equiv=\"refresh\" content=\"4;url=reboot.sh?reboot=1\">"
		echo "</pre></td></tr>"
	else
		echo "<p>@TR<<confman_bad_dir#bad dir>>: $dir</p>"
	fi

display_form <<EOF
end_form
EOF
fi

elif ! equal $FORM_reset "" ; then
display_form <<EOF
start_form|@TR<<Restore Default Configuration>>
EOF
   echo "<tr><td colspan=\"2\">@TR<<confman_restoring_conf#Restoring configuration.>><br /><pre>"
   for file in $COPY_FILES; do
      rm $file 2>/dev/null
      [ -e /rom$file ] && [ ! -h /rom$file ] && {
      d=`dirname $file`; [ -d $d ] || mkdir -p $d
      cp -af /rom$file $file 2>/dev/null
      }
   done

   for dir in $COPY_DIRS; do
      [ -e $dir ] && {
	 rm $dir/* 2>/dev/null
	 cp -afr /rom$dir/* $dir/ 2>/dev/null
      }
   done

   DEFIP="192.168.168.1"
   echo "<br />@TR<<Rebooting now>>...<meta http-equiv=\"refresh\" content=\"4;url=reboot.sh?reboot=1&amp;defaultip=$DEFIP\">"
   echo "</pre></td></tr>"

display_form <<EOF
end_form
EOF

elif ! equal $FORM_chkconfig "" ; then

		if [ -n "$FORM_configfile" ] && [ -e "$FORM_configfile" ]; then
			
		echo "<form method=\"get\" name=\"install\" action=\"$SCRIPT_NAME\">"
display_form <<EOF
start_form|@TR<<Restore Configuration>>
EOF
if equal $FORM_rdflash "1" ; then
	echo "<h2><font color=red>@TR<<confman_warn_erase#WARNING !!! This operation will erase current flash.>></font></h2>"
	echo "<input type='hidden' name='rdflash' value='1' /><input type='hidden' name='file' value=\"$FORM_configfile\" />"	
else
			rm -rf /tmp/config.* 2>/dev/null
			tmp=/tmp/config.$$
			mkdir $tmp
			(cd $tmp; tar xzf $FORM_configfile)
			rm $FORM_configfile

			if [ ! -e "$tmp/config.name" ] || [ ! -e "$tmp/config.boardtype" ]; then
                                echo "<tr><td colspan=\"2\"><font color=\"red\">@TR<<big_warning#WARNING>></font>: @TR<<Invalid File >>!</td></tr>"
			else
				nm=$(cat $tmp/config.name)
				bd=$(cat $tmp/config.boardtype)
				dt=$(cat $tmp/config.date)
                                bi=$(cat $tmp/config.boardinfo)    
        			CFGGOOD="<tr><td colspan=\"2\">@TR<<confman_good_conf#The configuration looks good>>!<br /><br /></td></tr>"
                                echo "product is $PRODUCT file product is $(echo $bi |awk -F ":" '{print $2}')" >/tmp/aaaaa
                                if [ "$VENDOR" != "$(echo $bi |awk -F ":" '{print $1}')" ]; then
        			    echo "<tr><td colspan=\"2\"><font color=\"red\">@TR<<big_warning#WARNING>></font>: @TR<<Different Vendor ID. >>!</td></tr>"
                                elif [ "${productname}-${PRODUCT}" != "$(echo $bi |awk -F ":" '{print $2}')" ]; then
				    echo "<tr><td colspan=\"2\"><font color=\"red\">@TR<<big_warning#WARNING>></font>: @TR<<Different Product>></td></tr>"
                                elif [ "$HARDWARE" != "$(echo $bi |awk -F ":" '{print $3}')" ]; then
				    echo "<tr><td colspan=\"2\"><font color=\"red\">@TR<<big_warning#WARNING>></font>: @TR<<Different Hardware Type>></td></tr>"
			        elif [ "$bd" != $(dmesg | grep "CPU revision is:" | sed -e s/'CPU revision is: '//g) ]; then
				    echo "<tr><td colspan=\"2\"><font color=\"red\">@TR<<big_warning#WARNING>></font>: @TR<<confman_other_board#different board type>> (@TR<<confman_board_ours#ours>>: $(dmesg | grep "CPU revision is:" | sed -e s/'CPU revision is: '//g), @TR<<confman_board_file#file>>: $bd)!</td></tr>"
			        else
				    echo $CFGGOOD

display_form <<EOF
field|@TR<<Config file Name>>
string|$nm
field|@TR<<Generated>>
string|$dt
field|@TR<<Vendor>>
string|$(echo $bi |awk -F ":" '{print $1}')
field|@TR<<Product>>
string|$(echo $bi |awk -F ":" '{print $2}')
field|@TR<<Hardware Type>>
string|$(echo $bi |awk -F ":" '{print $3}')
field
EOF
echo "</td></tr>"

cat <<EOF
<tr><td>&nbsp;</td></tr>
<tr><td><input type='hidden' name='dir' value="$tmp" />
<input type="submit" class="buttons" name="instconfig" value="@TR<<Restore>>" /></td></tr>
<tr><td>&nbsp;</td></tr>
EOF

			        fi


			fi
fi


display_form <<EOF
end_form
EOF
		echo "</form>"
		fi
footer
exit
fi

if [ "$full_flash" = "1" ]; then
cat <<EOF
<form method="post" name="download" action="$SCRIPT_NAME" enctype="multipart/form-data">
&nbsp;&nbsp;&nbsp;<input type="radio" name="rdflash" value="0" checked="checked" />&nbsp;@TR<<confman_Configuration#Configuration>>
<br/>
&nbsp;&nbsp;&nbsp;<input type="radio" name="rdflash" value="1" />&nbsp;@TR<<confman_Entire_Flash#Entire Flash>><br/><br/>
EOF
else
cat <<EOF
<form method="post" name="download" action="$SCRIPT_NAME" enctype="multipart/form-data">
<br/>
EOF
fi

display_form <<EOF
start_form|@TR<<Reset to Default>>
EOF

cat <<EOF
<tr><td width="68%">@TR<<Reset to Default Configuration>>&nbsp;&nbsp;&nbsp;</td>
<td><input class="buttons" type="submit" name="reset" value="@TR<< Reset  to   Default >>" /></td> </tr> 
EOF

display_form <<EOF
end_form
EOF

display_form <<EOF
start_form|@TR<<Backup Configuration>>
EOF

cat <<EOF
<tr><td width="40%">@TR<<Name this configuration>></td>
<td width="28%"><input name="name" value="${FORM_name:-$PRODUCT}" /></td>
<td><input class="buttons" type="submit" name="download" value="@TR<<Backup Configuration>>" /></td>
</tr>
EOF

display_form <<EOF
end_form
EOF

display_form <<EOF
start_form|@TR<<Restore Configuration>>
EOF

cat<<EOF
<tr>
<td width="40%">@TR<<Restore Configuration file>></td>
<td width="28%"><input type="file" class="buttons" name="configfile" /></td>
<td><input class="buttons" type="submit" name="chkconfig" value="@TR<<Check  Restore  File >>" /></td>
</tr>
EOF

display_form <<EOF
end_form|
string|</form>
EOF

footer
?>
<!--
##WEBIF:name:System:900:Maintenance
-->
