#!/bin/ash
 
. /etc/functions.sh

me="mipsec.sh[$$]"

config_cb() {
    local cfg_type="$1"
    local cfg_name="$2"

    case "$cfg_type" in
        s2stunnel)
            append s2s_cfgs "$cfg_name" "$N"
        ;;
        x2ctunnel)
            append x2c_cfgs "$cfg_name" "$N"
        ;;
        vca)
            append vca_cfgs "$cfg_name" "$N"
        ;;
        x2stunnel)
            append x2s_cfgs "$cfg_name" "$N"
        ;;
        gretunnel)
            append gre_cfgs "$cfg_name" "$N"
        ;;
    esac
}

my_lockfile ()
{
        TEMPFILE="$1.$$"
        LOCKFILE="$1.lock"
        echo $$ > $TEMPFILE || {
                echo "You don't have permission to access `dirname $TEMPFILE`"
                return 1
        }
        ln $TEMPFILE $LOCKFILE && {
                rm -f $TEMPFILE
                return 0
        }
        kill -0 `cat $LOCKFILE` && {
                rm -f $TEMPFILE
                return 1
        }
        echo "Removing stale lock file"
        rm -f $LOCKFILE
        ln $TEMPFILE $LOCKFILE && {
                rm -f $TEMPFILE
                return 0
        }
        rm -f $TEMPFILE
        return 1
}
 
[ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@"
 
case "$1" in 
update_vca)
    uci_load ipsec
    cnt="1"
    [ -z $vca_cfgs ] && {
             [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ vca_cfgs is empty"
             echo "" > /etc/ppp/chap-secrets
             echo "" > /etc/ppp/pap-secrets
    } || {
    for vca in $vca_cfgs; do
        config_get vca_pwd $vca pwd
        [ $cnt = "1" ] && {
             echo "$vca * $vca_pwd *" > /etc/ppp/chap-secrets
             echo "$vca * $vca_pwd *" > /etc/ppp/pap-secrets
        } || {
             echo "$vca * $vca_pwd *" >> /etc/ppp/chap-secrets
             echo "$vca * $vca_pwd *" >> /etc/ppp/pap-secrets
        }
        [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ write $vca * $vca_pwd *"
        let "cnt+=1"
    done
    }
    ;;

###############################################################################################
#              operations regarding to gateway to gateway vpn tunnel                          #
###############################################################################################
#$2--tunnel name
del_s2s)
   [ -f /var/run/mipsec/${2}.pid ] && {
   plist=$(cat /var/run/mipsec/${2}.pid)
   rm -f /var/run/mipsec/${2}.pid
   logger -p local0.error -t "$me" "$@ plist=$plist"
   for pl in $plist
   do
       kill -9 $pl
   done
   }
   /usr/sbin/ipsec auto --delete "$2" >/dev/null 2>&1 
   /usr/sbin/ipsec auto --delete "${2}_tb" >/dev/null 2>&1
   [ -f /etc/ipsec.d/${2}.backup ] && rm -f /etc/ipsec.d/${2}.backup
   rm -f /var/run/mipsec/$2.status
   [ -f /var/run/ipsecfirewall/firewall.$2 ] && rm -f /var/run/ipsecfirewall/firewall.$2
   [ -f /var/run/ipsecfirewall/dfirewall.$2 ] && {
         sh /var/run/ipsecfirewall/dfirewall.$2
         rm -f /var/run/ipsecfirewall/dfirewall.$2
   }
   /etc/init.d/ipsec_vpn start >/dev/null 2>&1
   ;;

#$2--tunnel name
add_s2s)
    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
    enabled=$(/sbin/uci get ipsec.${2}.enabled)
    [ "$enabled" = "1" ] && {
        right=$(/sbin/uci get ipsec.${2}.right)
        echo "action=add" > /var/run/mipsec/$2.status
        echo "status=waiting for connection" >> /var/run/mipsec/$2.status
        echo "constat=Connect" >> /var/run/mipsec/$2.status
        echo "peer=$right" >> /var/run/mipsec/$2.status
        interface=$(/sbin/uci get ipsec.${2}.interface)
        if [ $interface = "br-wan2" ]
        then
            wan_status=$(/usr/lib/mipsec check_wan2)
        else
            wan_status=$(/usr/lib/mipsec check_wan)
        fi
        [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ wan_status=$wan_status interface=$interface"
        if [ "$wan_status" -eq 8 ]; then
              /usr/sbin/ipsec auto --add $2 >/dev/null 2>&1
        fi
    }
    ;;

#$2--tunnel name
edit_s2s)
   [ -f /var/run/mipsec/${2}.pid ] && {
   plist=$(cat /var/run/mipsec/${2}.pid)
   rm -f /var/run/mipsec/${2}.pid
   logger -p local0.error -t "$me" "$@ plist=$plist"
   for pl in $plist
   do
       kill -9 $pl
   done
   }
    /usr/sbin/ipsec auto --down "$2" >/dev/null 2>&1
    /usr/sbin/ipsec auto --delete "$2" >/dev/null 2>&1
    /usr/sbin/ipsec auto --delete "${2}_tb" >/dev/null 2>&1
    [ -f /etc/ipsec.d/${2}.backup ] && rm -f /etc/ipsec.d/${2}.backup
    [ -f /var/run/ipsecfirewall/firewall.$t2 ] && rm -f /var/run/ipsecfirewall/firewall.$2
    [ -f /var/run/ipsecfirewall/dfirewall.$2 ] && {
         sh /var/run/ipsecfirewall/dfirewall.$2
         rm -f /var/run/ipsecfirewall/dfirewall.$2
    }
    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
    enabled=$(/sbin/uci get ipsec.${2}.enabled)
    [ "$enabled" = "1" ] && {
        right=$(/sbin/uci get ipsec.${2}.right)
        echo "action=add" > /var/run/mipsec/$2.status
        echo "status=waiting for connection" >> /var/run/mipsec/$2.status
        echo "constat=Connect" >> /var/run/mipsec/$2.status
        echo "peer=$right" >> /var/run/mipsec/$2.status
        interface=$(/sbin/uci get ipsec.${2}.interface)
        if [ $interface = "br-wan2" ]
        then
            wan_status=$(/usr/lib/mipsec check_wan2)
        else
            wan_status=$(/usr/lib/mipsec check_wan)
        fi
        [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ wan_status=$wan_status interface=$interface"
        if [ "$wan_status" -eq 8 ]; then
              /usr/sbin/ipsec auto --add $2 >/dev/null 2>&1
        fi
    }
    ;;

#$2--tunnel name
connect_s2s)
   [ -f /var/run/mipsec/${2}.pid ] && {
   plist=$(cat /var/run/mipsec/${2}.pid)
   rm -f /var/run/mipsec/${2}.pid
   logger -p local0.error -t "$me" "$@ plist=$plist"
   for pl in $plist
   do
       kill -9 $pl
   done
   }
    interface=$(/sbin/uci get ipsec.${2}.interface)
    if [ $interface = "br-wan2" ]
    then
         wan_status=$(/usr/lib/mipsec check_wan2)
    else
         wan_status=$(/usr/lib/mipsec check_wan)
    fi

    [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ wan_status=$wan_status interface=$interface"
    case "$wan_status" in
    #1 -- interface down or ip_addr is 0.0.0.0
    1)
          echo "status=interface down" > /var/run/mipsec/$2.status
          echo "constat=N/A" >> /var/run/mipsec/$2.status
          echo "action=up" >> /var/run/mipsec/$2.status
          right=$(/sbin/uci get ipsec.${2}.right)
          echo "peer=$right" >> /var/run/mipsec/$2.status
          exit 0
          ;;
    #2 -- ipsec is not running
    #3 -- interface is not added into ipsec
    2|3)
          /sbin/mifupdown restart >/dev/null 2>&1 &
          exit 0
          ;;
    #8 -- success
    8)
          /usr/lib/mipsec s2s_up $2 >/dev/null 2>&1 &
          exit 0
          ;;
    *)
          logger -p local0.error -t "$me" "ERROR $@ wan_status=$wan_status interface=$interface"
          ;;
    esac
    ;;

#$2--tunnel name
disconnect_s2s)
   [ -f /var/run/mipsec/${2}.pid ] && {
   plist=$(cat /var/run/mipsec/${2}.pid)
   rm -f /var/run/mipsec/${2}.pid
   logger -p local0.error -t "$me" "$@ plist=$plist"
   for pl in $plist
   do
       kill -9 $pl
   done
   }
    [ -f "/var/run/mipsec/${2}.status" ] && {
    s2s_peer=`cat /var/run/mipsec/${2}.status | grep 'peer' | cut -d '=' -f 2`
    dpdaction=$(/sbin/uci get ipsec.${2}.dpdaction)
    logger -p local0.error -t "$me" "$@ dpdaction=$dpdaction s2s_peer=$s2s_peer" 
    if [ $dpdaction = "restart_by_peer" ]; then
          s2s_right=$(/sbin/uci get ipsec.${2}.right)
          if [ $s2s_peer = "$s2s_right" ]; then
               /usr/sbin/ipsec auto --down "$2"
          else
               /usr/sbin/ipsec auto --delete "${2}_tb"
               /usr/sbin/ipsec auto --add "$2"
               /usr/lib/mipsec s2s_up $2 >/dev/null 2>&1 &
          fi
    else
          /usr/sbin/ipsec auto --down "$2"              
    fi
    } || {
    /usr/sbin/ipsec auto --down "$2"
    }
    ;;

#$2--tunnel name
#$3--peer host ip
backup_s2s)
    ttype=$(/sbin/uci get ipsec.${2})
    logger -p local0.error -t "$me" "$@ ttype=$ttype"
    tun="$2"
    case $ttype in
    s2stunnel)
        /usr/sbin/ipsec auto --delete "$2"
        ;;
    s2stb)
        /usr/sbin/ipsec auto --delete "$2"
        tun=$(/sbin/uci get ipsec.${2}.primary)
        ;;
    *)
        logger -p local0.error -t "$me" "ERROR $@ ttype=$ttype"
        exit 0
        ;;
    esac
    logger -p local0.error -t "$me" "$@ tunnel=$tun"
    bright=$(/sbin/uci get ipsec.${tun}.bright)
    right=$(/sbin/uci get ipsec.${tun}.right)
    if [ "$3" = "$bright" ]
    then
         interface=$(/sbin/uci get ipsec.${tun}.interface)
    else
         interface=$(/sbin/uci get ipsec.${tun}.binterface)
    fi

    if [ $interface = "br-wan2" ]
    then
         wan_status=$(/usr/lib/mipsec check_wan2)
    else
         wan_status=$(/usr/lib/mipsec check_wan)
    fi

    logger -p local0.error -t "$me" "$@ wan_status=$wan_status interface=$interface"
    case "$wan_status" in
    #1 -- interface down or ip_addr is 0.0.0.0
    1)
        if [ "$3" = "$bright" ]
        then
          echo "status=interface down" > /var/run/mipsec/${tun}.status
          echo "constat=N/A" >> /var/run/mipsec/${tun}.status
        else
          echo "status=interface(S) down" > /var/run/mipsec/${tun}.status
          echo "constat=Connect" >> /var/run/mipsec/${tun}.status
        fi
        echo "action=up" >> /var/run/mipsec/${tun}.status
        echo "peer=$right" >> /var/run/mipsec/${tun}.status
        ipsec auto --add "${tun}"
        exit 0
        ;;
    #2 -- ipsec is not running
    #3 -- interface is not added into ipsec
    2|3)
          /sbin/mifupdown restart >/dev/null 2>&1 &
          exit 0
          ;;
    #8 -- success
    8)
          if [ "$3" = "$bright" ]
          then
               /usr/lib/mipsec s2s_up $tun >/dev/null 2>&1 &
          else
               ipsec auto --rereadall >/dev/null 2>&1
               echo "status=waiting for connection" > /var/run/mipsec/${tun}.status
               echo "constat=Waiting..." >> /var/run/mipsec/${tun}.status
               echo "action=up" >> /var/run/mipsec/${tun}.status
               echo "peer=$bright" >> /var/run/mipsec/${tun}.status
               ipsec auto --add --config /etc/ipsec.d/${tun}.backup "${tun}_tb"
               ipsec auto --add "${tun}"
               ipsec auto --asynchronous --up "${tun}_tb" &
          fi
          exit 0
          ;;
    *)
          logger -p local0.error -t "$me" "ERROR $@ wan_status=$wan_status interface=$interface"
          ;;
    esac
    ;;

#$2--tunnel name
vtbit_s2s)
    logger -p local0.error -t "$me" "$@"
    ttype=$(/sbin/uci get ipsec.${2})
    if [ $ttype = "s2stunnel" ]
    then
        
        dpdaction=$(/sbin/uci get ipsec.${2}.dpdaction)
        if [ "$dpdaction" = "restart_by_peer" ]; then
             echo "$$" >> /var/run/mipsec/${2}.pid
             vtbit=$(/sbin/uci get ipsec.${2}.vtbit)
             logger -p local0.error -t "$me" "$@ sleep $vtbit"
             sleep $vtbit
             logger -p local0.error -t "$me" "$@ end sleep"
 
             interface=$(/sbin/uci get ipsec.${2}.binterface)
             [ -z $interface ] && {
                  logger -p local0.error -t "$me" "$@ interface=$interface ao exit"
                  exit 0
             }
             cmd="ipsec auto --status | grep '\"$2\"' | grep -c 'IPsec SA established'"
             result=`eval $cmd`
             [ $result -gt 0 ] && {
                  logger -p local0.error -t "$me" "$@ result=$result ao exit"
                  rm -f /var/run/mipsec/${2}.pid
                  exit 0
             }
             if [ $interface = "br-wan2" ]
             then
                  wan_status=$(/usr/lib/mipsec check_wan2)
             else
                  wan_status=$(/usr/lib/mipsec check_wan)
             fi

             logger -p local0.error -t "$me" "$@ wan_status=$wan_status interface=$interface"
             case "$wan_status" in
             #1 -- interface down or ip_addr is 0.0.0.0
             1)
                right=$(/sbin/uci get ipsec.${2}.right)
                echo "status=interface(S) down" > /var/run/mipsec/${2}.status
                echo "constat=Connect" >> /var/run/mipsec/${2}.status
                echo "action=up" >> /var/run/mipsec/${2}.status
                echo "peer=$right" >> /var/run/mipsec/${2}.status
             ;;
             #2 -- ipsec is not running
             #3 -- interface is not added into ipsec
             2|3)
                /sbin/mifupdown restart >/dev/null 2>&1 &
             ;;
             #8 -- success
             8)
               bright=$(/sbin/uci get ipsec.${2}.bright)
               /usr/sbin/ipsec auto --delete "$2"
               ipsec auto --rereadall >/dev/null 2>&1
               echo "status=waiting for connection" > /var/run/mipsec/${2}.status
               echo "constat=Waiting..." >> /var/run/mipsec/${2}.status
               echo "action=up" >> /var/run/mipsec/${2}.status
               echo "peer=$bright" >> /var/run/mipsec/${2}.status
               ipsec auto --add --config /etc/ipsec.d/${2}.backup "${2}_tb"
               /usr/sbin/ipsec auto --add "$2"
               ipsec auto --asynchronous --up "${2}_tb" &
             ;;
             *)
               logger -p local0.error -t "$me" "ERROR $@ wan_status=$wan_status interface=$interface"
             ;;
             esac
             exit 0
        else
             logger -p local0.error -t "$me" "$@ ttype=$ttype dpdaction=$dpdaction so exit"
             exit 0
        fi
    else
             logger -p local0.error -t "$me" "$@ ttype=$ttype so exit"
    fi
    exit 0
    ;;
#$2--up or down
#$3--tunnel name
#$4--primary or backup tunnel
s2s_trap)
    logger -p local0.error -t "$me" "$@"
    version=$(uci get snmpd.snmpd.NetWork_SNMP_Trap_Version)
    community=$(uci get snmpd.snmpd.NetWork_SNMP_Trap_Community_Name)
    remoteip=$(uci get snmpd.snmpd.NetWork_SNMP_Trap_Manage_Host)
    oid="1.3.6.1.4.1.21703.6000.100.6"
    vtype="s"
    myuser=$(uci get snmpd.snmpd.NetWork_SNMP_V3_User_Name)
    mypassword=$(uci get snmpd.snmpd.NetWork_SNMP_V3_Auth_Password)
    v3auth=$(uci get snmpd.snmpd.NetWork_SNMP_V3_User_Auth_Level)

    dpdaction=$(uci get ipsec.${3}.dpdaction)
    if [ $dpdaction = "restart_by_peer" ]; then
         case "$4" in
         s2stunnel)
             value="${3}(Primary)=$2"
             ;;
         s2stb)
             value="${3}(Secondary)=$2"
             /usr/sbin/ipsec auto --delete "$3" >/dev/null 2>&1
             ;;
         *)
             value=""
             ;;
         esac
    else
         value="$3=$2"
    fi
    logger -p local0.error -t "$me" "snmptrap $version $community $remoteip $oid $vtype $value $myuser $mypassword $v3auth"
    /bin/snmptrap.sh $version $community $remoteip $oid $vtype $value $myuser $mypassword $v3auth &
    exit 0
    ;;

###############################################################################################
#                 operations regarding to xl2tp client tunnel                                 #
###############################################################################################
#$2--tunnel name
add_x2c)
    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
    enabled=$(/sbin/uci get ipsec.${2}.enabled)
    [ "$enabled" = "1" ] && {
        echo "action=add" > /var/run/mipsec/$2
        echo "status=waiting for connection" >> /var/run/mipsec/$2
        echo "constat=Connect" >> /var/run/mipsec/$2
        interface=$(/sbin/uci get ipsec.${2}.interface)
        ipsec=$(/sbin/uci get ipsec.${2}.ipsec)
        if [ $interface = "br-wan2" ]
        then
            wan_status=$(/usr/lib/mipsec check_wan2)
        else
            wan_status=$(/usr/lib/mipsec check_wan)
        fi
        [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ wan_status=$wan_status interface=$interface ipsec=$ipsec"
        if [ "$wan_status" -eq 8 ]; then
              [ "$ipsec" = "enable" ] && /usr/sbin/ipsec auto --add $2 >/dev/null 2>&1
        fi
    }
    ;;

#$2--tunnel name
edit_x2c)
    # a flage indicates mifupdown ppp-ip-down proccess doesn't re-up this tunnel during editing
    up=$(cat /var/run/mipsec/${2} | grep -c 'inf=ppp')
    [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ pppup=$up"
    if [ $up -gt 0 ]; then
         touch /var/run/${2}.ctl
    fi
    #/usr/sbin/xl2tpd-control -c /var/run/xl2tpd/wan2-control remove "$2"
    #/usr/sbin/xl2tpd-control -c /var/run/xl2tpd/wan-control remove "$2"
    [ -p /var/run/xl2tpd/wan2-control ] && echo "r $2" > /var/run/xl2tpd/wan2-control
    [ -p /var/run/xl2tpd/wan-control ] && echo "r $2" > /var/run/xl2tpd/wan-control
    [ -p /var/run/xl2tpd/nwan2-control ] && echo "r $2" > /var/run/xl2tpd/nwan2-control
    [ -p /var/run/xl2tpd/nwan-control ] && echo "r $2" > /var/run/xl2tpd/nwan-control

    interface=$(/sbin/uci get ipsec.${2}.interface)
    
    if [ $interface = "br-wan2" ]
    then
         wan_status=$(/usr/lib/mipsec check_wan2)
    else
         wan_status=$(/usr/lib/mipsec check_wan)
    fi
    [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ wan_status=$wan_status interface=$interface"
    /usr/sbin/ipsec auto --delete "$2" >/dev/null 2>&1
    rm -f /var/run/mipsec/$2
    rm -f /etc/ppp/options.l2tpd.$2
    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
    enabled=$(/sbin/uci get ipsec.${2}.enabled)
    ipsec=$(/sbin/uci get ipsec.${2}.ipsec)
    [ "$enabled" = "1" ] && {
        echo "action=add" > /var/run/mipsec/$2
        echo "status=waiting for connection" >> /var/run/mipsec/$2
        echo "constat=Connect" >> /var/run/mipsec/$2
        if [ "$wan_status" -eq 8 ]; then
              [ "$ipsec" = "enable" ] && /usr/sbin/ipsec auto --add $2 >/dev/null 2>&1
        fi
    }
    ;;

#$2--tunnel name
#$3--interface
del_x2c)
    # a flage indicates mifupdown ppp-ip-down proccess doesn't re-up this tunnel during deleting
    up=$(cat /var/run/mipsec/${2} | grep -c 'inf=ppp')
    [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ pppup=$up"
    if [ $up -gt 0 ]; then
         pppid=$(cat /var/run/mipsec/${2} | grep 'inf=ppp' | cut -d'=' -f 2)
         touch /var/run/${2}.ctl
    fi

    interface=$3
    if [ $interface = "br-wan2" ]
    then
         #/usr/sbin/xl2tpd-control -c /var/run/xl2tpd/wan2-control remove "$2"
         [ -p /var/run/xl2tpd/wan2-control ] && echo "r $2" > /var/run/xl2tpd/wan2-control
         [ -p /var/run/xl2tpd/nwan2-control ] && echo "r $2" > /var/run/xl2tpd/nwan2-control
    else
         #/usr/sbin/xl2tpd-control -c /var/run/xl2tpd/wan-control remove "$2"
         [ -p /var/run/xl2tpd/wan-control ] && echo "r $2" > /var/run/xl2tpd/wan-control
         [ -p /var/run/xl2tpd/nwan-control ] && echo "r $2" > /var/run/xl2tpd/nwan-control
    fi
    [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ interface=$interface"
    /usr/sbin/ipsec auto --delete "$2" >/dev/null 2>&1
    rm -f /var/run/mipsec/$2
    rm -f /etc/ppp/options.l2tpd.$2
    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
   ;;

#$2--tunnel name
connect_x2c)
    interface=$(/sbin/uci get ipsec.${2}.interface)
    if [ $interface = "br-wan2" ]
    then
         wan_status=$(/usr/lib/mipsec check_wan2)
    else
         interface="br-wan"
         wan_status=$(/usr/lib/mipsec check_wan)
    fi
    ipsec=$(/sbin/uci get ipsec.${2}.ipsec)
    [ "$ipsec" = "disable" ] || ipsec="enable"
    [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ wan_status=$wan_status interface=$interface ipsec=$ipsec"
    case "$wan_status" in
    #1 -- interface down or ip_addr is 0.0.0.0
    1)
          echo "status=interface down" > /var/run/mipsec/$2
          echo "constat=N/A" >> /var/run/mipsec/$2
          echo "action=up" >> /var/run/mipsec/$2
          exit 0
          ;;
    #2 -- ipsec is not running
    #3 -- interface is not added into ipsec
    2|3)
          if [ "$ipsec" = "enable" ]; then
               /sbin/mifupdown restart >/dev/null 2>&1 &
          else
               /usr/lib/mipsec up_x2c $2 $interface >/dev/null 2>&1 &
          fi
          exit 0
          ;;
    #8 -- success
    8)
          /usr/lib/mipsec up_x2c $2 $interface >/dev/null 2>&1 &
          exit 0
          ;;
    *)
          logger -p local0.error -t "$me" "ERROR $@ wan_status=$wan_status interface=$interface"
          exit 0
          ;;
    esac
    exit 0
    ;;

#$2--tunnel name
disconnect_x2c)
    /usr/lib/mipsec down_x2c $2 &
    ;;

#$2--tunnel name
mre_x2c)
    cmd="/usr/sbin/ipsec auto --status | grep '$2' | grep -c 'refhim='"
    ipsecup=`eval $cmd`
    if [ $ipsecup -gt 0 ]; then
          cmd="/usr/sbin/ipsec auto --status | grep '$2' | grep 'refhim=' | cut -d'=' -f 2 | cut -d' ' -f 1"
          refme=`eval $cmd`
          cmd="/usr/sbin/ipsec auto --status | grep '$2' | grep 'refhim=' | cut -d'=' -f 3"
          refhim=`eval $cmd`
          logger -p local0.error -t "$me" "$@ refme=$refme refhim=$refhim"
          /sbin/mxl2tpd call_xl2tp $2 $refme $refhim &
          exit 0
    fi
    interface=$(/sbin/uci get ipsec.${2}.interface)
    if [ $interface = "br-wan2" ]
    then
         wan_status=$(/usr/lib/mipsec check_wan2)
    else
         interface="br-wan"
         wan_status=$(/usr/lib/mipsec check_wan)
    fi
    [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ wan_status=$wan_status interface=$interface"
    case "$wan_status" in
    #1 -- interface down or ip_addr is 0.0.0.0
    1)
          echo "status=interface down" > /var/run/mipsec/$2
          echo "constat=N/A" >> /var/run/mipsec/$2
          echo "action=up" >> /var/run/mipsec/$2
          exit 0
          ;;
    #2 -- ipsec is not running
    #3 -- interface is not added into ipsec
    2|3)
          /sbin/mifupdown restart >/dev/null 2>&1 &
          exit 0
          ;;
    #8 -- success
    8)
          /usr/lib/mipsec up_x2c $2 $interface >/dev/null 2>&1 &
          exit 0
          ;;
    *)
          logger -p local0.error -t "$me" "ERROR $@ wan_status=$wan_status interface=$interface"
          exit 0
          ;;
    esac
    exit 0
    ;;

###############################################################################################
#                 operations regarding to xl2tp server tunnel                                 #
###############################################################################################
#$2 -- interface
check_status_x2s)
    interface=$2
    if [ $interface = "br-wan2" ]
    then
         fp="/var/run/mipsec/M2xl2tpserver.status"
         ipsec=$(/sbin/uci get ipsec.M2xl2tpserver.ipsec)
         [ "$ipsec" = "disable" ] || ipsec="enable"
         if [ "$ipsec" = "enable" ]; then
         cmd="ipsec auto --status|grep -c 'M2xl2tpserver'"
         ctlfile="/var/run/xl2tpd/wan2-control"
         else
         ctlfile="/var/run/xl2tpd/nwan2-control"
         fi
    else
         fp="/var/run/mipsec/Mxl2tpserver.status"
         ipsec=$(/sbin/uci get ipsec.Mxl2tpserver.ipsec)
         [ "$ipsec" = "disable" ] || ipsec="enable"
         if [ "$ipsec" = "enable" ]; then
         cmd="ipsec auto --status|grep -c 'Mxl2tpserver'"
         ctlfile="/var/run/xl2tpd/wan-control"
         else
         ctlfile="/var/run/xl2tpd/nwan-control"
         fi
    fi

    if [ "$ipsec" = "enable" ]; then
    result=`eval $cmd`
    else
    result="1"
    fi

    if [ $result -gt 0 -a -p $ctlfile ]; then
           echo "status=started" > $fp
           echo "serstat=Stop" >> $fp
           echo "action=checked" >> $fp 
    else
           echo "status=stopped" > $fp
           echo "serstat=Start" >> $fp
           echo "action=checked" >> $fp 
    fi
    exit 0
    ;;

#$2 -- enable(1) or disable(0)
#$3 -- interface
update_x2s)
    interface=$3
    if [ $interface = "br-wan2" ]
    then
         wan_status=$(/usr/lib/mipsec check_wan2)
         fp="/var/run/mipsec/M2xl2tpserver.status"
         ipsec=$(/sbin/uci get ipsec.M2xl2tpserver.ipsec)
         [ "$ipsec" = "disable" ] || ipsec="enable"
    else
         interface="br-wan"
         wan_status=$(/usr/lib/mipsec check_wan)
         fp="/var/run/mipsec/Mxl2tpserver.status"
         ipsec=$(/sbin/uci get ipsec.Mxl2tpserver.ipsec)
         [ "$ipsec" = "disable" ] || ipsec="enable"
    fi
    [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ wan_status=$wan_status interface=$interface ipsec=$ipsec"
    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
    case "$wan_status" in
    #1 -- interface down or ip_addr is 0.0.0.0
    1)
          echo "status=interface down" > $fp
          echo "constat=N/A" >> $fp
          echo "action=up" >> $fp
          exit 0
          ;;
    #2 -- ipsec is not running
    #3 -- interface is not added into ipsec
    2|3)
          if [ "$ipsec" = "enable" ]; then
               /sbin/mifupdown restart >/dev/null 2>&1 &
          else
          if [ "$2" = "1" ]; then
               /usr/lib/mipsec enable_x2s $interface >/dev/null 2>&1
          else
               /usr/lib/mipsec disable_x2s $interface >/dev/null 2>&1
          fi
          fi
          exit 0
          ;;
    #8 -- success
    8)
          #/usr/lib/mipsec down_x2c_all $interface
          if [ "$2" = "1" ]; then
               /usr/lib/mipsec enable_x2s $interface >/dev/null 2>&1
          else
               /usr/lib/mipsec disable_x2s $interface >/dev/null 2>&1
          fi
          #/usr/lib/mipsec up_x2c_all $interface
          exit 0
          ;;
    *)
          logger -p local0.error -t "$me" "ERROR $@ wan_status=$wan_status interface=$interface"
          ;;
    esac
    ;;

###############################################################################################
#                 operations regarding to gre with ipsec tunnel                               #
###############################################################################################
#$2--tunnel name
add_grebgs)
    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
    echo "action=add" > /var/run/mipsec/$2.status
    echo "status=waiting for connection" >> /var/run/mipsec/$2.status
    echo "constat=Connect" >> /var/run/mipsec/$2.status
    interface=$(/sbin/uci get gre-tunnels.${2}.interface)
    if [ $interface = "br-wan2" ]
    then
            wan_status=$(/usr/lib/mipsec check_wan2)
    else
            wan_status=$(/usr/lib/mipsec check_wan)
    fi
    [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ wan_status=$wan_status interface=$interface"
    if [ "$wan_status" -eq 8 ]; then
              /usr/sbin/ipsec auto --add $2 >/dev/null 2>&1
    fi
    ;;

add_greags)
    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
    gre_status=$(/usr/lib/mipsec check_greinf $2)
    [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ gre_status=$gre_status"
    if [ "$gre_status" -eq 18 ]; then
              /usr/sbin/ipsec auto --add $2 >/dev/null 2>&1
    fi
    ;;

#$2 -- tunnel name
connect_gre)
    interface=$(/sbin/uci get gre-tunnels.$2.interface)
    if [ $interface = "br-wan2" ]
    then
         wan_status=$(/usr/lib/mipsec check_wan2)
    else
         wan_status=$(/usr/lib/mipsec check_wan)
    fi
    enableipsec=$(/sbin/uci get gre-tunnels.$2.enableipsec)
    ipsec_mode=$(/sbin/uci get gre-tunnels.$2.ipsec_mode)

    if [ $enableipsec = "ags" -a $ipsec_mode = "tunnel" ]
    then
         gre_status=$(/usr/lib/mipsec check_greinf $2)
    else
         gre_status="0"
    fi

    [ -f "/etc/debugipsec" ] && logger -p local0.error -t "$me" "$@ wan_status=$wan_status gre_status=$gre_status"
    case "$wan_status" in
    8)
          case "$gre_status" in
          12|13)
               gre_up=$(ifconfig -a $2 | grep -c 'UP')
               [ $gre_up != "1" ] && {
               /bin/gre add_gre $2 >/dev/null 2>&1
               /etc/init.d/firewall restart  >/dev/null 2>&1
               }
               /sbin/mifupdown restart >/dev/null 2>&1 &
               exit 0
               ;;
          18|0)
               /usr/lib/mipsec up $2 >/dev/null 2>&1 &
               exit 0
               ;;
          *)
               logger -p local0.error -t "$me" "ERROR $@ gre_status=$gre_status"
               exit 0
               ;;
          esac
          
          exit 0
          ;;
    1)
          echo "status=interface down" > /var/run/mipsec/$2.status
          echo "constat=N/A" >> /var/run/mipsec/$2.status
          echo "action=up" >> /var/run/mipsec/$2.status
          exit 0
          ;;
    2|3)
          /sbin/mifupdown restart >/dev/null 2>&1 &
          exit 0
          ;;
    *)
          logger -p local0.error -t "$me" "ERROR $@ wan_status=$wan_status gre_status=$gre_status"
          ;;
    esac
    ;;

disconnect_gre)
    /usr/sbin/ipsec auto --delete "$2" >/dev/null 2>&1
    ;;

*)
   ;;
esac 
exit 0

