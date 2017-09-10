#!/usr/bin/webif-page "-U /tmp -u 16384"
<?
. /usr/lib/webif/webif.sh
. /etc/version

COPY_FILES="/etc/firewall.config
/etc/firewall.user
/etc/httpd.conf
/etc/hosts
/etc/ethers
/etc/passwd
/etc/dhcp_ippbackup
/usr/lib/hardware_desc"

COPY_DIRS="/etc/config
/etc/openvpn
/etc/crontabs
/etc/ssl
/etc/dropbear"

board_info="${VENDOR}:${PRODUCTNAME}-${PRODUCT}:${HARDWARE}"

EXTRACT="/bin/extractmylofw"

uci_load "system"

header "System" "Maintenance" "@TR<<System Maintenance>>" '' "$SCRIPT_NAME"

disable_reboot() {
        #stop keepalive when start upgrade firmware
        #echo 'Stop keepalive for upgrading system'
        keepalivebootup_pid=$(ps |grep keepalivebootup |grep -v grep |awk '{print $1}')
        if [ "$keepalivebootup_pid" = "" ] ;then
	    killall -9 S${START}keepalive 2>&- >&-
	    killall -9 keepalive 2>&- >&-
        else
            kill $keepalivebootup_pid 2>&- >&-
        fi

        #echo 'Stop traffic watchdog for upgrading system'
        #stop traffic watchdog timer to prevent system reboot 
        killall -9 S${START}twatchdog 2>&- >&-
        killall -9 t_watchdog 2>&- >&-
}

DOWNLOAD()
{
cat <<EOF
&nbsp;&nbsp;&nbsp;@TR<<confman_noauto_click#If downloading does not start automatically, click here>> ... <a href="/$1">$1</a><br /><br />
<script language="JavaScript" type="text/javascript">
setTimeout('top.location.href=\"/$1\"',"5000")
</script>
EOF
}

FIRMWARE_VALID=0
check_image() {
        if [ -x "$EXTRACT" ];
        then
            VendorId="MICROHARD"
            vendor_match=$($EXTRACT -V $1 | grep "Vendor:" | grep $VendorId)
	    productname_match=$($EXTRACT -V $1 | grep "Device Model:" | grep "$PRODUCT" |awk '{print $3}') 
            if [ -n "$vendor_match" ] && [ "$productname_match" = "$PRODUCT" ] ;
            then 
                FIRMWARE_VALID=1
            else
                FIRMWARE_VALID=0
            fi                
        fi

}

reboot_bar()
{
local IPADDR="$1"
local TIMEOUT="$2"

cat <<EOF
<script type="text/javascript" src="/js/waitbox.js"></script>
<script type="text/javascript">
<!--
    showOverlay_VIP4G(document.getElementById('waitbox'));
-->
</script>
EOF

reboot_msg="
Please wait about $TIMEOUT seconds.The web interface should automatically reload.
<center>
<table>
<tr>
<td><div class=\"progressbar\" id=\"rbbar\" style=\"width:300px\">
<span id=\"bar_1\" class=\"progress\" style=\"background-color:#87CEFA; width:0%\">0%</span>
</div>
</td>
</tr>
</table>

<script type=\"text/javascript\">
<!--
//var bar1=createBar(243,15,'white',1,'black','blue',2000,15,3,'');
var N=1;
var interval=$TIMEOUT*1000/100;
setInterval('show_percent(N++)',interval);

function show_percent(tm){
    var per=tm;
    var p=document.getElementById('bar_1');
    if (per > 100){
        per=100
        p.innerHTML='100%';
    }else
        p.innerHTML=per + '%' ;

    p.style.width = per +'%';
}
-->
</script>
</center>"
?>
<br/>
<table width="90%" border="0" cellpadding="2" cellspacing="2" align="center">
<tr>
	<td><script type="text/javascript" src="/js/progress.js"></script><? echo -n "$reboot_msg" ?><br/>
</tr>
</table>
<?
      echo "<meta http-equiv=\"refresh\" content=\"$TIMEOUT;url=http://$IPADDR/cgi-bin/webif/system-info.sh\" />"
}

upgrade_bar()
{

local IPADDR="$1"
local TIMEOUT="$2"

if [ "$IPADDR" = "nochange" ];then
   RB_script=$(echo $SCRIPT_NAME|sed 's/system-upgrade.sh/system-info.sh/g')
else
   RB_script="http://$IPADDR/cgi-bin/webif/system-info.sh"
fi
cat <<EOF
<script type="text/javascript" src="/js/waitbox.js"></script>
<script type="text/javascript">
<!--
   //if the browser is IE8, do not show the waitbox
   if(navigator.appName=='Microsoft Internet Explorer'){
      var re = new RegExp("MSIE 8.0");
      var getre=re.exec(navigator.userAgent);
      if (getre == null){
            showOverlay_VIP4G(document.getElementById('waitbox'));
      }
   } else {
      showOverlay_VIP4G(document.getElementById('waitbox'));
   }
-->
</script>

EOF

reboot_msg="
Please wait about $TIMEOUT seconds.The web interface should automatically reload .
<center>
<table>
<tr>
<td><div class=\"progressbar\" id=\"rbbar\" style=\"width:300px\">
<span id=\"bar_1\" class=\"progress\" style=\"background-color:#87CEFA; width:0%\">0%</span>
</div>
<div id=\"percent\"><p>Preparing for upgrade to new firmware ...</p></div>
</td>
</tr>
</table>

<script type=\"text/javascript\">
<!--
//var bar1=createBar(243,15,'white',1,'black','blue',2000,15,3,'');
var N=1;
var interval=$TIMEOUT*1000/100;
var ipaddr=\"http://$IPADDR\"

setInterval('show_percent(N++,ipaddr)',interval);

function show_percent(tm,ip){
    var per=tm;
    var p=document.getElementById('bar_1');
    var txt=document.getElementById('percent');

    if (per > 100){
        per=100
        p.innerHTML='100%';
        txt.innerHTML='Complete upgrading, Reloading ...' + ip;
        //setTimeout(\"window.location=ipaddr\",1000);
    }else {
        if (per >=25 && per < 50 ){
            txt.innerHTML='Writing System Files ...';
        }else if (per >= 50 && per <75 ){
            txt.innerHTML='Writing Application Files ...'; 
        }else if (per >= 75 && per <=100 ){
            txt.innerHTML='Cleaning up old Files and Rebooting ...';
        } 
        p.innerHTML=per + '%' ;
    }
    p.style.width = per +'%';
}
-->
</script>
</center>"
?>
<br/>
<table width="90%" border="0" cellpadding="2" cellspacing="2" align="center">
<tr>
	<td><script type="text/javascript" src="/js/progress.js"></script><? echo -n "$reboot_msg" ?><br/>
</tr>
</table>
<?
      echo "<meta http-equiv=\"refresh\" content=\"$TIMEOUT;url=$RB_script\" />"
}

revision_text=" $SOFTWARE build $BUILD"
Built_date=$(cat /etc/banner|grep "Built time"|awk '{print $3}')
Built_time=$(cat /etc/banner|grep "Built time"|awk '{print $4}')

UB_CONFIG=$(grep params /proc/mtd | cut -d: -f1)
ub_serial=$(grep "MHX_SERIAL" /dev/$UB_CONFIG | cut -d= -f2)
ub_sku=$(grep "MHX_SKU" /dev/$UB_CONFIG | cut -d= -f2)


cat <<EOF
<script type="text/javascript" src="/js/waitbox.js"></script>
<script type="text/javascript">
<!--
function set_upgrade()
{
   //if the browser is IE8, do not show the waitbox
   if(navigator.appName=='Microsoft Internet Explorer'){
      var re = new RegExp("MSIE 8.0");
      var getre=re.exec(navigator.userAgent);
      if (getre == null){
         showLightbox_VIP4G(document.getElementById('waitbox'));
      }
   } else {
      showLightbox_VIP4G(document.getElementById('waitbox'));
   }
}
-->
</script>

EOF

cat <<EOF
<div class="settings">
<table style="width: 100%; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr><th><h3><strong>Version Information</strong></h3></th>
</table>

<table style="width: 95%; margin-left: 2em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings">
<tr>
	<th><strong>Product Name</th>
	<th><strong>Part No.</th>
	<th><strong>Serial No.</th>
	<th><strong>Hardware Type</th>
	<th><strong>Build Version</th>
	<th><strong>Build Date</th>
	<th><strong>Build Time</th>
</tr>

<tr class="odd">
	<td>$PRODUCTNAME</td>
	<td>$ub_sku</td>
	<td>$ub_serial</td>
	<td>$HARDWARE</td>
	<td>$revision_text</td>
	<td>$Built_date</td>
	<td>$Built_time</td>
</tr>
</table></div>
<br/>
EOF

if ! equal $FORM_reset "" ; then
display_form <<EOF
start_form|@TR<<Restore Default Configuration>>
EOF

cat <<EOF
<input type="submit" class="buttons" name="confirmreset" value="@TR<<OK, reset now >>" /></td></tr>
EOF

if ! equal $FORM_reset_keep_carrier "" ; then
cat <<EOF
<input type="hidden" id="hidden1" name="hidden1" value="1"/>
EOF
fi

display_form <<EOF
end_form
EOF
footer_nosubmit

elif ! equal $FORM_confirmreset "" ; then
        echo "<tr><td colspan=\"2\">@TR<<confman_restoring_conf#Reset configuration.>>"
        for file in $COPY_FILES; do
            rm $file 2>/dev/null
            [ -e /rom$file ] && [ ! -h /rom$file ] && {
                d=`dirname $file`; [ -d $d ] || mkdir -p $d
                cp -af /rom$file $file 2>/dev/null
            }
        done
    
        # if user choose to keep carrier settings
        if ! equal $FORM_hidden1 "" ; then
            logger ">>>>>>>Reset to default, but keep carrier settings 1" 
            cp /etc/config/lte /tmp/lte
        fi
        for dir in $COPY_DIRS; do
            [ -e $dir ] && {
    	        rm $dir/* 2>/dev/null
    	        cp -afr /rom$dir/* $dir/ 2>/dev/null
            }
        done

        #special handler for VERIZON E362 Module
        if [ "$PRODUCTSKU" = "MHS116705" ];then
            #this is the board for VIP4G-Verizon LTE 4G + WiFi Cellular Gateway
            #set default config apn to Verizon
            cp /rom/etc/config/lte362 /etc/config/lte
        fi

        # if user choose to keep carrier settings
        if ! equal $FORM_hidden1 "" ; then
            logger ">>>>>>>Reset to default, but keep carrier settings 2"
            cp /tmp/lte /etc/config/lte
        fi 

        DEFIP="192.168.168.1"
        reboot_bar $DEFIP 120
        reboot &
        echo "</td></tr>"
        footer_nosubmit

elif ! equal $FORM_instconfig "" ; then
        dir=$FORM_dir
    	if [ -n "$dir" ] && [ -d "$dir" ] && [ -e "$dir/config.name" ] && [ -e "$dir/config.boardtype" ] && [ -e "$dir/config.boardinfo" ]; then
   	    echo "<tr><td colspan=\"2\">@TR<<confman_restoring_conf#Restoring configuration.>><br />"
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
            NEWIP=$(uci get network.lan.ipaddr)
            #NEWADDR=$(echo $NEWIP$SCRIPT_NAME|sed 's/system-upgrade.sh/system-info.sh/g')
            reboot_bar $NEWIP 60
            reboot &
            echo "</td></tr>"
            footer_nosubmit
    	else
            echo "<p>@TR<<confman_bad_dir#bad dir>>: $dir</p>"
            footer_nosubmit
    	fi

elif ! equal $FORM_upgrade "" ; then
        if empty "$FORM_upgradefile_name"; then
display_form <<EOF
start_form|@TR<<Firmware Upgrade>>
EOF
echo "<tr><td colspan=\"2\"><font color=\"red\">@TR<<big_warning#WARNING>></font>: @TR<<Upgrade Filename cannot be empty>>!</td></tr>"
display_form<<EOF
end_form
EOF
            footer_nosubmit
        else
            check_image $FORM_upgradefile
            if [ "$FIRMWARE_VALID" != "1" ];then
                    echo "<br /><p><font color=\"red\">Upgrading firmware, Wrong firmware to update !!! </p><br />"
                    footer_nosubmit
            else
                    cp $FORM_upgradefile /tmp/waitingforupgrade           
                    if [ "$?" != "0" ]; then  
                        logger -t "Systemupgrade" "No more memery left ... "
                        echo "<br /><p><font color=\"red\">Upgrading firmware, Memory Not enough. Please reboot system first and upgrade again!!! </p><br />"
                        footer_nosubmit
                    else
                    echo "<br />Upgrading firmware, please wait ... <br />"
                    #just in case to prevent system reboot
                    disable_reboot
                    #check ippassthrough backup file  
                    if [ -f "/etc/dhcp_ippbackup" ] ;then
                        mv /etc/dhcp_ippbackup /etc/config/dhcp
                        logger -t "ippassthrough" "restore dhcp config file to normal"
                    fi

        	    if [ "$FORM_nokeepconfig" = "1" ]; then
                        DEFIP="192.168.168.1"
                        upgrade_bar $DEFIP 400
                        footer_nosubmit
                        sysupgrade -n /tmp/waitingforupgrade >/dev/null 2>&1 &
        	    elif [ "$FORM_nokeepconfig" = "0" ];then
                        #NEWIP=$(uci get network.lan.ipaddr)
                        upgrade_bar nochange 400
                        footer_nosubmit
        		sysupgrade /tmp/waitingforupgrade >/dev/null 2>&1 &
                    elif [ "$FORM_nokeepconfig" = "2" ]; then
                        DEFIP="192.168.168.1"
                        upgrade_bar $DEFIP 400
                        footer_nosubmit
        		sysupgrade -c /tmp/waitingforupgrade >/dev/null 2>&1 &
        	    else
                        #NEWIP=$(uci get network.lan.ipaddr)
                        upgrade_bar nochange 400
                        footer_nosubmit
        		sysupgrade /tmp/waitingforupgrade >/dev/null 2>&1 &
        	        #echo "<br />@TR<<Rebooting now>>...<meta http-equiv=\"refresh\" content=\"4;url=reboot.sh?reboot=1\">"
        	    fi
                    #/etc/VIP4G_upgrade.sh $FORM_upgradefile $FORM_nokeepconfig
                    fi
            fi
    fi
elif ! equal $FORM_download "" ; then
        echo "<br />Downloading Configuration File, please wait ... <br />"
        
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
        footer_nosubmit
        sleep 25 
        #rm $tgz

elif ! equal $FORM_chkconfig "" ; then
        if [ -n "$FORM_configfile" ] && [ -e "$FORM_configfile" ]; then

            echo "<form method=\"get\" name=\"install\" action=\"$SCRIPT_NAME\">"
display_form <<EOF
start_form|@TR<<Restore Configuration>>
EOF
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
                elif [ "${PRODUCTNAME}-${PRODUCT}" != "$(echo $bi |awk -F ":" '{print $2}')" ]; then
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
display_form <<EOF
end_form
EOF
            echo "</form>"
        fi
        footer_nosubmit
else

    if empty "$FORM_upgrade"; then
display_form <<EOF
start_form|@TR<<Firmware Upgrade>>
field|@TR<<Erase Current Configuration>>
select|nokeepconfig|$FORM_nokeepconfig
option|0|@TR<<Keep ALL Configuration>>
option|2|@TR<<Keep Carrier Configuration>>
option|1|@TR<<Erase Configuration>>

field|@TR<<Firmware Image>>
upload|upgradefile|$FORM_upgradefile_name
field|@TR<<Upgrade>>
EOF

cat <<EOF
<input type="submit" class="buttons" name="upgrade" value="@TR<<Upgrade Firmware>>" onclick="set_upgrade();"/></td></tr>
EOF

display_form <<EOF
end_form
EOF
    fi

cat <<EOF
<form method="post" name="download" action="$SCRIPT_NAME" enctype="multipart/form-data">
EOF

display_form <<EOF
start_form|@TR<<Reset to Default>>
EOF

cat <<EOF
<tr><td width="40%">@TR<<Reset to Default>></td>
<td><input class="buttons" type="submit" name="reset" value="@TR<<Reset to Default>>"/></td>
<td><input id="reset_keep_carrier" type="checkbox" name="reset_keep_carrier" value="1" checked="true" />Keep Carrier Settings</td>
</tr> 
EOF

display_form <<EOF
end_form
EOF

display_form <<EOF
start_form|@TR<<Backup Configuration>>
EOF

if [ "$PRODUCT" = "IPnVT" ];then
    PRODUCT_CONF="nVIP2400"
else
    PRODUCT_CONF="$PRODUCT"
fi
cat <<EOF
<tr><td width="40%">@TR<<Name this configuration>></td>
<td><input name="name" value="${FORM_name:-$PRODUCT_CONF}" /></td>
</tr>
<tr><td width="40%">@TR<<Backup>></td>
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
<td>@TR<<Restore Configuration file>></td>
<td><input type="file" class="buttons" name="configfile" /></td>
</tr>
<tr>
<td>@TR<<Check Configuration file>></td>
<td><input class="buttons" type="submit" name="chkconfig" value="@TR<<Check Restore File>>" /></td>
</tr>
EOF

display_form <<EOF
end_form|
string|</form>
EOF
footer_nosubmit
fi
?>
<!--
##WEBIF:name:System:900:Maintenance
-->
