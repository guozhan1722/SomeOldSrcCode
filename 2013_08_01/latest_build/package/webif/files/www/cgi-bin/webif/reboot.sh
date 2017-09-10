#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
. /etc/version


productname="$PRODUCTNAME"

write2systemdebug(){
   dfile_size=$(ls -s /etc/system.debug |awk '{print $1}')
   if [ "$dfile_size" -gt "2" ];then
      echo -n `date`>/etc/system.debug   
   else
      echo -n `date`>>/etc/system.debug   
   fi
   echo " User reboot : user make system reboot." >>/etc/system.debug
}
#timeout=120

if empty "$FORM_reboot"; then
	reboot_msg="<form method=\"post\" action=\"$SCRIPT_NAME\"><input type=\"submit\" value=\" @TR<<OK, reboot now>> \" name=\"reboot\" /></form>"
else
        ssl_enable=`uci get webif.general.ssl`

	uci_load "network"
        [ -z "$FORM_defaultip" ] && {
            need_default=0
        }||{
            need_default=1
        }

        [ -z "$FORM_timeout" ] && {
            timeout=60
        }||{
            timeout=$FORM_timeout
        }
        
        
        RB_script=$(echo $SCRIPT_NAME|sed 's/reboot.sh/system-info.sh/g')

        if [ "$need_default" = "1" ]; then
	    header_inject_head="<meta http-equiv=\"refresh\" content=\"$timeout;url=http://$FORM_defaultip/cgi-bin/webif/system-info.sh\" />"
        else
	    header_inject_head="<meta http-equiv=\"refresh\" content=\"$timeout;url=$RB_script\" />"
        fi
	reboot_msg="@TR<<Rebooting now>>...
<br/>
@TR<<reboot_wait#Please wait about>> $timeout @TR<<reboot_seconds#seconds.>> @TR<<reboot_reload#The web interface should automatically reload.>>
<br/>
<center>
<script type=\"text/javascript\">
<!--
var bar1=createBar(350,15,'white',1,'black','blue',85,7,3,'');
-->
</script>
</center>"
fi

header "System" "Reboot" ""
?>
<br/>
<table width="90%" border="0" cellpadding="2" cellspacing="2" align="center">
<tr>
	<td><script type="text/javascript" src="/js/progress.js"></script><? echo -n "$reboot_msg" ?><br/>
</tr>
</table>
<? footer_nosubmit ?>
<?
! empty "$FORM_reboot" && {
        write2systemdebug
	reboot &
	exit
}
?>
<!--
##WEBIF:name:System:910:Reboot
-->
