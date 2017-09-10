#!/bin/sh
local arg="$1"
if [ "$arg" = "C" ];then
awk -f /usr/lib/common.awk -f - $* <<EOF
BEGIN {
	netmask=ip2int(ARGV[2])

        if( "255.255.255.0" == int2ip(and(ip2int("255.255.255.0"),netmask)) ) {
            nettype="C"
        }else if( "255.255.0.0" == int2ip(and(ip2int("255.255.0.0"),netmask)) ) {
            nettype="B"
        }else if( "255.0.0.0" == int2ip(and(ip2int("255.0.0.0"),netmask)) ) {
            nettype="A"
        } else nettype="Unknown"

	print "NetType="nettype
}
EOF

elif [ "$arg" = "N" ];then
awk -f /usr/lib/common.awk -f - $* <<EOF
BEGIN {
	ipaddr=ip2int(ARGV[2])
	netmask=ip2int(ARGV[3])
	network=and(ipaddr,netmask)
	print "NETWORK="int2ip(network)
}
EOF
elif [ "$arg" = "S" ];then
awk -f /usr/lib/common.awk -f - $* <<EOF
BEGIN {
	ipaddr=ip2int(ARGV[2])
	netmask=ip2int(ARGV[3])
	network=and(ipaddr,netmask)
        start=or(network,and(ip2int(ARGV[4]),compl(netmask)))
	print "FORM_dstart="int2ip(start)
}
EOF

elif [ "$arg" = "E" ];then
awk -f /usr/lib/common.awk -f - $* <<EOF
BEGIN {
	ipaddr=ip2int(ARGV[2])
	netmask=ip2int(ARGV[3])
	network=and(ipaddr,netmask)
	start=or(network,and(ip2int(ARGV[4]),compl(netmask)))

	end=start+ARGV[5]
	limit=or(network,compl(netmask))-1
	if (end>limit){
            print "ERRORLim=\"Error in LAN DHCP Limit: Requested too many, should be less than: \""limit-start
            end=limit
        }

       	reqstart=ip2int(ARGV[4])
        if (reqstart >limit ||reqstart< network+1 ){
            print "ERRORST=\"Error in LAN DHCP Start: IP Not In Range: \"" int2ip(network+1) "-" int2ip(limit)
        }
}
EOF
else
awk -f /usr/lib/common.awk -f - $* <<EOF
BEGIN {
	ipaddr=ip2int(ARGV[1])
	netmask=ip2int(ARGV[2])
	network=and(ipaddr,netmask)
	broadcast=or(network,compl(netmask))
	start=or(network,and(ip2int(ARGV[3]),compl(netmask)))

        if( "255.255.255.0" == int2ip(and(ip2int("255.255.255.0"),netmask)) ) {
            nettype="C"
        }else if( "255.255.0.0" == int2ip(and(ip2int("255.255.0.0"),netmask)) ) {
            nettype="B"
        }else if( "255.0.0.0" == int2ip(and(ip2int("255.0.0.0"),netmask)) ) {
            nettype="A"
        } else nettype="Unknown"

	limit=network+1
	if (start<limit) start=limit
	
	end=start+ARGV[4]
	limit=or(network,compl(netmask))-1
	if (end>limit){
            print "Wrong End IP address can not over than " int2ip(limit)
            end=limit
        }
	print "IP="int2ip(ipaddr)
	print "NETMASK="int2ip(netmask)
	print "BROADCAST="int2ip(broadcast)
	print "NETWORK="int2ip(network)
	print "PREFIX="32-bitcount(compl(netmask))
	print "NetType="nettype
	
	# range calculations:
	# ipcalc <ip> <netmask> <start> <num>
	if (ARGC > 3) {
        	reqstart=ip2int(ARGV[3])
        	if (reqstart >limit ||reqstart< network+1 ){
                    print "Wrong start IP address the range is " int2ip(network+1) " and " int2ip(limit)
                }

		print "START="int2ip(start)
		print "END="int2ip(end)
        	print "limit="int2ip(limit)
	}
}
EOF

fi
