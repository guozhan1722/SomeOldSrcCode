#!/bin/sh
port=/dev/ttyUSB1
timeLimit=8
timerInterrupt=14
noKill=0
line=0
cid=0
pdp_type=0
apn=0
pdp_addr=0
data_comp=0
head_comp=0
debug=0
success=OK
error=ERROR


code=0
gpsenable=1

atnwCmd=0
divctrlCmd=0
divctrlonCmd=0
divctrloffCmd=0
evdodivctrlCmd=0
evdodivctrlonCmd=0
evdodivctrloffCmd=0
lockstatusCmd=0
lockdevCmd=0
unlockdevCmd=0
ltimeCmd=0
mdnCmd=0
modeprefCmd=0
nnCmd=0
rssiCmd=0
sysmodeCmd=0
prefmodeCmd=0
agcCmd=0
allupCmd=0
gpsCmd=0
gpsstartCmd=0
gpsstopCmd=0
iccidCmd=0
tempCmd=0
voltCmd=0
disconnectCmd=0	
connectCmd=0
statusCmd=0
vzwapneCmd=0
degcCmd=0
imstestmodeCmd=0
mfgCmd=0


WRITETIMEGAP=1

clearFile ()
{
   : > $port
    lock -u "/var/run/VIP4G_STAT_busy"
}

redirect_stdin ()
{
    lock "/var/run/VIP4G_STAT_busy"
    # Link file descriptor #6 with stdin, Saves stdin.
    exec 6<&0            
    # stdin replaced by file
    exec < $port
     
}

restore_stdin ()
{
    # Now restore stdin from fd #6, where it had been saved,
    #+ and close fd #6 ( 6<&- ) to free it for other processes to use.
    exec 0<&6 6<&-
}

printDebug ()
{
    if [ $debug -eq 1 ]; then
       echo "$1"
    fi 
}

int14Vector()
{
    echo "Read TIMEOUT"
    exit $timerInterrupt
}

timerOn()
{
   noKill=0
   {  
      sleep $timeLimit 
      if [ $noKill -ne 1 ]; then
         if [ -d "/proc/$$" ]; then
            kill  -s $timerInterrupt $$ 
         fi
      fi
   }&
   # Waits $timeLimit seconds, then sends sigalarm to script.
}


timerOff()
{
   noKill=1
}

readData ()
{
   readCnt=0
   timerOn
   read -t 2 line
   timerOff
   length=${#line}
   failure=$(echo $line |grep $error)
   if [ -n "$failure" ];then
        line="$error"
   fi
   printDebug "line length = $length"
   while [ "$line" != "$success" -a "$line" != "$error" ]; do 
       timerOn
       read -t 2 line 
       timerOff
       length=${#line}
       printDebug "line length = $length"
       if [ "$length" -gt 0 ]; then
          echo "$line"
       fi

       failure=$(echo $line |grep $error)
       if [ -n "$failure" ];then
            line="$error"
       fi

   done    
}

readResponse () 
{
    read -t 2 line  #flushing the sent command
    printDebug "Flush: $line"

    echo "Response: "
    readData
}

# configure and open tty device
openPort()
{
   printDebug  "opening $port"
   stty -F $port sane clocal -crtscts -hupcl -echo
   if [ "$?" -ne 0 ]; then
      echo "Error Opening $port" 
      exit 1;
   fi 
}

printUsage()
{
    echo
    echo "Usage: atcmd <option> <command> <command_arguments>" 
    echo "Options:"
    echo "  -p|--port" $'\t' "Use to open port instead of default /dev/ttyUSB1"
    echo "  -h|--help" $'\t' "Describes the usages"
   
    echo "Commands:"
    echo "  --atnw" $'\t' "Company details for Novatel Wireless Inc."
    echo "  --divctrl" $'\t' "Current State of Diversity on 1X"
    echo "  --divctrlon" $'\t' "Current State of Diversity is ON"
    echo "  --divctrloff" $'\t' "Current State of Diversity is OFF"
    echo "  --evdodivctrl" $'\t' "Current State of Diversity on EVDO data connectio"
    echo "  --evdodivctrlon" $'\t' "Current State of Diversity on EVDO is ON"
    echo "  --evdodivctrloff" $'\t' "Current State of Diversity on EVDO is OFF"
    echo "  --lockstatus" $'\t' "Lock or unlock status the device"
    echo "  --lockdev" $'\t' "Lock device with <command_arguments> -code"
    echo "  --unlockdev" $'\t' "unLock device with <command_arguments> -code"
    echo "  --ltime" $'\t' "Local time and time zone offset"
    echo "  --mdn" $'\t' "Mobile Directory Number of the Device"
    echo "  --modepref" $'\t' "preferred Mode Setting"
    echo "  --nn" $'\t' "Carrier Network Information"
    echo "  --rssi" $'\t' "RSSI Information"
    echo "  --sysmode" $'\t' "Network Type"
    echo "  --prefmode" $'\t' "Current Mode set"
    echo "  --agc" $'\t' "Display the agc value"
    echo "  --allup" $'\t' "Turn RF Transmitter On/off"
    echo "  --gps" $'\t' "Enable/disable GPS -gpsenable=0 or =1"
    echo "  --gpsstart" $'\t' "Start GPS "
    echo "  --gpsstop" $'\t' "Stop GPS "
    echo "  --iccid" $'\t' "SIM's ICCID "
    echo "  --temp" $'\t' "Display Device Temperature "
    echo "  --volt" $'\t' "Input Voltage "
    echo "  --connect" $'\t' "Start a package data session with <command_arguments> -tech, -call_para, -pdns, -sdns, -pnbns, -snbns, \
-apn, -ip, -auth, -user, -pwd"
    echo "  --status" $'\t' "Query a package data connect status"  
    echo "  --disconnect" $'\t' "Stop a package data session"
    echo "  --vzwapne" $'\t' "Overwrite APN Table"
    echo "  --degc" $'\t' "Query the PMIC Temperature"
    echo "  --imstestmode" $'\t' "Turn ON/OFF the IMS Test Mode"
    echo "  --mfg" $'\t' "Get the Manufacture Date of the Device"

    echo "Examples:"
    echo "  atcmd.sh --port /dev/ttyUSB0 --pdp_context -cid=1 -pdp_type=\"IP\" -pdp_addr=\"abc.com\" "
    echo "  atcmd.sh --port /dev/ttyUSB0 --connect -tech=1 -call_para=2, -pdns=\"abc.com\" -user=\"test\" -apn=\"abc\" "
}

# Parse command line arguments
while [ $# -gt 0 ]; do
    case $1 in
    --atnw)
        atnwCmd=1
        ;;
    --divctrl)
        divctrlCmd=1
        ;;
    --divctrlon)
        divctrlonCmd=1
        ;;
    --divctrloff)
        divctrloffCmd=1
        ;;

    --evdodivctrl)
        evdodivctrlCmd=1
        ;;
    --evdodivctrlon)
        evdodivctrlonCmd=1
        ;;
    --evdodivctrloff)
        evdodivctrloffCmd=1
        ;;

    --lockstatus)
        lockstatusCmd=1
        ;;
    --lockdev)
        lockdevCmd=1
        ;;
    --unlockdev)
        unlockdevCmd=1
        ;;
    --ltime)
        ltimeCmd=1
        ;;
    --mdn)
        mdnCmd=1
        ;;
    --modepref)
        modeprefCmd=1
        ;;
    --nn)
        nnCmd=1
        ;;
    --rssi)
        rssiCmd=1
        ;;

    --sysmode)
        sysmodeCmd=1
        ;;

    --prefmode)
        prefmodeCmd=1
        ;;

    --agc)
        agcCmd=1
        ;;
    --allup)
        allupCmd=1
        ;;
    --gps)
        gpsCmd=1
        ;;
    --gpsstart)
        gpsstartCmd=1
        ;;
    --gpsstop)
        gpsstopCmd=1
        ;;
    --iccid)
        iccidCmd=1
        ;;

    --temp)
        tempCmd=1
        ;;
    --volt)
        voltCmd=1
        ;;

    --disconnect)
        disconnectCmd=1	
        ;;

    --connect)
        connectCmd=1
        ;;

    --status)
        statusCmd=1
        ;;

    --vzwapne)
        vzwapneCmd=1
        ;;

    --degc)
        degcCmd=1
        ;;

    --imstestmode)
        imstestmodeCmd=1
        ;;
    --mfg)
        mfgCmd=1
        ;;


    -gpsenable*)
         if echo $1 | grep '=' >/dev/null ; then
            gpsenable=`echo $1 | sed 's/^.*=//'`
         else
            gpsenable="$2"
            shift
         fi
	 ;;

      -code*)
         if echo $1 | grep '=' >/dev/null ; then
            code=`echo $1 | sed 's/^.*=//'`
         else
            code="$2"
            shift
         fi
	 ;;


    -p|--port)
        if echo $1 | grep '=' >/dev/null ; then
            port=`echo $1 | sed 's/^.*=//'`
        else
            port="$2"
            shift
        fi

	;;
   
    -d|--debug)
        debug=1
        ;;

    -h|--help)
        echo "It is necessary to specify the device port with option \
-p or --port along with command parameters with command options, the default port=/dev/ttyUSB0\n"
	printUsage
        exit 0
        ;;

    -cid*)
         if echo $1 | grep '=' >/dev/null ; then
            cid=`echo $1 | sed 's/^.*=//'` 
         else
            cid="$2"
            shift
         fi
	 ;;

      -pdp_type*)
	 if echo $1 | grep '=' >/dev/null ; then
            pdp_type=`echo $1 | sed 's/^.*=//'`
         else
            pdp_type="$2"
            shift
         fi
	 ;;
     
      -apn*)
         if echo $1 | grep '=' >/dev/null ; then
            apn=`echo $1 | sed 's/^.*=//'`
         else
            apn="$2"
            shift
         fi
	 ;;

      -pdp_addr*)
         if echo $1 | grep '=' >/dev/null ; then
            pdp_addr=`echo $1 | sed 's/^.*=//'`
         else
            pdp_addr="$2"
            shift
         fi
	 ;;

     -data_comp*)
         if echo $1 | grep '=' >/dev/null ; then
            data_comp=`echo $1 | sed 's/^.*=//'`
         else
            data_comp="$2"
            shift
         fi
	 ;;
      
     -head_comp*)
         if echo $1 | grep '=' >/dev/null ; then
            head_comp=`echo $1 | sed 's/^.*=//'`
         else
            head_comp="$2"
            shift
         fi
	 ;;  

     -tech*)
         if echo $1 | grep '=' >/dev/null ; then
            tech=`echo $1 | sed 's/^.*=//'`
         else
            tech="$2"
            shift
         fi
	 ;;  

     -call_para*) 
	 if echo $1 | grep '=' >/dev/null ; then
            callpara=`echo $1 | sed 's/^.*=//'`
         else
            callpara="$2"
            shift
         fi
	 ;;  

     -pdns*)
	 if echo $1 | grep '=' >/dev/null ; then
            pdns=`echo $1 | sed 's/^.*=//'`
         else
            pdns="$2"
            shift
         fi
	 ;;  

     -sdns*)
         if echo $1 | grep '=' >/dev/null ; then
            sdns=`echo $1 | sed 's/^.*=//'`
         else
            sdns="$2"
            shift
         fi
	 ;;  

     -pnbns*)
         if echo $1 | grep '=' >/dev/null ; then
            pnbns=`echo $1 | sed 's/^.*=//'`
         else
            pnbns="$2"
            shift
         fi
	 ;; 

     -snbns*) 
         if echo $1 | grep '=' >/dev/null ; then
            pnbns=`echo $1 | sed 's/^.*=//'`
         else
            pnbns="$2"
            shift
         fi
	 ;; 

     -ip*)
         if echo $1 | grep '=' >/dev/null ; then
            ip=`echo $1 | sed 's/^.*=//'`
         else
            ip="$2"
            shift
         fi
	 ;; 

    -auth*)
         if echo $1 | grep '=' >/dev/null ; then
            auth=`echo $1 | sed 's/^.*=//'`
         else
            auth="$2"
            shift
         fi
	 ;; 

   -user*) 
         if echo $1 | grep '=' >/dev/null ; then
            user=`echo $1 | sed 's/^.*=//'`
         else
            user="$2"
            shift
         fi
         ;;        

   -pwd*)
         if echo $1 | grep '=' >/dev/null ; then
            pwd=`echo $1 | sed 's/^.*=//'`
         else
            pwd="$2"
            shift
         fi
         ;;   

    esac
    shift
done

openPort
trap int14Vector $timerInterrupt

if [ $atnwCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nw" 
   echo "at\$nw" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $divctrlCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwdivctrl?" 
   echo "at\$nwdivctrl?" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $divctrlonCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwdivctrl=1" 
   echo "at\$nwdivctrl=1" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $divctrloffCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwdivctrl=0" 
   echo "at\$nwdivctrl=0" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $evdodivctrlCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwevdodivctrl?" 
   echo "at\$nwevdodivctrl?" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $evdodivctrlonCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwevdodivctrl=1" 
   echo "at\$nwevdodivctrl=1" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $evdodivctrloffCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwevdodivctrl=0" 
   echo "at\$nwevdodivctrl=0" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $lockstatusCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwlock?" 
   echo "at\$nwlock?" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $lockdevCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwlock=$code,1" 
   #echo "at\$nwlock=code,1" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $unlockdevCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwlock=$code,0" 
   #echo "at\$nwlock=code,0" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $ltimeCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwltime" 
   echo "at\$nwltime" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $mdnCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwmdn=?" 
   echo "at\$nwmdn=?" >$port
   readResponse
   restore_stdin
   clearFile
elif [ $modeprefCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwmodepref" 
   echo "at\$nwmodepref" >$port
   readResponse
   restore_stdin
   clearFile
elif [ $nnCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwnn" 
   echo "at\$nwnn" >$port
   readResponse
   restore_stdin
   clearFile
elif [ $rssiCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwrssi=15" 
   echo "at\$nwrssi=15" >$port
   readResponse
   restore_stdin
   clearFile
elif [ $sysmodeCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwsysmode" 
   echo "at\$nwsysmode" >$port
   readResponse
   restore_stdin
   clearFile
elif [ $prefmodeCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwprefmode?" 
   echo "at\$nwprefmode?" >$port
   readResponse
   restore_stdin
   clearFile
elif [ $agcCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at+agc=?" 
   echo "at+agc=?" >$port
   readResponse
   restore_stdin
   clearFile
elif [ $allupCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at+allup=?" 
   echo "at+allup=?" >$port
   readResponse
   restore_stdin
   clearFile
elif [ $gpsCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwgps=$gpsenable" 
   echo "at\$nwgps=$gpsenable" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $gpsCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwgps=$gpsenable" 
   echo "at\$nwgps=$gpsenable" >$port
   readResponse
   restore_stdin
   clearFile
elif [ $gpsstartCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwgpsstart=3,1,65530,250,50,255" 
   echo "at\$nwgpsstart=3,1,65530,250,50,255" >$port
   readResponse
   restore_stdin
   clearFile
elif [ $gpsstopCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$nwgpsstop" 
   echo "at\$nwgpsstop" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $iccidCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at+iccid?" 
   echo "at+iccid?" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $tempCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at+temp" 
   echo "at+temp" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $voltCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at+volt?" 
   echo "at+volt?" >$port
   readResponse
   restore_stdin
   clearFile
elif [ $disconnectCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at$nwqmidisconnect'
   echo 'at$nwqmidisconnect' >$port
   readResponse 
   clearFile
   restore_stdin
 
elif [ $connectCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at$nwqmiconnect=' "$tech,$callpara,$pdns,$sdns,$pnbns,$snbns,\
$apn,$ip,$auth,$user,$pwd"
   echo 'at$nwqmiconnect=' "$tech,$callpara,$pdns,$sdns,$pnbns,$snbns,\
$apn,$ip,$auth,$user,$pwd" >$port
   readResponse 
   clearFile
   restore_stdin

elif [ $statusCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at$nwqmistatus'
   echo 'at$nwqmistatus' >$port
   readResponse 
   clearFile
   restore_stdin
elif [ $vzwapneCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at+vzwapne?'
   echo 'at+vzwapne?' >$port
   readResponse 
   clearFile
   restore_stdin

elif [ $degcCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at$nwdegc'
   echo 'at$nwdegc' >$port
   readResponse 
   clearFile
   restore_stdin

elif [ $imstestmodeCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at$nwimstestmode?'
   echo 'at$nwimstestmode?' >$port
   readResponse 
   clearFile
   restore_stdin
elif [ $mfgCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at$nwmfg?'
   echo 'at$nwmfg?' >$port
   readResponse 
   clearFile
   restore_stdin


else 
   echo "No command Specified, try --help"

fi

