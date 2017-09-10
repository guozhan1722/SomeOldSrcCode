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
   [ -f /var/log/messages.1 ] && cat /var/log/messages.1 2>/dev/null |awk '{printf("%s<br>",$0)}'
   [ -f /var/log/messages.0 ] && cat /var/log/messages.0 2>/dev/null |awk '{printf("%s<br>",$0)}'
   cat /var/log/messages 2>/dev/null |awk '{printf("%s<br>",$0)}'
   ;;
cat-conf)
   cat /etc/ipsec.conf 2>/dev/null |awk '{printf("%s<br>",$0)}'
   ;;
gre-tunnel)
   ip tunnel show 2>/dev/null |awk '{printf("%s<br>",$0)}'
   ;;
vpnlog)
   [ -f /var/log/vpnlog.1 ] && awk '{printf("%s<br>",$0)}' </var/log/vpnlog.1
   [ -f /var/log/vpnlog.0 ] && awk '{printf("%s<br>",$0)}' </var/log/vpnlog.0
   awk '{printf("%s<br>",$0)}' </var/log/vpnlog
   ;;
*)
   echo "Usage: http://ipaddress/cgi-bin/webif/ipsec.sh?cmd=listall|status|enable-debug|disable-debug|cat-secrets|syslog|cat-conf|gre-tunnel|vpnlog"
   ;;
esac
echo "</body></html>"
?>
