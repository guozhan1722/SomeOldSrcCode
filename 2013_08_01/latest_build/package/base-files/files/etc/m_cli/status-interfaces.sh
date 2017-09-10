ifconfig eth0 > /tmp/interfaces
ifconfig eth1 >> /tmp/interfaces
ifconfig wlan0 >> /tmp/interfaces

cat /tmp/interfaces
rm /tmp/interfaces 
