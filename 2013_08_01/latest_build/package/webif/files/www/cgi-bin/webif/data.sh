#!/usr/bin/webif-page
<?
[ -f /var/run/iperf_unit ] && {
    . /var/run/iperf_unit
    if [ "$unit" = "k" ];then
        factor=8000
    elif [ "$unit" = "K" ];then
        factor=1000
    elif [ "$unit" = "m" ];then
        factor=8000000
    elif [ "$unit" = "M" ];then
        factor=1000000
    fi
} || {
    unit="m"
}
echo "the unit is $unit factor is $factor" >/etc/aaaa
echo -e "\r"`date`
if [ "$FORM_if" ]; then
  if [ "$FORM_if" = "iperf" ]; then
#	grep "${FORM_if}:" /proc/net/dev
	if [ "$unit" = "m" ]; then
                cat /tmp/run/iperf_run |grep -v Please|awk -F "<br/>" '{print $(NF-1)}'| awk '{print "br-lan:"$7*1000000/8" 0 0 0 0 0 0 0 0 0 0 0 0 0 0"}'
#                tail -c 54 /var/run/iperf_run | awk '{print "br-lan:"$7*1000000/8" 1000 2000 3000 0 0 0 0 "$5*10" "$7*10" 0 0 0 0 0"}'
#    		tail -n1 /var/run/iperf_run |awk '{print "br-lan:"$7*1000000/8" 1000 2000 3000 0 0 0 0 "$5*10" "$7*10" 0 0 0 0 0"}'
#		echo "br-lan:1000 2000 3000 4000 5000 6000 7000 8000 9000 1000 1100 1200"
    	elif [ "$unit" = "M" ]; then
                cat /tmp/run/iperf_run |grep -v Please|awk -F "<br/>" '{print $(NF-1)}'|awk '{print "br-lan:"$7*1000000" 0 0 0 0 0 0 0 0 0 0 0 0 0 0"}'
#    		#tail -n1 /var/run/iperf_run |awk '{print "br-lan:"$7*1000000" 0 0 0 0 0 0 0 "$5*100" "$7*100" 0 0 0 0 0"}'
    	elif [ "$unit" = "k" ]; then
                cat /tmp/run/iperf_run |grep -v Please|awk -F "<br/>" '{print $(NF-1)}'|awk '{print "br-lan:"$7*1000/8" 0 0 0 0 0 0 0 0 0 0 0 0 0 0"}'
#    		#tail -n1 /var/run/iperf_run |awk '{print "br-lan:"$7*1000/8" 0 0 0 0 0 0 0 0 0 0 0 0"}'
    	elif [ "$unit" = "K" ]; then
                cat /tmp/run/iperf_run |grep -v Please|awk -F "<br/>" '{print $(NF-1)}'|awk '{print "br-lan:"$7*1000" 0 0 0 0 0 0 0 0 0 0 0 0 0 0"}'
#    		#tail -n1 /var/run/iperf_run |awk '{print "br-lan:"$7*1000" 0 0 0 0 0 0 0 0 0 0 0 0"}'
	fi
  else
	grep "${FORM_if}:" /proc/net/dev
  fi	
	
else
	head -n 1 /proc/stat
fi
?>
