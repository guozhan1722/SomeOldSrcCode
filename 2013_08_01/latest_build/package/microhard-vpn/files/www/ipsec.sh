#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

echo "Content-type: text/html"
echo
echo "<html>"
echo "<head>"
echo "<title>Run Commands</title>"
echo "<body>"

echo "<h2>$FORM_cmd</h2>"
case $FORM_cmd in
listall)
   /usr/sbin/ipsec auto --listall 2>/dev/null |awk '{printf("%s<br>",$0)}'
   ;;
status)
   /usr/sbin/ipsec auto --status 2>/dev/null |awk '{printf("%s<br>",$0)}'
   ;;
cat-secrets)
   cat /etc/ipsec.secrets 2>/dev/null |awk '{printf("%s<br>",$0)}'
   ;;
enable-debug)
   touch /etc/debugipsec
   ;;
disable-debug)
   rm -f /etc/debugipsec
   ;;
syslog)
   awk '{printf("%s<br>",$0)}' </var/log/messages
   ;;
cat-conf)
   cat /etc/ipsec.conf 2>/dev/null |awk '{printf("%s<br>",$0)}'
   ;;
x-state)
   ip xfrm state 2>/dev/null |awk '{printf("%s<br>",$0)}'
   ;;
x-policy)
   ip xfrm policy 2>/dev/null |awk '{printf("%s<br>",$0)}'
   ;;
gre-tunnel)
   ip tunnel show 2>/dev/null |awk '{printf("%s<br>",$0)}'
   ;;
*)
   echo "Usage: http://ipaddress/cgi-bin/webif/ipsec.sh?cmd=listall|status|enable-debug|disable-debug|cat-secrets|syslog|cat-conf|x-state|x-policy|gre-tunnel"
   ;;
esac
echo "</body></html>"
?>
