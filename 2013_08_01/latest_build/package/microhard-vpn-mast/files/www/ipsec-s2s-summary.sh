#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

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
    esac
}

cur_color="even"
get_tr() {
	if equal "$cur_color" "odd"; then
		cur_color="even"
		tr="string|<tr>"
	else
		cur_color="odd"
		tr="string|<tr class=\"odd\">"
	fi
}
[ -n "$FORM_interval" ] || FORM_interval=20

! empty "$FORM_refreshstop" && {
	FORM_interval=0
}

[ "$FORM_interval" -gt 0 ] && {
	meta_refresh="<meta http-equiv=\"refresh\" content=\"$FORM_interval;url=$SCRIPT_NAME?interval=$FORM_interval\" />"
}

header_inject_head=$(cat <<EOF
$meta_refresh
EOF

)

header "VPN" "Summary" "@TR<<Summary>>" '' "$SCRIPT_NAME"
#remove s2s tunnel
if ! empty "$FORM_remove_s2s_vcfg"; then
	uci_remove "ipsec" "$FORM_remove_s2s_vcfg"
        /usr/lib/webif/apply.sh del_s2s "$FORM_remove_s2s_vcfg" >/dev/null 2>&1
fi

#remove x2c tunnel
if ! empty "$FORM_remove_x2c_vcfg"; then
        xinf=$(sbin/uci get ipsec.${FORM_remove_x2c_vcfg}.interface)
	uci_remove "ipsec" "$FORM_remove_x2c_vcfg"
        /usr/lib/webif/apply.sh del_x2c "$FORM_remove_x2c_vcfg $xinf" >/dev/null 2>&1
fi

#remove vpn client access
if ! empty "$FORM_remove_vca_vcfg"; then
	uci_remove "ipsec" "$FORM_remove_vca_vcfg"
        /usr/lib/webif/apply.sh update_vca "$FORM_remove_vca_vcfg" >/dev/null 2>&1
fi

#add s2s tunnel
if ! empty "$FORM_add_s2s" ; then
        echo "<meta http-equiv=\"refresh\" content=\"0; url=/cgi-bin/webif/ipsec-s2s.sh\" />"
else
#add x2c tunnel
if ! empty "$FORM_add_x2c" ; then
        echo "<meta http-equiv=\"refresh\" content=\"0; url=/cgi-bin/webif/ipsec-x2c.sh\" />"
else
#add vpn client access
if ! empty "$FORM_add_vca" ; then
        echo "<meta http-equiv=\"refresh\" content=\"0; url=/cgi-bin/webif/ipsec-vca.sh\" />"
else

uci_load ipsec
for s2s in $s2s_cfgs; do
    
    eval FORM_connect="\$FORM_${s2s}"
    
  if ! empty "$FORM_connect" ; then
       [ -f "/etc/debugipsec" ] && logger -p local0.error -t "ipsec-s2s-summary.sh" "$FORM_connect is not empty and action"
       s2s_peer=$(/sbin/uci get ipsec.${s2s}.right)
          
       case "$FORM_connect" in
             Connect)
                     echo "status=waiting for connection" > /var/run/mipsec/$s2s.status
                     echo "constat=Waiting..." >> /var/run/mipsec/$s2s.status
                     echo "action=up" >> /var/run/mipsec/$s2s.status
                     echo "peer=$s2s_peer" >> /var/run/mipsec/$s2s.status
                     /sbin/mipsec.sh connect_s2s $s2s >/dev/null 2>&1 &
             ;;
             Disconnect|Waiting...)
                     #echo "status=waiting for connection" > /var/run/mipsec/$s2s.status
                     #echo "constat=Waiting..." >> /var/run/mipsec/$s2s.status
                     #echo "action=down" >> /var/run/mipsec/$s2s.status
                     #echo "peer=$s2s_peer" >> /var/run/mipsec/$s2s.status
                     /sbin/mipsec.sh disconnect_s2s $s2s >/dev/null 2>&1
             ;;
             *)
             ;;
        esac
  fi
  
done

for x2c in $x2c_cfgs; do
    
    eval FORM_connectx2c="\$FORM_${x2c}"
    
  if ! empty "$FORM_connectx2c" ; then
       [ -f "/etc/debugipsec" ] && logger -p local0.error -t "ipsec-s2s-summary.sh" "$FORM_connectx2c is not empty and action"
       case "$FORM_connectx2c" in
             Connect)
                     echo "action=up" > /var/run/mipsec/$x2c
                     echo "status=waiting for connection" >> /var/run/mipsec/$x2c
                     echo "constat=Waiting..." >> /var/run/mipsec/$x2c
                     /sbin/mipsec.sh connect_x2c $x2c >/dev/null 2>&1 &
             ;;
             Disconnect|Waiting...)
                     /sbin/mipsec.sh disconnect_x2c $x2c >/dev/null 2>&1
             ;;
             *)
             ;;
        esac
  fi
  
done
    
#summary of s2stunnel
form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<Gateway To Gateway>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<Gateway To Gateway>>\">
	string|<tr><th>@TR<<No.>></th>
	string|<th>@TR<<Name>></th>
	string|<th>@TR<<Status>></th>
	string|<th>@TR<<Phase2 Enc/Auth/Grp>></th>
        string|<th>@TR<<Interface>></th>
	string|<th>@TR<<Local Group>></th>
	string|<th>@TR<<Remote Group>></th>
	string|<th>@TR<<Remote Gateway>></th>
        string|<th>@TR<<RX/TX Bytes>></th>
	string|<th>@TR<<Tunnel Test>></th>
	string|<th>@TR<<Config.>></th>
	string|</tr>"
append forms "$form" "$N"

cfgcnt="0"

for s2s in $s2s_cfgs; do
    config_get FORM_esp_alg           $s2s esp_alg  
    config_get FORM_esp_auth          $s2s esp_auth 
    config_get FORM_esp_dh            $s2s esp_dh   
    config_get FORM_leftsubnet        $s2s leftsubnet  
    config_get FORM_rightsubnet       $s2s rightsubnet  
    config_get FORM_righttype         $s2s righttype
    config_get FORM_right             $s2s right
    config_get FORM_rightid           $s2s rightid  
    config_get s2s_enabled            $s2s enabled
    config_get FORM_s2s_inf           $s2s interface
    config_get s2s_dpdaction          $s2s dpdaction
    config_get FORM_binf              $s2s binterface
    FORM_s2s_RX="N/A"
    FORM_s2s_TX=""
    FORM_peer="$FORM_right"

    if [ "$s2s_enabled" = "0" ]; then
    FORM_status="disable"
    FORM_connect_status="N/A"
    FORM_peer="$FORM_right"
    else
    #/sbin/mipsec.sh check_status_s2s $s2s >/dev/null 2>&1
    [ -f "/var/run/mipsec/${s2s}.status" ] && {
    FORM_status=`cat /var/run/mipsec/${s2s}.status | grep 'status' | cut -d '=' -f 2`
    FORM_connect_status=`cat /var/run/mipsec/${s2s}.status | grep 'constat' | cut -d '=' -f 2`
    FORM_peer=`cat /var/run/mipsec/${s2s}.status | grep 'peer' | cut -d '=' -f 2`
    } || {
    FORM_status="waiting for connection"
    FORM_connect_status="Connect"
    }
    if [ $FORM_s2s_inf = "br-wan" ]; then
         wan_up=$(/sbin/uci -P /var/state get network.wan.up)
    else 
         wan_up=$(/sbin/uci -P /var/state get network.wan2.up)
    fi
    [ $wan_up = "1" ] || {
    FORM_status="interface down"
    FORM_connect_status="N/A"
    }
    fi

    if [ $s2s_dpdaction = "restart_by_peer" ]; then
          if [ $FORM_peer = "$FORM_right" ]; then
               name_display="${s2s}(Primary)"

          else
               name_display="${s2s}(Secondary)"
               if [ $FORM_binf = "br-wan" ]; then
                     wan_up=$(/sbin/uci -P /var/state get network.wan.up)
               else 
                     wan_up=$(/sbin/uci -P /var/state get network.wan2.up)
               fi
               [ $wan_up = "1" ] || {
                    FORM_status="interface(S) down"
                    FORM_connect_status="N/A"
               }
               FORM_s2s_inf="$FORM_binf"
          fi
    else
          name_display="${s2s}"              
    fi
    let "cfgcnt+=1"
    
    get_tr
    form="$tr  
            string|<td>
            string|$cfgcnt
            string|</td>

            string|<td>
            string|$name_display
            string|</td>
    
            string|<td>
            string|$FORM_status
            string|</td>
    
            string|<td>
            string|${FORM_esp_alg}/${FORM_esp_auth}/${FORM_esp_dh}
            string|</td>
            
            string|<td>
            string|${FORM_s2s_inf}
            string|</td>

            string|<td>
            string|$FORM_leftsubnet
            string|</td>
    
            string|<td>
            string|$FORM_rightsubnet
            string|</td>"
     append forms "$form" "$N"
   if [ "$FORM_righttype" = "dip-id" ]; then
      form="string|<td>
            string|$FORM_rightid
            string|</td>"
      append forms "$form" "$N"
   else
      form="string|<td>
            string|$FORM_peer
            string|</td>"
      append forms "$form" "$N"
   fi

   if [ "$FORM_status" = "connected" ]; then
       cmd="ipsec auto --status | grep '\"$s2s\" newest-IPSEC' | cut -d ' ' -f 6"
       tunin=`eval $cmd`
       cmd="ipsec auto --status | grep '\"$s2s\" newest-IPSEC' | cut -d ' ' -f 5"
       tunout=`eval $cmd`
       cmd="cat /proc/net/ipsec/spi/all | grep '$tunin' | grep -c 'life(c,s,h)=bytes('"
       s2sRX=`eval $cmd`
       FORM_s2s_RX="0 B"
       FORM_s2s_TX="0 B"
       [ "$s2sRX" -gt 0 ] && {
           cmd="cat /proc/net/ipsec/spi/all | grep '$tunin' | cut -d ')' -f 2 | cut -d '(' -f 2 | cut -d ',' -f 1"
           s2sRX=`eval $cmd`
           if [ "$s2sRX" -gt 1073741824 ]; then
                gb=$((s2sRX / 1073741824))
                sb=$((s2sRX % 1073741824))
                cb=$((sb / 107374182))
                if [ $cb -gt 0 ]; then
                     FORM_s2s_RX="$gb.$cb GB"
                else
                     FORM_s2s_RX="$gb GB"
                fi
          else
                if [ "$s2sRX" -gt 1048576 ]; then
                     gb=$((s2sRX / 1048576))
                     sb=$((s2sRX % 1048576))
                     cb=$((sb / 104857))
                     if [ $cb -gt 0 ]; then
                          FORM_s2s_RX="$gb.$cb MB"
                     else
                          FORM_s2s_RX="$gb MB"
                     fi
                else
                     if [ "$s2sRX" -gt 1024 ]; then
                          gb=$((s2sRX / 1024))
                          sb=$((s2sRX % 1024))
                          cb=$((sb / 102))
                          if [ $cb -gt 0 ]; then
                               FORM_s2s_RX="$gb.$cb KB"
                          else
                               FORM_s2s_RX="$gb KB"
                          fi
                     else 
                          FORM_s2s_RX="$s2sRX B"
                     fi
                fi
           fi
       }
       cmd="cat /proc/net/ipsec/spi/all | grep '$tunout' | grep -c 'life(c,s,h)=bytes('"
       s2sTX=`eval $cmd`
       [ "$s2sTX" -gt 0 ] && {
           cmd="cat /proc/net/ipsec/spi/all | grep '$tunout' | cut -d ')' -f 2 | cut -d '(' -f 2 | cut -d ',' -f 1"
           s2sTX=`eval $cmd`
           if [ "$s2sTX" -gt 1073741824 ]; then
                gb=$((s2sTX / 1073741824))
                sb=$((s2sTX % 1073741824))
                cb=$((sb / 107374182))
                if [ $cb -gt 0 ]; then
                     FORM_s2s_TX="$gb.$cb GB"
                else
                     FORM_s2s_TX="$gb GB"
                fi
          else
                if [ "$s2sTX" -gt 1048576 ]; then
                     gb=$((s2sTX / 1048576))
                     sb=$((s2sTX % 1048576))
                     cb=$((sb / 104857))
                     if [ $cb -gt 0 ]; then
                          FORM_s2s_TX="$gb.$cb MB"
                     else
                          FORM_s2s_TX="$gb MB"
                     fi
                else
                     if [ "$s2sTX" -gt 1024 ]; then
                          gb=$((s2sTX / 1024))
                          sb=$((s2sTX % 1024))
                          cb=$((sb / 102))
                          if [ $cb -gt 0 ]; then
                               FORM_s2s_TX="$gb.$cb KB"
                          else
                               FORM_s2s_TX="$gb KB"
                          fi
                     else 
                          FORM_s2s_TX="$s2sTX B"
                     fi
                fi
           fi
       }
   fi
      form="string|<td>
            string|$FORM_s2s_RX
            string|<br>$FORM_s2s_TX
            string|</td>"
      append forms "$form" "$N"

   if [ "$FORM_connect_status" = "Waiting..." -o "$FORM_connect_status" = "N/A" ]; then
      form="string|<td>
            string|$FORM_connect_status
            string|</td>"
   else
      form="string|<td>
            string|<input type=\"submit\" name=\"$s2s\" id=\"id_$s2s\" value=\"$FORM_connect_status\" />
            string|</td>"
   fi
      append forms "$form" "$N"
      form="string|<td>
            string|<a href=\"$SCRIPT_NAME?remove_s2s_vcfg=$s2s\">@TR<<Remove>></a>
            string|</td>

            string|<td>
            string|<a href=\"/cgi-bin/webif/ipsec-s2s-edit.sh?editname=$s2s\">@TR<<Edit>></a>
            string|</td>

            string|</tr>"

      append forms "$form" "$N"
done

get_tr
if [ "$cfgcnt" -lt "16" ]; then
form="$tr
        string|<tr><td colspan="2">
        string|<input type=\"submit\" name=\"add_s2s\" value=\"Add\" />
        string|</td></tr>
        string|</table></div>
        string|<br/>"
else
form="  string|</table></div>
        string|<br/>"
fi

append forms "$form" "$N"

cur_color="even"
#summary of x2ctunnel
form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<Client To Gateway>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<Client To Gateway>>\">
	string|<tr><th>@TR<<No.>></th>
	string|<th>@TR<<Name>></th>
	string|<th>@TR<<Status>></th>
        string|<th>@TR<<Interface>></th>
	string|<th>@TR<<Local/Remote IP Address>></th>
	string|<th>@TR<<Server Gateway>></th>
        string|<th>@TR<<Start Time>></th>
        string|<th>@TR<<Duration>></th>
        string|<th>@TR<<RX/TX Bytes>></th>
	string|<th>@TR<<Tunnel Test>></th>
	string|<th>@TR<<Config.>></th>
	string|</tr>"
append forms "$form" "$N"

cfgcnt="0"

rm -rf /var/run/ipsecx2c >&- 2>&-
for x2c in $x2c_cfgs; do
    config_get FORM_status_x2c            $x2c status    
    config_get FORM_connect_status_x2c    $x2c connect_status
    config_get FORM_right_x2c             $x2c right  
    config_get x2c_enabled                $x2c enabled
    config_get FORM_x2c_inf               $x2c interface
    if [ "$x2c_enabled" = "0" ]; then
    FORM_status_x2c="disable"
    FORM_connect_status_x2c="N/A"
    else
    [ -f "/var/run/mipsec/$x2c" ] && {
    FORM_status_x2c=$(cat /var/run/mipsec/$x2c | grep 'status' | cut -d '=' -f 2)
    FORM_connect_status_x2c=$(cat /var/run/mipsec/$x2c | grep 'constat' | cut -d '=' -f 2)
    } || {

    FORM_status_x2c="waiting for connection"
    FORM_connect_status_x2c="Connect"
    } 
    if [ $FORM_x2c_inf = "br-wan" ]; then
         wan_up=$(/sbin/uci -P /var/state get network.wan.up)
    else 
         wan_up=$(/sbin/uci -P /var/state get network.wan2.up)
    fi
    [ $wan_up = "1" ] || {
    FORM_status_x2c="interface down"
    FORM_connect_status_x2c="N/A"        
    }
    fi

    if [ "$FORM_status_x2c" = "connected" ]; then
    FORM_lip=$(cat /var/run/mipsec/$x2c | grep 'localip=' | cut -d '=' -f 2)
    FORM_serverg=$(cat /var/run/mipsec/$x2c | grep 'serverip=' | cut -d '=' -f 2)
    FORM_serip=""
    [ -z "$FORM_serverg" ] || FORM_serip="/$FORM_serverg"
    x2c_inf=$(cat /var/run/mipsec/$x2c | grep 'inf=' | cut -d '=' -f 2)
    FORM_x2c_rx=`ifconfig -a $x2c_inf | grep 'RX bytes' | cut -d ':' -f 2 | cut -d '(' -f 2 | cut -d ')' -f 1 | sed 's/i//g`
    FORM_x2c_tx=`ifconfig -a $x2c_inf | grep 'RX bytes' | cut -d ':' -f 3 | cut -d '(' -f 2 | cut -d ')' -f 1 | sed 's/i//g`
    FORM_starttime=`cat /var/run/${x2c_inf}.status | grep 'starttime' | cut -d '=' -f 2`
    start=`cat /var/run/${x2c_inf}.status | grep 'stime' | cut -d '=' -f 2`
    endt=$(date +"%s")
    let durition="$endt-$start"
    let h=durition/3600
    let durition%=3600
    let m=durition/60
    let s=durition%60
    else
    FORM_lip=""
    FORM_serip=""
    FORM_x2c_rx="N/A"
    FORM_x2c_tx=""
    FORM_starttime="N/A"
    h="0"
    m="0"
    s="0"
    fi
  
    let "cfgcnt+=1"
    
    get_tr
    form="$tr  
            string|<td>
            string|$cfgcnt
            string|</td>

            string|<td>
            string|$x2c
            string|</td>
    
            string|<td>
            string|$FORM_status_x2c
            string|</td>
            
            string|<td>
            string|${FORM_x2c_inf}
            string|</td>
             
            string|<td>
            string|$FORM_lip$FORM_serip
            string|</td>
    
            string|<td>
            string|$FORM_right_x2c
            string|</td>

            string|<td>
            string|$FORM_starttime
            string|</td>

            string|<td>
            string|${h}h${m}m${s}s
            string|</td>

            string|<td>
            string|$FORM_x2c_rx
            string|<br>$FORM_x2c_tx
            string|</td>"
      append forms "$form" "$N"

   if [ "$FORM_connect_status_x2c" = "Waiting..." -o "$FORM_connect_status_x2c" = "N/A" ]; then
      form="string|<td>
            string|$FORM_connect_status_x2c
            string|</td>"
   else
      form="string|<td>
            string|<input type=\"submit\" name=\"$x2c\" id=\"id_$x2c\" value=\"$FORM_connect_status_x2c\" />
            string|</td>"
   fi
      append forms "$form" "$N"
      form="string|<td>
            string|<a href=\"$SCRIPT_NAME?remove_x2c_vcfg=$x2c\">@TR<<Remove>></a>
            string|</td>

            string|<td>
            string|<a href=\"/cgi-bin/webif/ipsec-x2c-edit.sh?editname=$x2c\">@TR<<Edit>></a>
            string|</td>

            string|</tr>"

      append forms "$form" "$N"
done

get_tr
if [ "$cfgcnt" -lt "16" ]; then
form="$tr
        string|<tr><td colspan="2">
        string|<input type=\"submit\" name=\"add_x2c\" value=\"Add\" />
        string|</td></tr>
        string|</table></div>
        string|<br/>"
else
form="  string|</table></div>
        string|<br/>"
fi

append forms "$form" "$N"

cur_color="even"
#summary of xl2tp server
form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<L2TP Server>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<Client To Gateway>>\">
	string|<tr><th>@TR<<Status>></th>
        string|<th>@TR<<Interface>></th>
	string|<th>@TR<<Local IP>></th>
        string|<th>@TR<<Client IP Range Start>></th>
        string|<th>@TR<<Client IP Range End>></th>
	string|<th>@TR<<Config.>></th>
	string|</tr>"
append forms "$form" "$N"

FORM_x2s_name="Mxl2tpserver"
    config_get x2s_enabled          $FORM_x2s_name enabled
    config_get FORM_x2s_inf         $FORM_x2s_name interface
    if [ "$x2s_enabled" = "0" ]; then
    FORM_status_x2s="disable"
    FORM_server_status="N/A"
    else
    wan_up=$(/sbin/uci -P /var/state get network.wan.up)
    if [ $wan_up = "1" ]; then
    /sbin/mipsec.sh check_status_x2s br-wan >/dev/null 2>&1
    [ -f "/var/run/mipsec/Mxl2tpserver.status" ] && {
    FORM_status_x2s=`cat /var/run/mipsec/Mxl2tpserver.status | grep 'status' | cut -d '=' -f 2`
    FORM_server_status=`cat /var/run/mipsec/Mxl2tpserver.status | grep 'serstat' | cut -d '=' -f 2`
    } || {
    logger -p local0.error -t "ipsec-s2s-summary ERROR" "Mxl2tpserver.status doesn't exist"
    FORM_status_x2s="waiting for start"
    FORM_server_status="Start"
    }
    else
         FORM_status_x2s="interface down"
    FORM_server_status="N/A"

    fi             
    
    fi
    config_get FORM_localip         $FORM_x2s_name localip
    config_get FORM_startip         $FORM_x2s_name startip    
    config_get FORM_endip           $FORM_x2s_name endip
    
    get_tr
    form="  $tr
            string|<td>
            string|$FORM_status_x2s
            string|</td>

            string|<td>
            string|br-wan
            string|</td>

            string|<td>
            string|$FORM_localip
            string|</td>

            string|<td>
            string|$FORM_startip
            string|</td>

            string|<td>
            string|$FORM_endip
            string|</td>
            string|<td>
            string|<a href=\"/cgi-bin/webif/ipsec-x2s.sh\">@TR<<Edit>></a>
            string|</td>

            string|</tr>"

      append forms "$form" "$N"

FORM_x2s_name="M2xl2tpserver"
    config_get x2s_enabled          $FORM_x2s_name enabled
    if [ "$x2s_enabled" = "0" ]; then
    FORM_status_x2s="disable"
    FORM_server_status="N/A"
    else
    wan_up=$(/sbin/uci -P /var/state get network.wan2.up)
    if [ $wan_up = "1" ]; then
    /sbin/mipsec.sh check_status_x2s br-wan2 >/dev/null 2>&1
    [ -f "/var/run/mipsec/M2xl2tpserver.status" ] && {
    FORM_status_x2s=`cat /var/run/mipsec/M2xl2tpserver.status | grep 'status' | cut -d '=' -f 2`
    FORM_server_status=`cat /var/run/mipsec/M2xl2tpserver.status | grep 'serstat' | cut -d '=' -f 2`
    } || {
       
    FORM_status_x2s="waiting for start"
    FORM_server_status="Start"
    }
    else
         FORM_status_x2s="interface down"
    FORM_server_status="N/A"

    fi             
    
    fi
    config_get FORM_localip         $FORM_x2s_name localip
    config_get FORM_startip         $FORM_x2s_name startip    
    config_get FORM_endip           $FORM_x2s_name endip
    
    get_tr
    form="  $tr  
            string|<td>
            string|$FORM_status_x2s
            string|</td>

            string|<td>
            string|br-wan2
            string|</td>

            string|<td>
            string|$FORM_localip
            string|</td>

            string|<td>
            string|$FORM_startip
            string|</td>

            string|<td>
            string|$FORM_endip
            string|</td>
            string|<td>
            string|<a href=\"/cgi-bin/webif/ipsec-x2s-wan2.sh\">@TR<<Edit>></a>
            string|</td>

            string|</tr>"

      append forms "$form" "$N"

form="  string|</table></div>
        string|<br/>"

append forms "$form" "$N"

cur_color="even"
#summary of xl2tp client list
form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<L2TP Connection List>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<L2tp Connection List>>\">
	string|<tr><th>@TR<<No.>></th>
        string|<th>@TR<<Remote Address>></th>
	string|<th>@TR<<L2TP IP Address>></th>
        string|<th>@TR<<Start Time>></th>
        string|<th>@TR<<Duration>></th>
        string|<th>@TR<<RX Btyes>></th>
	string|<th>@TR<<TX Btyes>></th>
	string|</tr>"
append forms "$form" "$N"

cfgcnt="0"
inflist=`ip route show | grep $FORM_localip | grep ppp | cut -d ' ' -f 3`
[ -n "$inflist" ] && {
for inf in $inflist; do
 
    let "cfgcnt+=1"
    FORM_remotegateway=`cat /var/run/${inf}.status | grep 'remotegateway' | cut -d '=' -f 2`
    FORM_remoteip=`cat /var/run/${inf}.status | grep 'remoteip' | cut -d '=' -f 2`
    FORM_starttime=`cat /var/run/${inf}.status | grep 'starttime' | cut -d '=' -f 2`
    FORM_stime=`cat /var/run/${inf}.status | grep 'stime' | cut -d '=' -f 2`
    FORM_etime=$(date +"%s")
    let FORM_dur="$FORM_etime-$FORM_stime"
    let dur_h=FORM_dur/3600
    let FORM_dur%=3600
    let dur_m=FORM_dur/60
    let dur_s=FORM_dur%60
    FORM_rx=`ifconfig -a $inf | grep 'RX bytes' | cut -d ':' -f 2 | cut -d '(' -f 2 | cut -d ')' -f 1 | sed 's/i//g`
    FORM_tx=`ifconfig -a $inf | grep 'RX bytes' | cut -d ':' -f 3 | cut -d '(' -f 2 | cut -d ')' -f 1 | sed 's/i//g`
    get_tr
    form="$tr  
            string|<td>
            string|$cfgcnt
            string|</td>

            string|<td>
            string|$FORM_remotegateway
            string|</td>

            string|<td>
            string|$FORM_remoteip
            string|</td>

            string|<td>
            string|$FORM_starttime
            string|</td>

            string|<td>
            string|${dur_h}h${dur_m}m${dur_s}s
            string|</td>

            string|<td>
            string|$FORM_rx
            string|</td>

            string|<td>
            string|$FORM_tx
            string|</td>

            string|</tr>"

      append forms "$form" "$N"
done
}
form="  string|</table></div>
        string|<br/>"

append forms "$form" "$N"

cur_color="even"
#summary of vpn client access
form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<VPN Client Access>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<Client To Gateway>>\">
	string|<tr><th>@TR<<No.>></th>
	string|<th>@TR<<Username>></th>
	string|<th>@TR<<Config.>></th>
	string|</tr>"
append forms "$form" "$N"

cfgcnt="0"

for vca in $vca_cfgs; do
 
    let "cfgcnt+=1"
    
    get_tr
    form="$tr  
            string|<td>
            string|$cfgcnt
            string|</td>

            string|<td>
            string|$vca
            string|</td>

            string|<td>
            string|<a href=\"$SCRIPT_NAME?remove_vca_vcfg=$vca\">@TR<<Remove>></a>
            string|</td>

            string|<td>
            string|<a href=\"/cgi-bin/webif/ipsec-vca-edit.sh?editname=$vca\">@TR<<Edit>></a>
            string|</td>

            string|</tr>"

      append forms "$form" "$N"
done


get_tr
if [ "$cfgcnt" -lt "16" ]; then
form="$tr
        string|<tr><td colspan="2">
        string|<input type=\"submit\" name=\"add_vca\" value=\"Add\" />
        string|</td></tr>
        string|</table></div>
        string|<br/>"
else
form="  string|</table></div>
        string|<br/>"
fi

append forms "$form" "$N"

display_form <<EOF
$forms
EOF

echo "<div class=\"settings\" style=\"float: right;\">"
echo "<form name=\"refresh\" method=\"post\" action=\"$SCRIPT_NAME\">"

[ "$FORM_interval" -gt 0 ] >/dev/null 2>&1 && {
	cat <<EOF
<input type="submit" value=" @TR<<status_comports_Stop_Refreshing#Stop Refreshing>> " name="refreshstop" />
@TR<<status_comports_Interval#Interval>>: $FORM_interval (@TR<<status_comports_in_seconds#in seconds>>)
EOF
} || {
	cat <<EOF
<input type="submit" value=" @TR<<status_comports_Auto_Refresh#Auto Refresh>> " name="refreshstart" />
@TR<<status_comports_Interval#Interval>>:

<select name="interval">
EOF
	for sec in $(seq 3 59); do
		[ "$sec" -eq 20 ] && {
			echo "<option value=\"$sec\" selected=\"selected\">$sec</option>"
		} || {
			echo "<option value=\"$sec\">$sec</option>"
		}
	done
	cat <<EOF
</select>
@TR<<status_comports_in_seconds#in seconds>>
EOF
}
echo "</form></div>"
fi
fi
fi

footer_nosubmit ?> 
<!--
##WEBIF:name:VPN:100:Summary
-->
