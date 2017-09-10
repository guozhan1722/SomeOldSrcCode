#!/bin/sh

button_run_file=/var/run/button_count

. /etc/functions.sh
. /etc/version

COPY_FILES="/etc/firewall.config
/etc/firewall.user
/etc/httpd.conf
/etc/hosts
/etc/ethers
/etc/dhcp_ippbackup
/etc/passwd
/usr/lib/hardware_desc"

COPY_DIRS="/etc/config
/etc/openvpn
/etc/crontabs
/etc/ssl
/etc/dropbear"

reset_default () {

   logger "reseting the default value..."
   for file in $COPY_FILES; do
      rm $file 2>/dev/null
      [ -e /rom$file ] && [ ! -h /rom$file ] && {
      d=`dirname $file`; [ -d $d ] || mkdir -p $d
      cp -af /rom$file $file 2>/dev/null
      }
   done

   for dir in $COPY_DIRS; do
      [ -e $dir ] && {
	 rm $dir/* 2>/dev/null
	 cp -afr /rom$dir/* $dir/ 2>/dev/null
      }
   done

   logger "done. rebooting..."
   killall init

}

button_pressed () {
	local button
	local action
	local handler
	local min
	local max

	button="config"
	action="released"
        handler="reset_default"
        min=8
        max=30

	while [ -f $button_run_file ]; do
	   . $button_run_file
	   let count=$count+1
	   echo "count=$count" > $button_run_file
	   [ $min -le $count ] && eval $handler
	   sleep 1
	done
}

#config_load system
button_pressed
