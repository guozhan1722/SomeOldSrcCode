#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
. /etc/version

PRON=`cat /etc/version |grep PRODUCTNAME`
PRON_R=$(echo $PRON |grep IPnV)

productname="VIP2.0"

if [ "$PRODUCT" = "IPnVT" ];then
    productname="IPnVT"
fi

[ -z $PRON_R ] && {
   timeout=120
}||{
   timeout=80
}

if empty "$FORM_reboot"; then
	reboot_msg="<form method=\"post\" action=\"$SCRIPT_NAME\"><input type=\"submit\" value=\" @TR<<OK, reboot now>> \" name=\"reboot\" /></form>"
else
	uci_load "network"
        [ -z "$FORM_defaultip" ] && {
	    router_ip="$CONFIG_lan_ipaddr"
        }||{
	    router_ip="$FORM_defaultip"
        }

	[ -n "$SERVER_PORT" ] && [ "$SERVER_PORT" != "80" ] && router_ip="$router_ip:$SERVER_PORT"
	header_inject_head="<meta http-equiv=\"refresh\" content=\"$timeout;url=http://$router_ip/cgi-bin/webif/system-info.sh\" />"
	reboot_msg="@TR<<Rebooting now>>...
<br/><br/>
@TR<<reboot_wait#Please wait about>> $timeout @TR<<reboot_seconds#seconds.>> @TR<<reboot_reload#The $productname web interface should automatically reload.>>
<br/><br/>
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
<br/><br/><br/>
<table width="90%" border="0" cellpadding="2" cellspacing="2" align="center">
<tr>
	<td><script type="text/javascript" src="/js/progress.js"></script><? echo -n "$reboot_msg" ?><br/><br/><br/></td>
</tr>
</table>
<? footer ?>
<?
! empty "$FORM_reboot" && {
	reboot &
	exit
}
?>
<!--
##WEBIF:name:System:910:Reboot
-->
