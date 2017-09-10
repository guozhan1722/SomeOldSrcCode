#!/bin/sh
port=/dev/ttyUSB0
timeLimit=130
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

atiCmd=0
techCmd=0
techAvailCmd=0
techSuppCmd=0
pdpContextCmd=0
rssiCmd=0
ratCmd=0
simpinCmd=0
disconnectCmd=0	
connectCmd=0
statusCmd=0
imeiCmd=0
simidCmd=0
networkCmd=0
searchCarrierCmd=0
roamingCmd=0
phonenumCmd=0
cellidlacCmd=0
tempCmd=0
gpsstartCmd=0
gpsstopCmd=0
resetCmd=0
imsiCmd=0
WRITETIMEGAP=1

clearFile ()
{
   : > $port
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
    lock -u "/var/run/VIP4G_STAT_busy"
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
    echo "  -p|--port" $'\t' "Use to open port instead of default /dev/ttyUSB0"
    echo "  -h|--help" $'\t' "Describes the usages"
   
    echo "Commands:"
    echo "  --ati" $'\t' "Get General Information about Modem"
    echo "  --tech" $'\t' "Current technology in use"
    echo "  --tech_avail" $'\t' "available technologies on the current network"
    echo "  --tech_supp" $'\t' "All technologies supported by the device"
    echo "  --pdp_context " "Configures pdp context with <command_arguments> -cid, -pdp_type, -apn, -pdp_addr, -data_comp, -head_comp"
    echo "  --rssi" $'\t' "Determines Signal Quality with <rssi> and <ber> on response "
    echo "  --rat" $'\t' "Determines Signal State with <rat> on response "
    echo "  --connect" $'\t' "Start a package data session with <command_arguments> -tech, -call_para, -pdns, -sdns, -pnbns, -snbns, \
-apn, -ip, -auth, -user, -pwd"
    echo "  --status" $'\t' "Query a package data connect status"  
    echo "  --disconnect" $'\t' "Stop a package data session"
    echo "  --simid" $'\t' "Get SIMID"
    echo "  --simpin" $'\t' "Get SIM PIN"
    echo "  --roaming" $'\t' "Get Roaming Info"
    echo "  --network" $'\t' "Get Network Info"
    echo "  --search" $'\t' "Get avilable Carrier Info"
    echo "  --cellidlac" $'\t' "Get Cell id and Lac"
    echo "  --temp" $'\t' "Get TEMPERATURE"
    echo "  --reset" $'\t' "Reset"
    echo "  --imei" $'\t' "Get IMEI"
    echo "  --phonenum" $'\t' "Get Phone num"
    echo "  --gpsstart" $'\t' "Set GPS start"
    echo "  --gpsstop" $'\t' "Set GPS stop"
    echo "  --imsi" $'\t' "Get IMSI"
    echo "Examples:"
    echo "  atcmd.sh --port /dev/ttyUSB0 --pdp_context -cid=1 -pdp_type=\"IP\" -pdp_addr=\"abc.com\" "
    echo "  atcmd.sh --port /dev/ttyUSB0 --connect -tech=1 -call_para=2, -pdns=\"abc.com\" -user=\"test\" -apn=\"abc\" "
}

# Parse command line arguments
while [ $# -gt 0 ]; do
    case $1 in
    --ati)
        atiCmd=1
        ;;

    --tech)
        techCmd=1
	;;

    --tech_avail)
   	techAvailCmd=1
	;;

    --tech_supp)
	techSuppCmd=1
	;;

    --pdp_context)
        pdpContextCmd=1
        ;;
     
    --rssi)
        rssiCmd=1 
        ;;

    --rat)
        ratCmd=1 
        ;;

    --simpin)
        simpinCmd=1 
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

    --imei)
        imeiCmd=1
        ;;
    --simid)
        simidCmd=1
        ;;

    --roaming)
        roamingCmd=1
        ;;

    --network)
        networkCmd=1
        ;;

    --search)
        searchCarrierCmd=1
        ;;

    --phonenum)
        phonenumCmd=1
        ;;

    --cellidlac)
        cellidlacCmd=1
        ;;

    --temp)
        tempCmd=1
        ;;

    --gpsstart)
        gpsstartCmd=1
        ;;

    --gpsstop)
        gpsstopCmd=1
        ;;

    --reset)
        resetCmd=1
        ;;

    --imsi)
        imsiCmd=1
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

if [ $atiCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending ati" 
   echo "ati" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $techCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$cnti=0" 
   echo "at\$cnti=0" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $techAvailCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$cnti=1" 
   echo "at\$cnti=1" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $techSuppCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at\$cnti=2" 
   echo "at\$cnti=2" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $pdpContextCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo "Sending at+cgdcont=$cid,$pdp_type,$apn,$pdp_addr,$data_comp,$head_comp"
   echo "at+cgdcont=$cid, $pdp_type, $apn, $pdp_addr, $data_comp, $head_comp" >$port
   clearFile
   readResponse

elif [ $rssiCmd -eq 1 ]; then 
   clearFile
   redirect_stdin
   echo "Sending at\$nwrssi=15" 
   echo "at\$nwrssi=31" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $ratCmd -eq 1 ]; then 
   clearFile
   redirect_stdin
   echo "Sending at\$nwrat?" 
   echo "at\$nwrat?" >$port
   readResponse
   restore_stdin
   clearFile

elif [ $simpinCmd -eq 1 ]; then 
   clearFile
   redirect_stdin
   echo "Sending at+cpin?" 
   echo "at+cpin?" >$port
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

elif [ $imeiCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at+cgsn'
   echo 'at+cgsn' >$port
   readResponse 
   clearFile
   restore_stdin

elif [ $simidCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at+iccid?'
   echo 'at+iccid?' >$port
   readResponse 
   clearFile
   restore_stdin

elif [ $roamingCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at+cereg?'
   echo 'at+cereg?' >$port
   readResponse 
   clearFile
   restore_stdin

elif [ $networkCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at+cops?'
   echo 'at+cops?' >$port
   readResponse 
   clearFile
   restore_stdin

elif [ $searchCarrierCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at+cops=?'
   echo 'at+cops=?' >$port
   readResponse 
   clearFile
   restore_stdin

elif [ $phonenumCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at+cnum'
   echo 'at+cnum' >$port
   readResponse 
   clearFile
   restore_stdin

elif [ $cellidlacCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at$nwcid?'
   echo 'at$nwcid?' >$port
   readResponse 
   clearFile
   restore_stdin

elif [ $tempCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at$nwdegc'
   echo 'at$nwdegc' >$port
   readResponse 
   clearFile
   restore_stdin

elif [ $gpsstartCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at$nwgpsstart=3,1,4,250,50,255'
   echo 'at$nwgpsstart=3,1,4,250,50,255' >$port
   readResponse 
   clearFile
   restore_stdin

elif [ $gpsstopCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at$nwgpsstop'
   echo 'at$nwgpsstop' >$port
   readResponse 
   clearFile
   restore_stdin

elif [ $resetCmd -eq 1 ]; then
   clearFile
   redirect_stdin

   #echo 'Sending at+cfun=1,1'
   #echo 'at+cfun=1,1' >$port
   echo 'Sending at+cfun=7'
   echo 'at+cfun=7' >$port
   readResponse 
   clearFile
   echo 'Sending at+cfun=6'
   echo 'at+cfun=6' >$port
   readResponse 
   clearFile
   restore_stdin

elif [ $imsiCmd -eq 1 ]; then
   clearFile
   redirect_stdin
   echo 'Sending at+cimi'
   echo 'at+cimi' >$port
   readResponse 
   clearFile
   restore_stdin
else 
   echo "No command Specified, try --help"

fi

