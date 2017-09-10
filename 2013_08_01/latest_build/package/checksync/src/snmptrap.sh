#! /bin/sh

version=$1
community=$2
remoteip=$3
oid=$4
vtype=$5
value=$6
myuser=$7
mypassword=$8
v3auth=$9

OID_STRING="1.3.6.1.4.1.21703.6000.100"
OID_V2_STRING="1.3.6.1.6.3.1.1.4.1.0"

[ "$remoteip" = "0.0.0.0" ] || { 
    if [ "x$version" = "xA" ]; then    
        snmptrap -v1 -c $community $remoteip  $OID_STRING '' 0 0 "" $oid $vtype "$value"
        ##echo "snmptrap -v1 -c $community $remoteip  $OID_STRING '' 0 0 "" $oid $vtype $value"
    
    elif [ "x$version" = "xB" ]; then
    
        snmptrap -v2c -c $community $remoteip uptime $OID_V2_STRING $oid $vtype "$value"
        #echo "snmptrap -v2c -c $community $remoteip uptime $OID_V2_STRING $oid $vtype $value" 
    
    elif [ "x$version" = "xC" ]; then
    
        snmptrap -v 3 -u $myuser -a MD5 -A $mypassword -l $v3auth  $remoteip uptime $OID_V2_STRING $oid $vtype "$value"
        #echo "snmptrap -v 3 -u $myuser -a MD5 -A $mypassword -l $v3auth  $remoteip uptime $OID_V2_STRING $oid $vtype $value" 
    
    elif [ "x$version" = "xD" ]; then
    
        snmptrap -v1 -c $community $remoteip  $OID_STRING '' 0 0 "" $oid $vtype "$value"
        snmptrap -v2c -c $community $remoteip "" $OID_V2_STRING $oid $vtype "$value" 
        #echo "snmptrap -v1 -c $community $remoteip  $OID_STRING '' 0 0 "" $oid $vtype $value"
        #echo "snmptrap -v2c -c $community $remoteip "" $OID_V2_STRING $oid $vtype $value"
    
    elif [ "x$version" = "xE" ]; then
    
        snmptrap -v1 -c $community $remoteip  $OID_STRING '' 0 0 "" $oid $vtype "$value"
        snmptrap -v2c -c $community $remoteip uptime $OID_V2_STRING $oid $vtype "$value" 
        snmptrap -v 3 -u $myuser -a MD5 -A $mypassword -l $v3auth  $remoteip uptime $OID_V2_STRING $oid $vtype "$value"
        
        #echo "snmptrap -v1 -c $community $remoteip  $OID_STRING '' 0 0 "" $oid $vtype $value"
        #echo "snmptrap -v2c -c $community $remoteip uptime $OID_V2_STRING $oid $vtype $value" 
        #echo "snmptrap -v 3 -u $myuser -a MD5 -A $mypassword -l $v3auth  $remoteip uptime $OID_V2_STRING $oid $vtype $value" 
     
    else
    
        snmptrap -v2c -c $community $remoteip uptime $OID_V2_STRING $oid $vtype "$value" 
        #echo "snmptrap -v2c -c $community $remoteip uptime $OID_V2_STRING $oid $vtype $value"
    
    fi
}
