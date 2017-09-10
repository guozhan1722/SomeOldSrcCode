#!/bin/ash
 
. /etc/functions.sh

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
 
[ -f "/etc/debugipsec" ] && logger -t "mipsec.sh" "$1 $2"
 
case "$1" in 

del_s2s)
   ipsec auto --delete "$2" >/dev/null 2>&1 
   rm -f /var/run/mipsec/$2.status
   [ -f /var/run/ipsecfirewall/firewall.$2 ] && rm -f /var/run/ipsecfirewall/firewall.$2
   [ -f /var/run/ipsecfirewall/dfirewall.$2 ] && {
         sh /var/run/ipsecfirewall/dfirewall.$2
         rm -f /var/run/ipsecfirewall/dfirewall.$2
   }
   /etc/init.d/ipsec_vpn start >/dev/null 2>&1
   #ipsec auto --rereadsecrets >/dev/null 2>&1
   ipsec auto --rereadall >/dev/null 2>&1
   
   ;;

add_s2s)
    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
    #ipsec auto --rereadsecrets
    ipsec auto --rereadall >/dev/null 2>&1
    enabled=$(uci get ipsec.${2}.enabled)
    [ "$enabled" = "1" ] && {
        echo "action=add" > /var/run/mipsec/$2.status
        echo "status=waiting for connection" >> /var/run/mipsec/$2.status
        echo "constat=Connect" >> /var/run/mipsec/$2.status
        if [ -S "/var/run/pluto/pluto.ctl" ]; then
                 ipsec auto --add $2 >/dev/null 2>&1
        else
                 my_lockfile /var/lock/mipsec/ipsec_vpn && {
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh add_s2s $2" "my_lockfile ipsec_vpn success"
                    
                    ipsec setup restart >/dev/null 2>&1
                    rm -f /var/lock/mipsec/ipsec_vpn.lock
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh add_s2s $2" "delete ipsec_vpn.lock"
                 }
        fi
    }
    ;;

edit_s2s)
    ipsec auto --down "$2" >/dev/null 2>&1
    ipsec auto --delete "$2" >/dev/null 2>&1
    [ -f /var/run/ipsecfirewall/firewall.$t2 ] && rm -f /var/run/ipsecfirewall/firewall.$2
    [ -f /var/run/ipsecfirewall/dfirewall.$2 ] && {
         sh /var/run/ipsecfirewall/dfirewall.$2
         rm -f /var/run/ipsecfirewall/dfirewall.$2
    }
    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
    #ipsec auto --rereadsecrets
    ipsec auto --rereadall >/dev/null 2>&1
    enabled=$(uci get ipsec.${2}.enabled)
    [ "$enabled" = "1" ] && {
        echo "action=add" > /var/run/mipsec/$2.status
        echo "status=waiting for connection" >> /var/run/mipsec/$2.status
        echo "constat=Connect" >> /var/run/mipsec/$2.status
        if [ -S "/var/run/pluto/pluto.ctl" ]; then
                 ipsec auto --add $2 >/dev/null 2>&1
        else
                 my_lockfile /var/lock/mipsec/ipsec_vpn && {
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh edit_s2s $2" "my_lockfile ipsec_vpn success"
                    
                    ipsec setup restart >/dev/null 2>&1
                    rm -f /var/lock/mipsec/ipsec_vpn.lock
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh edit_s2s $2" "delete ipsec_vpn.lock"
                 }
        fi
    }
    ;;

connect_s2s)
    inf=$(uci get ipsec.${2}.interface)
    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_s2s $2" "inf=$inf"
    case $inf in
      "br-wan2")
          left=$(/sbin/uci -P /var/state get network.wan2.up)
          if [ $left = "1" ]; then
            left=$(/sbin/uci -P /var/state get network.wan2.ipaddr)
          else
            left="x"
          fi
          cmd="ipsec auto --status|grep -c 'interface br-wan2/br-wan2 $left'"
          ;;
      "br-wan")
          left=$(/sbin/uci -P /var/state get network.wan.up)
          if [ $left = "1" ]; then
            left=$(/sbin/uci -P /var/state get network.wan.ipaddr)
          else
            left="x"
          fi
          cmd="ipsec auto --status|grep -c 'interface br-wan/br-wan $left'"
          ;;
    esac

        # if there's no WAN IP, get out of here
    if [ "$left" = "x" ]; then
          logger -t "mipsec.sh connect_s2s $2" "WAN IP is NULL. exit." 
          echo "status=waiting for connection" > /var/run/mipsec/$2.status
          echo "constat=Connect" >> /var/run/mipsec/$2.status
          echo "action=up" >> /var/run/mipsec/$2.status
          exit 0
    fi
    
    [ -d "/var/lock/mipsec" ] || mkdir /var/lock/mipsec
    my_lockfile /var/lock/mipsec/"$2" || {
          [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_s2s $2" "my_lockfile failed and exist connection"
          echo "status=waiting for connection" > /var/run/mipsec/$2.status
          echo "constat=Connect" >> /var/run/mipsec/$2.status
          echo "action=up" >> /var/run/mipsec/$2.status
          exit 0
    }
    [ -S "/var/run/pluto/pluto.ctl" ] || {
                 my_lockfile /var/lock/mipsec/ipsec_vpn && {
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_s2s $2" "my_lockfile ipsec_vpn success"
                    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
                    ipsec setup restart >/dev/null 2>&1
                    rm -f /var/lock/mipsec/ipsec_vpn.lock
                    sleep 2
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_s2s $2" "delete ipsec_vpn.lock"
                 }
    }
    sleep 1
    bwan=`eval $cmd`
    [ "$bwan" -eq 0 ] && {
                 logger -t "mipsec.sh connect_s2s $2" "ipsec started but works incorrectly because interface $inf isn't added by ipsec, restart ipsec"
                 my_lockfile /var/lock/mipsec/ipsec_vpn && {
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_s2s $2" "my_lockfile ipsec_vpn success"
                    ipsec setup restart >/dev/null 2>&1
                    rm -f /var/lock/mipsec/ipsec_vpn.lock
                    sleep 2
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_s2s $2" "delete ipsec_vpn.lock"
                 }
    }
    bwan=`eval $cmd`
    [ "$bwan" -eq 0 ] && {
             logger -t "mipsec.sh connect_s2s $2" "ipsec started but works incorrectly because interface $inf isn't added by ipsec, exit connecting"
             rm -rf /var/lock/mipsec/"$2".lock
             [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_s2s $2" "delete lock file"
             echo "status=waiting for connection" > /var/run/mipsec/$2.status
             echo "constat=Connect" >> /var/run/mipsec/$2.status
             echo "action=up" >> /var/run/mipsec/$2.status
             exit 0
    }
    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_s2s" "ipsec up $2"
    cmd="ipsec auto --status|grep -c '\"$2\":500'"
    result=`eval $cmd`
    [ $result -gt 0 ] && ipsec auto --delete "$2"
    #ipsec auto --rereadsecrets
    ipsec auto --rereadall >/dev/null 2>&1
    echo "status=waiting for connection" > /var/run/mipsec/$2.status
    echo "constat=Waiting..." >> /var/run/mipsec/$2.status
    echo "action=up" >> /var/run/mipsec/$2.status
    ipsec auto --add "$2"
    ipsec auto --asynchronous --up $2 &
    rm -rf /var/lock/mipsec/"$2".lock
    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_s2s" "ipsec up $2 delete lock file"
    ;;

disconnect_s2s)
    
    [ -d "/var/lock/mipsec" ] || mkdir /var/lock/mipsec
    my_lockfile /var/lock/mipsec/"$2" || {
    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh disconnect_s2s $2" "my_lockfile failed and exist connection"
    exit 0
    }
    ipsec auto --down "$2"
    
    rm -rf /var/lock/mipsec/"$2".lock
    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh disconnect_s2s $2" "ipsec delete $2 delete lock file"
    ;;

check_status_s2s)
   fp="/var/run/mipsec/${2}.status"
   status=`cat $fp | grep status | cut -d '=' -f 2`
   [ $status = "disable" ] && exit 0
   cmd="ipsec auto --status|grep 'IPsec SA established'|grep -c '\"$2\"'"
             result=`eval $cmd`
             if [ $result -gt 0 ]; then
                 echo "action=checked" > $fp
                 echo "status=connected" >> $fp
                 echo "constat=Disconnect" >> $fp
             else

                      echo "action=checked" > $fp
                      echo "status=waiting for connection" >> $fp
                      echo "constat=Connect" >> $fp
             fi
   ;;

check_status_x2s)
   fp="/var/run/mipsec/Mxl2tpserver.status"
   status=`cat $fp | grep status | cut -d '=' -f 2`
   [ $status = "disable" ] && exit 0
   inf=$(uci get ipsec.Mxl2tpserver.interface)

   case $inf in
      "br-wan2")
          wip=$(/sbin/uci -P /var/state get network.wan2.up)
          if [ $wip = "1" ]; then
            wip=$(/sbin/uci -P /var/state get network.wan2.ipaddr)
          else
            wip="x"
          fi
          ;;
      "br-wan")
          wip=$(/sbin/uci -P /var/state get network.wan.up)
          if [ $wip = "1" ]; then
            wip=$(/sbin/uci -P /var/state get network.wan.ipaddr)
          else
            wip="x"
          fi
          ;;
   esac
   [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh check_status_x2s" "inf=$inf wip=$wip"
   [ $wip = "x" ] && {
          echo "action=shutdown" > /var/run/mipsec/Mxl2tpserver.status
          echo "status=interface down" >> /var/run/mipsec/Mxl2tpserver.status
          echo "serstat=N/A" >> /var/run/mipsec/Mxl2tpserver.status
          exit 0
   }
   [ -p /var/run/xl2tpd/l2tp-control ] || /etc/init.d/xl2tpd restart > /dev/null 2>&1
   cmd="ipsec auto --status|grep -c 'Mxl2tpserver\": $wip'"
   result=`eval $cmd`
   result2=$(ps|grep -v grep|grep -c xl2tpd)
   [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh check_status_x2s" "result=$result result2=$result2"
   let xlserver=result+result2
   if [ $xlserver -gt 1 ]; then
         echo "status=started" > $fp
         echo "serstat=Stop" >> $fp 
         echo "action=checked" >> $fp 
   else 
         echo "serstat=Waiting..." > $fp
         echo "status=waiting for start" >> $fp
         echo "action=check" >> $fp
   fi
   if [ $result -eq 0 ]; then
         if [ -S "/var/run/pluto/pluto.ctl" ]; then
                 [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh check_status_x2s" "add conns"
                 #ipsec auto --rereadsecrets
                 ipsec auto --rereadall >/dev/null 2>&1
                 ipsec auto --add Mxl2tpserver
                 ipsec auto --add roadwarrior-l2tp
                 ipsec auto --add macintosh-l2tp
                 ipsec auto --add roadwarrior-l2tp-updatedwin
         else
                 [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh check_status_x2s" "ipsec restart"
                 ipsec setup restart >/dev/null 2>&1

         fi
    fi
   ;;

update_vca)
    uci_load ipsec
    cnt="1"
    [ -z $vca_cfgs ] && {
             [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh update_vca" "vca_cfgs is empty"
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
        [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh update_vca" "write $vca * $vca_pwd *"
        let "cnt+=1"
    done
    }
    ;;    

update_x2s)
   fp="/var/run/mipsec/Mxl2tpserver.status"
   /etc/init.d/ipsec_vpn start >/dev/null 2>&1
   if [ "$2" = "1" ]; then
         inf=$(uci get ipsec.Mxl2tpserver.interface)
         uci_load ipsec
         for x2c in $x2c_cfgs; do
           config_get x2c_enable $x2c enabled
           if [ $x2c_enable = "1" ]; then
              [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh update_x2s" "down x2ctunnel $x2c"
              /usr/sbin/xl2tpd-control remove $x2c
              ipsec auto --down "$x2c"
              ipsec auto --delete "$x2c"
              rm -f /var/log/mipsec.$x2c
              echo "action=checked" > /var/run/mipsec/$x2c
              echo "status=waiting for connection" >> /var/run/mipsec/$x2c
              echo "constat=Connect" >> /var/run/mipsec/$x2c
           fi
         done

         [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh update_x2s" "enable server"
         
         if [ -S "/var/run/pluto/pluto.ctl" ]; then
                 [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh update_x2s" "add conns"
                 ipsec auto --delete Mxl2tpserver
                 ipsec auto --delete roadwarrior-l2tp
                 ipsec auto --delete macintosh-l2tp
                 ipsec auto --delete roadwarrior-l2tp-updatedwin
                 #ipsec auto --rereadsecrets
                 ipsec auto --rereadall >/dev/null 2>&1
                 ipsec auto --add Mxl2tpserver
                 ipsec auto --add roadwarrior-l2tp
                 ipsec auto --add macintosh-l2tp
                 ipsec auto --add roadwarrior-l2tp-updatedwin
         else
                 [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh update_x2s" "ipsec restart"
                 ipsec setup restart >/dev/null 2>&1

         fi
         /etc/init.d/xl2tpd restart
         echo "action=start" > $fp
         echo "status=waiting for start" >> $fp
         echo "serstat=Waiting..." >> $fp
   else
         [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh update_x2s" "disable server"
         /etc/init.d/xl2tpd stop
         if [ -S "/var/run/pluto/pluto.ctl" ]; then
              [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh update_x2s" "delete conns"
              ipsec auto --delete Mxl2tpserver
              ipsec auto --delete roadwarrior-l2tp
              ipsec auto --delete macintosh-l2tp
              ipsec auto --delete roadwarrior-l2tp-updatedwin
         fi
         echo "action=checked" > $fp
         echo "status=disable" >> $fp
         echo "serstat=N/A" >> $fp
    fi
    ;;

add_x2c)
    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
    #ipsec auto --rereadsecrets
    ipsec auto --rereadall >/dev/null 2>&1
    enabled=$(uci get ipsec.${2}.enabled)
    [ "$enabled" = "1" ] && {
        echo "action=add" > /var/run/mipsec/$2
        echo "status=waiting for connection" >> /var/run/mipsec/$2
        echo "constat=Connect" >> /var/run/mipsec/$2
        if [ -S "/var/run/pluto/pluto.ctl" ]; then
                 ipsec auto --add $2 >/dev/null 2>&1
        else
                 my_lockfile /var/lock/mipsec/ipsec_vpn && {
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh add_x2c $2" "my_lockfile ipsec_vpn success"
                    
                    ipsec setup restart >/dev/null 2>&1
                    rm -f /var/lock/mipsec/ipsec_vpn.lock
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh add_x2c $2" "delete ipsec_vpn.lock"
                 }
        fi
    }
   ;;

edit_x2c)
    /usr/sbin/xl2tpd-control remove $2
    ipsec auto --delete "$2" >/dev/null 2>&1
    sleep 1
    plist=$(ps -ef | grep xlpconnect | grep -v grep | grep ${2} | awk '{print $1}')
    logger -t "mipsec.sh" "edit_x2c plist=$plist"
    for pl in $plist
    do
       kill -9 $pl
    done
    rm -f /var/run/mipsec/$2
    rm -f /var/log/mipsec.$2
    rm -f /etc/ppp/options.l2tpd.$2
    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
    #ipsec auto --rereadsecrets
    ipsec auto --rereadall >/dev/null 2>&1
    enabled=$(uci get ipsec.${2}.enabled)
    [ "$enabled" = "1" ] && {
        echo "action=add" > /var/run/mipsec/$2
        echo "status=waiting for connection" >> /var/run/mipsec/$2
        echo "constat=Connect" >> /var/run/mipsec/$2
        if [ -S "/var/run/pluto/pluto.ctl" ]; then
                 ipsec auto --add $2 >/dev/null 2>&1
        else
                 my_lockfile /var/lock/mipsec/ipsec_vpn && {
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh edit_x2c $2" "my_lockfile ipsec_vpn success"
                    
                    ipsec setup restart >/dev/null 2>&1
                    rm -f /var/lock/mipsec/ipsec_vpn.lock
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh edit_x2c $2" "delete ipsec_vpn.lock"
                 }
        fi
    }
   ;;

del_x2c)
   /usr/sbin/xl2tpd-control remove $2
   ipsec auto --delete "$2" >/dev/null 2>&1 
   /etc/init.d/ipsec_vpn start >/dev/null 2>&1
   #ipsec auto --rereadsecrets
   ipsec auto --rereadall >/dev/null 2>&1
   rm -f /var/run/mipsec/$2
   rm -f /var/log/mipsec.$2
   rm -f /etc/ppp/options.l2tpd.$2
   plist=$(ps -ef | grep xlpconnect | grep -v grep | grep ${2} | awk '{print $1}')
   logger -t "mipsec.sh" "del_x2c plist=$plist"
   for pl in $plist
   do
       kill -9 $pl
   done
   ;;

connect_x2c)
   plist=$(ps -ef | grep xlpconnect | grep -v grep | grep ${2} | awk '{print $1}')
   logger -t "mipsec.sh connect_x2c $2" "plist=$plist"
   for pl in $plist
   do
       kill -9 $pl
   done
   rm -f /var/lock/mipsec/${2}.lock
   [ -f "/var/run/${2}.ctl" ] && rm -f /var/run/${2}.ctl

    inf=$(uci get ipsec.${2}.interface)
    logger -t "mipsec.sh connect_x2c $2" "inf=$inf"
    case $inf in
      "br-wan2")
          left=$(/sbin/uci -P /var/state get network.wan2.up)
          if [ $left = "1" ]; then
            left=$(/sbin/uci -P /var/state get network.wan2.ipaddr)
          else
            left="x"
          fi
          cmd="ipsec auto --status|grep -c 'interface br-wan2/br-wan2 $left'"
          ;;
      "br-wan")
          left=$(/sbin/uci -P /var/state get network.wan.up)
          if [ $left = "1" ]; then
            left=$(/sbin/uci -P /var/state get network.wan.ipaddr)
          else
            left="x"
          fi
          cmd="ipsec auto --status|grep -c 'interface br-wan/br-wan $left'"
          ;;
    esac

        # if there's no WAN IP, get out of here
    if [ "$left" = "x" ]; then
          logger -t "mipsec.sh connect_x2c $2" "WAN IP is NULL. exit." 
          echo "action=up" > /var/run/mipsec/$2
          echo "status=waiting for connection" >> /var/run/mipsec/$2
          echo "constat=Connect" >> /var/run/mipsec/$2
          exit 0
    fi

    [ -d "/var/lock/mipsec" ] || mkdir /var/lock/mipsec

    [ -S "/var/run/pluto/pluto.ctl" ] || {
                 my_lockfile /var/lock/mipsec/ipsec_vpn && {
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_x2c $2" "my_lockfile ipsec_vpn success"
                    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
                    ipsec setup restart >/dev/null 2>&1
                    rm -f /var/lock/mipsec/ipsec_vpn.lock
                    sleep 2
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_x2c $2" "delete ipsec_vpn.lock"
                 }
    }
    sleep 1
    bwan=`eval $cmd`
    [ "$bwan" -eq 0 ] && {
                 logger -t "mipsec.sh connect_x2c $2" "ipsec started but works incorrectly because interface $inf isn't added by ipsec, restart ipsec"
                 my_lockfile /var/lock/mipsec/ipsec_vpn && {
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_x2c $2" "my_lockfile ipsec_vpn success"
                    ipsec setup restart >/dev/null 2>&1
                    rm -f /var/lock/mipsec/ipsec_vpn.lock
                    sleep 2
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_x2c $2" "delete ipsec_vpn.lock"
                 }
    }
    bwan=`eval $cmd`
    [ "$bwan" -eq 0 ] && {
             logger -t "mipsec.sh connect_x2c $2" "ipsec started but works incorrectly because interface $inf isn't added by ipsec, exit connecting"
             echo "action=up" > /var/run/mipsec/$2
             echo "status=waiting for connection" >> /var/run/mipsec/$2
             echo "constat=Connect" >> /var/run/mipsec/$2
             exit 0
    }
    if [ ! -p /var/run/xl2tpd/l2tp-control ]; then
	 /etc/init.d/xl2tpd restart >/dev/null 2>&1
    fi
    tname="$2"
    echo "action=up" > /var/run/mipsec/$tname
    echo "status=waiting for ipsec connection" >> /var/run/mipsec/$tname
    echo "constat=Waiting..." >> /var/run/mipsec/$tname
    ipsec auto --delete "$tname"
    /sbin/xlpconnect $tname 0 &
    ;;

call_xl2tp)
    tname="$2"
    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh call_xl2tp" "up $2"
    [ -p "/var/run/xl2tpd/l2tp-control" ] || /etc/init.d/xl2tpd restart >/dev/null 2>&1
    opts=$(cat /etc/xl2tpclient/${tname}.conf)
    /usr/sbin/xl2tpd-control add $tname $opts
    /usr/sbin/xl2tpd-control connect $tname
    ;;
   
disconnect_x2c)
    #touch /var/run/${2}.ctl
   plist=$(ps -ef | grep xlpconnect | grep -v grep | grep ${2} | awk '{print $1}')
   logger -t "mipsec.sh disconnect_x2c $2" "plist=$plist"
   for pl in $plist
   do
       kill -9 $pl
   done
   rm -f /var/lock/mipsec/${2}.lock
   touch /var/run/${2}.ctl
    /usr/sbin/xl2tpd-control remove $2
    ipsec auto --down "$2"
    ipsec auto --delete "$2"
    rm -f /var/log/mipsec.$2
    echo "action=checked" > /var/run/mipsec/$2
    echo "status=waiting for connection" >> /var/run/mipsec/$2
    echo "constat=Connect" >> /var/run/mipsec/$2
    ;;

connect_gre)
    inf=$(uci get gre-tunnels.${2}.interface)
    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_gre $2" "inf=$inf"
    case $inf in
      "br-wan2")
          left=$(/sbin/uci -P /var/state get network.wan2.up)
          if [ $left = "1" ]; then
            left=$(/sbin/uci -P /var/state get network.wan2.ipaddr)
          else
            left="x"
          fi
          cmd="ipsec auto --status|grep -c 'interface br-wan2/br-wan2 $left'"
          ;;
      "br-wan")
          left=$(/sbin/uci -P /var/state get network.wan.up)
          if [ $left = "1" ]; then
            left=$(/sbin/uci -P /var/state get network.wan.ipaddr)
          else
            left="x"
          fi
          cmd="ipsec auto --status|grep -c 'interface br-wan/br-wan $left'"
          ;;
       *)
           left="x"
           ;;
    esac

        # if there's no WAN IP, get out of here
    if [ "$left" = "x" ]; then
          logger -t "mipsec.sh connect_gre $2" "WAN IP is NULL. exit." 
          echo "status=waiting for connection" > /var/run/mipsec/$2.status
          echo "constat=Connect" >> /var/run/mipsec/$2.status
          echo "action=up" >> /var/run/mipsec/$2.status
          exit 0
    fi
    
    [ -d "/var/lock/mipsec" ] || mkdir /var/lock/mipsec
    my_lockfile /var/lock/mipsec/"$2" || {
          [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_gre $2" "my_lockfile failed and exist connection"
          echo "status=waiting for connection" > /var/run/mipsec/$2.status
          echo "constat=Connect" >> /var/run/mipsec/$2.status
          echo "action=up" >> /var/run/mipsec/$2.status
          exit 0
    }
    [ -S "/var/run/pluto/pluto.ctl" ] || {
                 my_lockfile /var/lock/mipsec/ipsec_vpn && {
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_gre $2" "my_lockfile ipsec_vpn success"
                    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
                    ipsec setup restart >/dev/null 2>&1
                    rm -f /var/lock/mipsec/ipsec_vpn.lock
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_gre $2" "delete ipsec_vpn.lock"
                 }
    }
    sleep 1
    bwan=`eval $cmd`
    [ "$bwan" -eq 0 ] && {
             logger -t "mipsec.sh connect_gre $2" "ipsec started but works incorrectly because interface $inf isn't added by ipsec, exit connecting"
             rm -rf /var/lock/mipsec/"$2".lock
             [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_gre $2" "delete lock file"
             echo "status=waiting for connection" > /var/run/mipsec/$2.status
             echo "constat=Connect" >> /var/run/mipsec/$2.status
             echo "action=up" >> /var/run/mipsec/$2.status
             exit 0
    }
    enableipsec=$(uci get gre-tunnels.${2}.enableipsec)
    local_tunnel_ip=$(uci get gre-tunnels.${2}.local_tunnel_ip)
    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_gre $2" "enableipsec=$enableipsec local_tunnel_ip=$local_tunnel_ip"
    if [ "$enableipsec" = "ags" ]; then
    cmd="ipsec auto --status|grep 'interface'|grep -c '$local_tunnel_ip'"
    bwan=`eval $cmd`
    [ "$bwan" -eq 0 ] && {
                 my_lockfile /var/lock/mipsec/ipsec_vpn && {
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_gre $2" "my_lockfile ipsec_vpn success"
                    /etc/init.d/ipsec_vpn start >/dev/null 2>&1
                    ipsec setup restart >/dev/null 2>&1
                    rm -f /var/lock/mipsec/ipsec_vpn.lock
                    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_gre $2" "delete ipsec_vpn.lock"
                 }
                 sleep 2
    }
    bwan=`eval $cmd`
    [ "$bwan" -eq 0 ] && {
             logger -t "mipsec.sh connect_gre $2" "ipsec restarted but works incorrectly because interface $2 isn't added by ipsec, exit connecting"
             rm -rf /var/lock/mipsec/"$2".lock
             [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_gre $2" "delete lock file"
             echo "status=waiting for connection" > /var/run/mipsec/$2.status
             echo "constat=Connect" >> /var/run/mipsec/$2.status
             echo "action=up" >> /var/run/mipsec/$2.status
             exit 0
    }
    fi
    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_gre" "ipsec up $2"
    cmd="ipsec auto --status|grep -c '\"$2\"'"
    result=`eval $cmd`
    [ $result -gt 0 ] && ipsec auto --delete "$2"
    #ipsec auto --rereadsecrets
    ipsec auto --rereadall >/dev/null 2>&1
    echo "status=waiting for connection" > /var/run/mipsec/$2.status
    echo "constat=Waiting..." >> /var/run/mipsec/$2.status
    echo "action=up" >> /var/run/mipsec/$2.status
    ipsec auto --add "$2"
    ipsec auto --asynchronous --up $2 &
    rm -rf /var/lock/mipsec/"$2".lock
    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh connect_gre" "ipsec up $2 delete lock file"
    ;;

disconnect_gre)
    
    [ -d "/var/lock/mipsec" ] || mkdir /var/lock/mipsec
    my_lockfile /var/lock/mipsec/"$2" || {
    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh disconnect_gre $2" "my_lockfile failed and exist connection"
    exit 0
    }
    ipsec auto --delete "$2"

    rm -rf /var/lock/mipsec/"$2".lock
    [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh disconnect_gre $2" "ipsec delete $2 delete lock file"
    ;;

shutdown_wan)
    uci_load ipsec
    for s2s in $s2s_cfgs; do
        config_get s2s_inf $s2s interface
        config_get s2s_enable $s2s enabled
        if [ $s2s_enable = "1" ]; then
           if [ $s2s_inf = "br-wan" ]; then
              [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh shutdown_wan" "down s2stunnel $s2s"
              ipsec auto --down $s2s
              ipsec auto --delete $s2s
              echo "status=interface down" > /var/run/mipsec/${s2s}.status
              echo "constat=N/A" >> /var/run/mipsec/${s2s}.status
              echo "action=checked" >> /var/run/mipsec/${s2s}.status
           fi
        fi
    done
    for x2c in $x2c_cfgs; do
        config_get x2c_inf $x2c interface
        config_get x2c_enable $x2c enabled
        plist=$(ps -ef | grep xlpconnect | grep -v grep | grep ${x2c} | awk '{print $1}')
        logger -t "mipsec.sh shutdown_wan" "$x2c plist=$plist"
        for pl in $plist
        do
            kill -9 $pl
        done
        if [ $x2c_enable = "1" ]; then
           if [ $x2c_inf = "br-wan" ]; then
              [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh shutdown_wan" "down x2ctunnel $x2c"
              /usr/sbin/xl2tpd-control remove $x2c
              ipsec auto --down "$x2c"
              ipsec auto --delete "$x2c"
              rm -f /var/log/mipsec.$x2c
              echo "action=checked" > /var/run/mipsec/$x2c
              echo "status=interface down" >> /var/run/mipsec/$x2c
              echo "constat=N/A" >> /var/run/mipsec/$x2c
           fi
        fi
    done
    for x2s in $x2s_cfgs; do
        config_get x2s_inf $x2s interface
        config_get x2s_enable $x2s enabled
        if [ $x2s_enable = "1" ]; then
           if [ $x2s_inf = "br-wan" ]; then
              [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh shutdown_wan" "down x2stunnel $x2s"
              /etc/init.d/xl2tpd stop
              if [ -S "/var/run/pluto/pluto.ctl" ]; then
                 [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh shutdown_wan" "delete conns"
                 ipsec auto --delete Mxl2tpserver
                 ipsec auto --delete roadwarrior-l2tp
                 ipsec auto --delete macintosh-l2tp
                 ipsec auto --delete roadwarrior-l2tp-updatedwin
              fi
              echo "action=shutdown" > /var/run/mipsec/Mxl2tpserver.status
              echo "status=interface down" >> /var/run/mipsec/Mxl2tpserver.status
              echo "constat=N/A" >> /var/run/mipsec/Mxl2tpserver.status
           fi
        fi
    done
    uci_load gre-tunnels
    for gre in $gre_cfgs; do
                 config_get local_status  $gre local_status
                 config_get enableipsec  $gre enableipsec
                 config_get gre_inf $gre interface
                 if [ "$local_status" = "Disable" ]; then
                     continue
                 fi
                 if [ "$enableipsec" = "Disable" ]; then
                     continue
                 fi
                 if [ "$gre_inf" = "br-wan2" ]; then
                     continue
                 fi
                 [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh" "shutdown_wan shutdown gretunnel $gre"
              ipsec auto --down $gre
              ipsec auto --delete $gre
              echo "status=interface down" > /var/run/mipsec/${gre}.status
              echo "constat=N/A" >> /var/run/mipsec/${gre}.status
              echo "action=checked" >> /var/run/mipsec/${gre}.status
    done    
    ;;

shutdown_wan2)
    uci_load ipsec
    for s2s in $s2s_cfgs; do
        config_get s2s_inf $s2s interface
        config_get s2s_enable $s2s enabled
        if [ $s2s_enable = "1" ]; then
           if [ $s2s_inf = "br-wan2" ]; then
              [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh shutdown_wan2" "down s2stunnel $s2s"
              ipsec auto --down $s2s
              ipsec auto --delete $s2s
              echo "status=interface down" > /var/run/mipsec/${s2s}.status
              echo "constat=N/A" >> /var/run/mipsec/${s2s}.status
              echo "action=checked" >> /var/run/mipsec/${s2s}.status
           fi
        fi
    done
    for x2c in $x2c_cfgs; do
        config_get x2c_inf $x2c interface
        config_get x2c_enable $x2c enabled
        plist=$(ps -ef | grep xlpconnect | grep -v grep | grep ${x2c} | awk '{print $1}')
        logger -t "mipsec.sh shutdown_wan2" "$x2c plist=$plist"
        for pl in $plist
        do
            kill -9 $pl
        done
        if [ $x2c_enable = "1" ]; then
           if [ $x2c_inf = "br-wan2" ]; then
              [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh shutdown_wan2" "down x2ctunnel $x2c"
              /usr/sbin/xl2tpd-control remove $x2c
              ipsec auto --down "$x2c"
              ipsec auto --delete "$x2c"
              rm -f /var/log/mipsec.$x2c
              echo "action=checked" > /var/run/mipsec/$x2c
              echo "status=interface down" >> /var/run/mipsec/$x2c
              echo "constat=N/A" >> /var/run/mipsec/$x2c
           fi
        fi
    done
    for x2s in $x2s_cfgs; do
        config_get x2s_inf $x2s interface
        config_get x2s_enable $x2s enabled
        if [ $x2s_enable = "1" ]; then
           if [ $x2s_inf = "br-wan2" ]; then
              [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh shutdown_wan2" "down x2stunnel $x2s"
              /etc/init.d/xl2tpd stop
              if [ -S "/var/run/pluto/pluto.ctl" ]; then
                 [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh shutdown_wan" "delete conns"
                 ipsec auto --delete Mxl2tpserver
                 ipsec auto --delete roadwarrior-l2tp
                 ipsec auto --delete macintosh-l2tp
                 ipsec auto --delete roadwarrior-l2tp-updatedwin
              fi
              echo "action=shutdown" > /var/run/mipsec/Mxl2tpserver.status
              echo "status=interface down" >> /var/run/mipsec/Mxl2tpserver.status
              echo "constat=N/A" >> /var/run/mipsec/Mxl2tpserver.status
           fi
        fi
    done
    uci_load gre-tunnels
    for gre in $gre_cfgs; do
                 config_get local_status  $gre local_status
                 config_get enableipsec  $gre enableipsec
                 config_get gre_inf $gre interface
                 if [ "$local_status" = "Disable" ]; then
                     continue
                 fi
                 if [ "$enableipsec" = "Disable" ]; then
                     continue
                 fi
                 if [ "$gre_inf" = "br-wan" ]; then
                     continue
                 fi
                 [ -f "/etc/debugipsec" ] && logger -t "mipsec.sh" "shutdown_wan2 shutdown gretunnel $gre"
              ipsec auto --down $gre
              ipsec auto --delete $gre
              echo "status=interface down" > /var/run/mipsec/${gre}.status
              echo "constat=N/A" >> /var/run/mipsec/${gre}.status
              echo "action=checked" >> /var/run/mipsec/${gre}.status
    done 
    ;;

*)
    exit 1
   ;;
esac 
exit 0

