#
# Copyright (C) 2009 OpenWrt.org
#

. /lib/ar71xx.sh
. /etc/version

PART_NAME=firmware
RAMFS_COPY_DATA=/lib/ar71xx.sh

CI_BLKSZ=262144
CI_LDADR=0x80060000
CI_KERNSZ=0x200000
CI_ROOTSZ=0xd80000
EXTRACT="/bin/extractmylofw"

KERNEL_BIN="/tmp/block0.bin"
ROOTFS_BIN="/tmp/block1.bin"

platform_find_partitions() {
        local first dev size erasesize name
        while read dev size erasesize name; do
                name=${name#'"'}; name=${name%'"'}
                case "$name" in
                        zImage|vmlinux.bin.l7|kernel|linux|rootfs)
                                if [ -z "$first" ]; then
                                        first="$name"
                                else
                                        echo "$erasesize:$first:$name"
                                        break
                                fi
                        ;;
                esac
        done < /proc/mtd
}

platform_find_kernelpart() {
        local part
        for part in "${1%:*}" "${1#*:}"; do
                case "$part" in
                        zImage|vmlinux.bin.l7|kernel|linux)
                                echo "$part"
                                break
                        ;;
                esac
        done
}



platform_check_image() {
        [ "$ARGC" -gt 1 ] && return 1
        echo -n "Checking image file..."
        if [ -x "$EXTRACT" ];
        then
            #VendorId=$(fw_printenv | grep $ENV_VENDOR | awk -F'[=]' '{print $2}')
            VendorId="MICROHARD"
            vendor_match=$($EXTRACT -V $1 | grep "Vendor:" | grep $VendorId)
            #ProductName=$(fw_printenv | grep $ENV_PRODUCTNAME | awk -F'[=]' '{print $2}')
            #ProductName="MPCI71"
            #productname_match=$($EXTRACT -V $1 | grep "Device Model:" | grep $ProductName)
	    productname_match=$($EXTRACT -V $1 | grep "Device Model:" | grep "$PRODUCT" |awk '{print $3}') 
            if [ -n "$vendor_match" ] && [ "$productname_match" = "$PRODUCT" ] ;
            then 
               Image_Path=$(dirname $1)
               cd "$Image_Path"
               $EXTRACT $1 >/dev/null 2>/dev/null
               return 0
            else
               echo -n "Invalid firmware, $PRODUCT for $VendorId firmware is required" 
               return 1
            fi

            return 0
        else
                echo -n "Firmware extracting utility not available"
                return 1
        fi

}

platform_do_upgrade() {

        local partitions=$(platform_find_partitions)
        local kernelpart=$(platform_find_kernelpart "${partitions#*:}")
        local erase_size=$((0x${partitions%%:*})); partitions="${partitions#*:}"


        echo -n "Upgrading..."

        #turn on the led 
        #echo "heartbeat" > /sys/class/leds/user/trigger
         /etc/init.d/ledcon stop
         echo "0" > /sys/class/leds/ledrx/brightness
         echo "0" > /sys/class/leds/ledtx/brightness

        if [ "$HARDWARE" = "v2.0.0" ]; then
            killall -9 ledcon.sh
            echo "THIS is VIP4G board $HARDWARE"
            echo "1" > /sys/class/leds/RSSIMIN/brightness
            echo "1" > /sys/class/leds/RSSIMID/brightness
            echo "1" > /sys/class/leds/RSSIMAX/brightness            
            /etc/ledcon.sh upgrade 2>/dev/null &
        else
            mpci-led -s 4 &
        fi
        

        if [ -e "$KERNEL_BIN" ] && [ -e "$ROOTFS_BIN" ] ;
        then

            if [ -n "$partitions" ] && [ -n "$kernelpart" ] ;
            then
                #echo -n "Eraseing jffs data partition..."
                    mtd erase rootfs_data
                local append=""
                if [ -f "$CONF_TAR" -a "$SAVE_CONFIG" -eq 1 ] ; then
                    mtd jffs2write $CONF_TAR rootfs_data
                elif [ -f "$CONF_TAR" -a "$SAVE_CARRIER" -eq 1 ] ;then
                    mtd jffs2write $CONF_TAR rootfs_data
                fi

                echo -n "Writing Kernel partition..."


                ( dd if="$KERNEL_BIN" bs=$CI_BLKSZ 2>/dev/null; ) | \
                        mtd  write - $kernelpart

                echo -n "Writing Rootfs partition..."

                ( dd if="$ROOTFS_BIN" bs=$CI_BLKSZ 2>/dev/null ) | \
                        mtd $append write - rootfs
            fi
        fi

        if [ "$HARDWARE" = "v2.0.0" ]; then
            killall -9 ledcon.sh
        else
            #turn off the led 
            #echo "none" > /sys/class/leds/user/trigger
            echo "0" > /sys/class/leds/ledrx/brightness
            echo "0" > /sys/class/leds/ledtx/brightness
            killall mpci-led
            mpci-led -s 0
        fi
}

disable_watchdog() {
        #stop keepalive when start upgrade firmware
        #echo 'Stop keepalive for upgrading system'
        keepalivebootup_pid=$(ps |grep keepalivebootup |grep -v grep |awk '{print $1}')
        if [ "$keepalivebootup_pid" = "" ] ;then
	    killall -9 S${START}keepalive 2>&- >&-
	    killall -9 keepalive 2>&- >&-
        else
            kill $keepalivebootup_pid 2>&- >&-
        fi

        #echo 'Stop traffic watchdog for upgrading system'
        #stop traffic watchdog timer to prevent system reboot 
        killall -9 S${START}twatchdog 2>&- >&-
        killall -9 t_watchdog 2>&- >&-
        
        #stop hardware watchdog timer to prevent system reboot 
	killall watchdog
	( ps | grep -v 'grep' | grep '/dev/watchdog' ) && {
		echo 'Could not disable watchdog'
		return 1
	}
        /etc/init.d/network stop
        killall -9 mpci-4g_server
        mpci -gpio set 8 0
        wifi down
}

append sysupgrade_pre_upgrade disable_watchdog
