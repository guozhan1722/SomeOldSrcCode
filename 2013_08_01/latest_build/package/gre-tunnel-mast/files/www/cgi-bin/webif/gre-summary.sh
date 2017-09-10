#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
    local cfg_type="$1"
    local cfg_name="$2"

    case "$cfg_type" in
        gretunnel)
            append gre_cfgs "$cfg_name" "$N"
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
#remove a gre tunnel
if ! empty "$FORM_remove_gre_vcfg"; then
	#uci_remove "gre-tunnels" "$FORM_remove_gre_vcfg"
        #/usr/lib/webif/apply.sh del_gre "$FORM_remove_gre_vcfg" >/dev/null 2>&1
        /bin/gre del "$FORM_remove_gre_vcfg" >/dev/null 2>&1
fi

#add a gre tunnel
if ! empty "$FORM_add_gre"; then
        header "Network" "GRE" "@TR<<Summary>>" '' "$SCRIPT_NAME"
        echo "<meta http-equiv=\"refresh\" content=\"0; url=/cgi-bin/webif/network-gre.sh\" />"
else

uci_load gre-tunnels
for gre in $gre_cfgs; do
    
    eval FORM_connect="\$FORM_${gre}"
    
  if ! empty "$FORM_connect" ; then
       [ -f "/etc/debugipsec" ] && logger -p local0.error -t "gre-summary.sh" "$FORM_connect is not empty and action"
       case "$FORM_connect" in
             Connect)
                     echo "status=waiting for connection" > /var/run/mipsec/${gre}.status
                     echo "constat=Waiting..." >> /var/run/mipsec/${gre}.status
                     echo "action=up" >> /var/run/mipsec/${gre}.status
                     /sbin/mipsec.sh connect_gre $gre >/dev/null 2>&1 &
             ;;
             Disconnect|Waiting...)
                     echo "status=waiting for connection" > /var/run/mipsec/${gre}.status
                     echo "constat=Waiting..." >> /var/run/mipsec/${gre}.status
                     echo "action=down" >> /var/run/mipsec/${gre}.status
                     /sbin/mipsec.sh disconnect_gre $gre >/dev/null 2>&1
             ;;
             *)
             ;;
        esac
  fi
  
done    
    
#summary of gre tunnels
form="  string|<div class=\"settings\">
	string|<table style=\"width: 100%; margin-left: 0.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<GRE Configuration Summary>>\">
	string|<th>@TR<<No.>></th>
	string|<th>@TR<<Name>></th>
	string|<th>@TR<<Status>></th>
	string|<th>@TR<<Multicast>></th>
	string|<th>@TR<<ARP>></th>
	string|<th>@TR<<TTL>></th>
        string|<th>@TR<<IPsec>></th>
	string|<th>@TR<<Local Tunnel IP>></th>
	string|<th>@TR<<Local Gateway>></th>
        string|<th>@TR<<Local Subnet>></th>
        string|<th>@TR<<Remote Gateway>></th>
        string|<th>@TR<<Remote Subnet>></th>
        string|<th>@TR<<RX/TX Bytes>></th>
        string|<th>@TR<<Tunnel Test>></th>
	string|<th>@TR<<Config.>></th>
	string|</tr>"
append forms "$form" "$N"

cfgcnt="0"

for gre in $gre_cfgs; do
    config_get FORM_local_status            $gre local_status    
    config_get FORM_multicast               $gre multicast    
    config_get FORM_arp                     $gre arp    
    config_get FORM_ttl                     $gre ttl   
    config_get FORM_local_tunnel_ip         $gre local_tunnel_ip
    config_get FORM_local_tunnel_mask       $gre local_tunnel_mask
    config_get FORM_local_wan               $gre local_wan  
    config_get FORM_local_subnet_ip         $gre local_subnet_ip
    config_get FORM_local_subnet_mask       $gre local_subnet_mask
    config_get FORM_remote_subnet           $gre remote_subnet
    config_get FORM_remote_subnet_mask      $gre remote_netmask   
    config_get FORM_remote_wan              $gre remote_wan 
    config_get FORM_enableipsec             $gre enableipsec
    config_get FORM_interface               $gre interface   
    let "cfgcnt+=1"
    if [ "$FORM_local_status" = "Disable" ]; then
           FORM_connect_status="N/A"
           FORM_grx="N/A"
           FORM_gtx=""
           FORM_status="Disable"
    else
           if [ "$FORM_enableipsec" = "Disable" ]; then
                 FORM_connect_status="N/A"
                 FORM_status="Enable"
           else
                 [ -f "/var/run/mipsec/${gre}.status" ] && {
                      FORM_connect_status=`cat /var/run/mipsec/${gre}.status | grep 'constat' | cut -d '=' -f 2`
                      FORM_status=`cat /var/run/mipsec/${gre}.status | grep 'status' | cut -d '=' -f 2`
                 } || {
                      FORM_connect_status="Connect"
                      FORM_status="waiting for connection"
                 }
           fi 
           FORM_grx=`ifconfig -a $gre | grep 'RX bytes' | cut -d ':' -f 2 | cut -d '(' -f 2 | cut -d ')' -f 1 | sed 's/i//g`
           FORM_gtx=`ifconfig -a $gre | grep 'RX bytes' | cut -d ':' -f 3 | cut -d '(' -f 2 | cut -d ')' -f 1 | sed 's/i//g`            
    fi 

    case "$FORM_enableipsec" in
    Disable)
            ipsec_enable="Disable"
            ;;
    ags)
            ipsec_enable="Tunnel mode</br>over GRE"
            ;;
    bgs)
           config_get ipsec_mode $gre ipsec_mode
           if [ $ipsec_mode = "tunnel" ]
           then
                ipsec_enable="GRE over</br>Tunnel mode"
           else
                ipsec_enable="GRE over</br>Transport mode"
           fi
           ;;
    esac                  
    
    get_tr
    form="$tr  
            string|<td>
            string|$cfgcnt
            string|</td>

            string|<td>
            string|$gre
            string|</td>
    
            string|<td>
            string|${FORM_status}
            string|</td>
    
            string|<td>
            string|$FORM_multicast
            string|</td>
    
            string|<td>
            string|$FORM_arp
            string|</td>
    
            string|<td>
            string|$FORM_ttl
            string|</td>
    
            string|<td>
            string|$ipsec_enable
            string|</td>
                   
            string|<td>
            string|$FORM_local_tunnel_ip
            string|<br>$FORM_local_tunnel_mask
            string|</td>

            string|<td>
            string|$FORM_local_wan
            string|</td>

            string|<td>
            string|$FORM_local_subnet_ip
            string|<br>$FORM_local_subnet_mask
            string|</td>

            string|<td>
            string|$FORM_remote_wan
            string|</td>

            string|<td>
            string|$FORM_remote_subnet
            string|<br>$FORM_remote_subnet_mask
            string|</td>

            string|<td>
            string|$FORM_grx
            string|<br>$FORM_gtx
            string|</td>"
      append forms "$form" "$N"

   if [ "$FORM_connect_status" = "Waiting..." -o "$FORM_connect_status" = "N/A" ]; then
      form="string|<td>
            string|$FORM_connect_status
            string|</td>"
   else
      form="string|<td>
            string|<input type=\"submit\" name=\"$gre\" id=\"id_$gre\" value=\"$FORM_connect_status\" />
            string|</td>"
   fi
      append forms "$form" "$N"
      form="string|<td>
            string|<a href=\"$SCRIPT_NAME?remove_gre_vcfg=$gre\">@TR<<Remove>></a>
            string|</td>

            string|<td>
            string|<a href=\"/cgi-bin/webif/network-gre-edit.sh?editname=$gre&eipsec=$FORM_enableipsec&oinf=$FORM_interface\">@TR<<Edit>></a>
            string|</td>

            string|</tr>"

      append forms "$form" "$N"
done

get_tr
if [ "$cfgcnt" -lt "16" ]; then
form="$tr
        string|<tr><td colspan="2">
        string|<input type=\"submit\" name=\"add_gre\" value=\"Add\" />
        string|</td></tr>
        string|</table></div>
        string|<br/>"
else
form="  string|</table></div>
        string|<br/>"
fi

append forms "$form" "$N"
sysmode=$(uci get system.@system[0].systemmode)

if [ "$sysmode" = "gateway" ]; then
header_inject_head=$(cat <<EOF
$meta_refresh
EOF
)
header "Network" "GRE" "@TR<<Summary>>" '' "$SCRIPT_NAME"
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

else
header "Network" "GRE" "@TR<<Summary>>" '' "$SCRIPT_NAME"
display_form <<EOF
string|<tr><td colspan=\"2\"><h3 class=\"warning\">No GRE Configuration in Bridge mode and Router mode </h3></td></tr>
EOF
fi

fi

footer_nosubmit ?> 
<!--
##WEBIF:name:Network:600:GRE
-->
