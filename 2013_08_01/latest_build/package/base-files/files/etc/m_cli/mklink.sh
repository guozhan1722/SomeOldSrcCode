#!/bin/sh

#This is the common script for make link file automatically from config file

CONF_FILE=/etc/m_cli/cli.conf
COMMON_FILE=/etc/m_cli/common_uci_ctl.sh
LINK_DIR=/etc/m_cli

#find out system mode

if [ "`ifconfig br-lan 2>/dev/null`" != "" ]; then

   if [ "`ifconfig br-wan 2>/dev/null`" = "" ]; then
      sysmode=bridge
   else 
      sysmode=route
   fi

else
   echo "cannot find any network"
   exit -1
fi
   
#added base commands for CLI
if [ -f $CONF_FILE.base ];then
   cat $CONF_FILE.base > /tmp/cli.conf.tmp
else
   echo "cannot find base file "
   exit -1
fi

#added lan commands for CLI
if [ -f $CONF_FILE.lan ];then
   cat $CONF_FILE.lan >> /tmp/cli.conf.tmp
else
   echo "cannot find lan file "
   exit -1
fi

#added wan command if it is set to router mode
#if [ "$sysmode" = "route" ];then
#   if [ -f $CONF_FILE.wan ];then
#      cat $CONF_FILE.wan >> /tmp/cli.conf.tmp
#   else
#      echo "cannot find wan file "
#      exit -1
#  fi
#fi

#set cli.conf file for different version of hardware
. /etc/version

   case "$HARDWARE" in

   #nano PCI board wifi + ether ..
   "v1.0.0")

      if [ "$PRODUCT" = "IPnDDL" ] ;then
        cat $CONF_FILE.IPnDDL > /tmp/cli.conf.tmp
      else 
        sed 's/'"nVIP2400"'/'"`echo $PRODUCTNAME`"'/g' $CONF_FILE.IPnVT > $CONF_FILE.IPnVT_tmp

        cat $CONF_FILE.IPnVT_tmp > /tmp/cli.conf.tmp

      #else  
          #if [ -f $CONF_FILE.wifi0 ];then
          #   cat $CONF_FILE.wifi0 >> /tmp/cli.conf.tmp
          #else
          #   echo "cannot find wifi_0 file "
          #   exit -1
          #fi
    
          #if [ -f $CONF_FILE.firewall ];then
          #   cat $CONF_FILE.firewall >> /tmp/cli.conf.tmp
          #else
          #   echo "cannot find firewall file "
          #   exit -1
          #fi
      fi
   ;;

   #small VIP wifi0 + eth0 + com1
   #"3.0.0")
   #   if [ -f $CONF_FILE.wifi0 ];then
   #      cat $CONF_FILE.wifi0 >> /tmp/cli.conf.tmp
   #   else
   #      echo "cannot find wifi0 file "
   #      exit -1
   #   fi

   #   if [ -f $CONF_FILE.com1 ];then
   #      cat $CONF_FILE.com1 >> /tmp/cli.conf.tmp
   #   else
   #      echo "cannot find com1 file "
   #      exit -1
   #   fi

   #;;

   #*)
   #   echo "dont know hardware version!"
   #;;
   esac

   cp -f /tmp/cli.conf.tmp $CONF_FILE  
   rm -f /tmp/cli.conf.tmp

#make link file for set command interface
rm -f $LINK_DIR/set-*
rm -f $LINK_DIR/get-*

cat $CONF_FILE |grep "set-"|awk 'FS=":" {print $(NF-1)}'>/tmp/cmd_set
linecount_1=$(cat /tmp/cmd_set | wc -l | tr -d ' ')

while [ "0" -lt "$linecount_1" ]  ; do
   head -n $linecount_1 /tmp/cmd_set > /tmp/cmd_set1
   LINK_FILE=`tail -n 1 /tmp/cmd_set1`
   ln -sf $COMMON_FILE $LINK_DIR/$LINK_FILE
   linecount_1=`expr $linecount_1 - 1` 
done

rm /tmp/cmd_set
rm /tmp/cmd_set1

#make link file for get command interface

cat $CONF_FILE |grep "get-"|awk 'FS=":" {print $(NF-1)}'>/tmp/cmd_set
linecount_1=$(cat /tmp/cmd_set | wc -l | tr -d ' ')

while [ "0" -lt "$linecount_1" ]  ; do
   head -n $linecount_1 /tmp/cmd_set > /tmp/cmd_set1
   LINK_FILE=`tail -n 1 /tmp/cmd_set1`
   ln -sf $COMMON_FILE $LINK_DIR/$LINK_FILE
   linecount_1=`expr $linecount_1 - 1` 
done
rm /tmp/cmd_set
rm /tmp/cmd_set1

