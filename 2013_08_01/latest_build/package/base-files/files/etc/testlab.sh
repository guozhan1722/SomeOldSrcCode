#!/bin/sh
. /etc/version
UB_CONFIG=$(grep params /proc/mtd | cut -d: -f1)
Testlab=$(grep "TESTLAB" /dev/$UB_CONFIG | cut -d= -f2)

if [ "$PRODUCT" = "VIP4G" ]; then
   LTE_MAIN_PORT="/dev/ttyUSB0"
   LTE_DIA_PORT="/dev/ttyUSB3"
elif [ "$PRODUCT" = "IPn4G" ]; then
   LTE_MAIN_PORT="/dev/ttyUSB1"
   LTE_DIA_PORT="/dev/ttyUSB4"
fi

if [ "$Testlab" = "1" ]; then 

    if [ ! -e $LTE_MAIN_PORT ];then
        echo "Please wait for device ready ."
        cnt=0
       
        while [ ! -e $LTE_MAIN_PORT -a "$cnt" -lt "30" ] ;do
            echo -n "."
            cnt=$(( $cnt + 1 ))
            sleep 1
        done
        killall -9 hotplug2 >/dev/null 2>&1
        echo "."
    fi
    
    echo "connecting ..."
    mpci -gpio set 8 1
    sleep 4
    
    tip -s 115200 -l $LTE_MAIN_PORT
    exit 0
elif [ "$Testlab" = "2" ]; then 

    if [ ! -e $LTE_DIA_PORT ];then
        echo "Please wait for device ready ."
        cnt=0
       
        while [ ! -e $LTE_DIA_PORT -a "$cnt" -lt "30" ] ;do
            echo -n "."
            cnt=$(( $cnt + 1 ))
            sleep 1
        done
        echo "."
    fi

    echo "connecting ..."
    mpci -gpio set 8 1
    sleep 4
    
    tip -s 115200 -l $LTE_DIA_PORT
    exit 0
else
    echo "This board not for TestLab using"
    sleep 1
    exit
fi


