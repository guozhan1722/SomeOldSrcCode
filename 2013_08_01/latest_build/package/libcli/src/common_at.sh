#!/bin/sh

#common at shell program for AT+Command 
#. /usr/lib/webif/webif.sh
#. /usr/lib/webif/functions.sh
#. /etc/functions.sh
. /etc/version

#echo "*:  $*"
#echo "#:  $#"
#echo "?:  $?"
#echo "0:  $0"
#echo "1:  $1"
#echo "2:  $2"
#echo "begin to start:"
#	VIP2> testd a b c d
#	*:  a b c d
#	#:  4
#	?:  0
#	0:  /etc/m_cli/test.sh
#	1:  a
#	2:  b

#exec>/dev/null 2>&1

 
#restore 0/1
#passwd admin %s %s
case "$1" in 

"passwd")
    if [ "$#" = "4" ]; then
    if [ "$2" = "admin" ]; then
    if [ "$3" = "$4" ]; then
       RES=$(
		(
		 echo "$3"
		 sleep 1
		 echo "$4"
		) | passwd "$2" 2>&1
	     )
       [ "$?" = "0" ] && {
                            echo "Successed."
                            exit 0
                         }    
        echo "Not successed."
    fi
    fi
    fi
    echo "Invalid parameters."
    ;;


"restore")
    if [ "$#" = "2" ]; then
    if [ "$2" = "1" ]; then
        COPY_FILES="/etc/firewall.config
        /etc/firewall.user
        /etc/httpd.conf
        /etc/hosts
        /etc/ethers
        /etc/passwd"
    
        COPY_DIRS="/etc/config
        /etc/openvpn
        /etc/crontabs
        /etc/ssl
        /etc/dropbear"

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

        echo "Restored OK"
        reboot &
        exit 0
    fi
    fi
    echo "Invalid parameters."
   ;;


"test")
    echo "test program:$0 ($#):$1 :$2 :$3"
   ;;
	
*)
    echo "none to do"
    exit 1
   ;;

esac 
exit 0



