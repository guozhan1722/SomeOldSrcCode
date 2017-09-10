. /etc/version

CI_BLKSZ=65536
CI_KERNSZ=0x200000
CI_LDADR=0x01600000
CI_ROOTSZ=0xa00000
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
	    #ProductName="$PRODUCT"
	    productname_match=$($EXTRACT -V $1 | grep "Device Model:" | grep "$PRODUCT" |awk '{print $3}') 

	    if [ -n "$vendor_match" ] && [ "$productname_match" = "$PRODUCT" ] ; 
	    then 
   	       Image_Path=$(dirname $1)
	       cd "$Image_Path"
	       $EXTRACT $1 >/dev/null 2>/dev/null
	       return 0
	    else
   	       echo -n "Invalid firmware, $ProductName for $VendorId firmware is required" 
	       return 1
	    fi

	    return 0
	else
		echo -n "Firmware extracting utility not available"
		return 1
	fi


#	case "$(get_magic_word "$1")" in
#		# Combined Image
#		4349)
#			local md5_img=$(dd if="$1" bs=2 skip=9 count=16 2>/dev/null)
#			local md5_chk=$(dd if="$1" bs=$CI_BLKSZ skip=1 2>/dev/null | md5sum -); md5_chk="${md5_chk%% *}"
#
#			if [ -n "$md5_img" -a -n "$md5_chk" ] && [ "$md5_img" = "$md5_chk" ]; then
#				return 0
#			else
#				echo "Invalid image. Contents do not match checksum (image:$md5_img calculated:$md5_chk)"
#				return 1
#			fi
#		;;
#		*)
#			echo "Invalid image. Use combined .img files on this platform"
#			return 1
#		;;
#	esac
}

platform_do_upgrade() {

	local partitions=$(platform_find_partitions)
	local kernelpart=$(platform_find_kernelpart "${partitions#*:}")
	local erase_size=$((0x${partitions%%:*})); partitions="${partitions#*:}"


	echo -n "Upgrading..."

	#turn on the led
	killall ledcon.sh
	if [ "$HARDWARE" = "5.0.0" -o "$HARDWARE" = "3.0.0" ]; then 
	        echo "heartbeat" > /sys/class/leds/phy0_rssi1/trigger
                echo "heartbeat" > /sys/class/leds/phy0_rssi2/trigger
                echo "heartbeat" > /sys/class/leds/phy0_rssi3/trigger
	else
	        echo "heartbeat" > /sys/class/leds/phy0_rssi1/trigger
                echo "heartbeat" > /sys/class/leds/phy0_rssi2/trigger
                echo "heartbeat" > /sys/class/leds/phy0_rssi3/trigger
	        echo "heartbeat" > /sys/class/leds/phy0_rssi4/trigger
                echo "heartbeat" > /sys/class/leds/phy0_rssi5/trigger
                echo "heartbeat" > /sys/class/leds/phy0_rssi6/trigger
	fi

	if [ -e "$KERNEL_BIN" ] && [ -e "$ROOTFS_BIN" ] ;
	then

	    #local kern_length=0x$(dd if="$KERNEL_BIN" bs=2 count=4 2>/dev/null) 
	    #local kern_blocks=$(($kern_length / $CI_BLKSZ))
	    #local root_blocks=$((0x$(dd if="$ROOTFS_BIN" bs=2 count=4 2>/dev/null) / $CI_BLKSZ))

	    if [ -n "$partitions" ] && [ -n "$kernelpart" ] ;
	    then
		local append=""
		[ -f "$CONF_TAR" -a "$SAVE_CONFIG" -eq 1 ] && append="-j $CONF_TAR"

		echo -n "Writing Kernel partition..."


		( dd if="$KERNEL_BIN" bs=$CI_BLKSZ 2>/dev/null; ) | \
			mtd -F$kernelpart:$CI_KERNSZ:$CI_LDADR write - $kernelpart

		echo -n "Writing Rootfs partition..."

		( dd if="$ROOTFS_BIN" bs=$CI_BLKSZ 2>/dev/null ) | \
			mtd $append -Frootfs:$CI_ROOTSZ write - rootfs 
	    fi 
	fi 

	#turn off the led 
        if [ "$HARDWARE" = "5.0.0" -o "$HARDWARE" = "3.0.0" ]; then 
                echo "none" > /sys/class/leds/phy0_rssi1/trigger
                echo "none" > /sys/class/leds/phy0_rssi2/trigger
                echo "none" > /sys/class/leds/phy0_rssi3/trigger
        else
                echo "none" > /sys/class/leds/phy0_rssi1/trigger
                echo "none" > /sys/class/leds/phy0_rssi2/trigger
                echo "none" > /sys/class/leds/phy0_rssi3/trigger
                echo "none" > /sys/class/leds/phy0_rssi4/trigger
                echo "none" > /sys/class/leds/phy0_rssi5/trigger
                echo "none" > /sys/class/leds/phy0_rssi6/trigger
	fi

}
