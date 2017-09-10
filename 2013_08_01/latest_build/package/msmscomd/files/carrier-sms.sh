#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"


   conf="msms_conf"


    if  [ "$FORM_deleteid" ];  then
        conf="msms_conf"
        /bin/msmsif delsms $FORM_deleteid &
        sleep 1
	meta_refresh="<meta http-equiv=\"refresh\" content=\"10;url=$SCRIPT_NAME\" />"
header_inject_head=$(cat <<EOF
$meta_refresh
EOF
)

    fi
    if  [ "$FORM_delall" = "Delete All Above SMS" ];  then
        /bin/msmsif delall 0 999 &
        sleep 1
	meta_refresh="<meta http-equiv=\"refresh\" content=\"10;url=$SCRIPT_NAME\" />"
header_inject_head=$(cat <<EOF
$meta_refresh
EOF
)

    fi
    if  [ "$FORM_submit_send" = "Submit" ];  then
        tmp_char=""
        tmp_char=$(echo $FORM_send_to | cut -c4)
        if [ "$tmp_char" ]; then
            echo "#$FORM_send_to" > /var/run/msmssend
            echo "$FORM_smscontent" >> /var/run/msmssend
            /bin/msmsif sendsms /var/run/msmssend
            Send_Msg_Error="Finished send to:$FORM_send_to <br>Send text: $FORM_smscontent <br><br><strong>New SMS</strong><br>"
            FORM_replyid="Send New SMS"
        else
            Send_Msg_Error="<font color=#FF0000>Send SMS Error. Please check your phone number.</font>"
            send_to_num=$FORM_send_to
            send_text=$FORM_smscontent
            FORM_replyid="1"
        fi
    fi


if  [ "$FORM_replyid" ];  then
#http://192.168.168.1/cgi-bin/webif/carrier-sms.sh?replyid=0#92466601
#####################################################################
header "Carrier" "SMS" "@TR<<SMS Send>>"  "$SCRIPT_NAME"
#####################################################################
    if  [ "$FORM_replyid" = "Send New SMS" ];  then
        send_to_num="+1"
    else
        tmp_char=""
        tmp_char=$(echo $FORM_replyid | cut -c2)
        if [ "$tmp_char" = "P" ]; then
            tmp_char=$(echo $FORM_replyid | cut -c3)
            if [ "$tmp_char" = "Q" ]; then
                send_to_num="+"$(echo $FORM_replyid | cut -c4-)
            else
                send_to_num=$(echo $FORM_replyid | cut -c3-)
            fi
        fi

        tmp_char=$(echo $FORM_replyid | cut -c3)
        if [ "$tmp_char" = "P" ]; then
            tmp_char=$(echo $FORM_replyid | cut -c4)
            if [ "$tmp_char" = "Q" ]; then
                send_to_num="+"$(echo $FORM_replyid | cut -c5-)
            else
                send_to_num=$(echo $FORM_replyid | cut -c4-)
            fi
        fi

        tmp_char=$(echo $FORM_replyid | cut -c4)
        if [ "$tmp_char" = "P" ]; then
            tmp_char=$(echo $FORM_replyid | cut -c5)
            if [ "$tmp_char" = "Q" ]; then
                send_to_num="+"$(echo $FORM_replyid | cut -c6-)
            else
                send_to_num=$(echo $FORM_replyid | cut -c5-)
            fi
        fi

    fi

cat <<EOF
<div class="settings"> <form name='carriersms' method='post' action='carrier-sms.sh'>
$Send_Msg_Error
<table style="width: 60%; margin-left: 0em; text-align: left; font-size: 1em;" border="0" cellpadding="2" cellspacing="2" summary="Settings">
<tr>
	<td width=10%>Send To:</td>
	<td><input type='text' name='send_to' value='$send_to_num' maxlength='30' size='20'></td>
</tr>
<tr>
	<td>Text:</td>
	<td><textarea rows='3' cols='60' id='smscontent' name='smscontent' value=''>$send_text</textarea></td>
</tr>
<tr>
	<td></td>
        <td>&nbsp; &nbsp;  &nbsp; &nbsp;  <input type='submit' name='submit_send' value='Submit'> &nbsp; &nbsp;  &nbsp; &nbsp;  <input type='submit' name='submit_reset' value='Cancel'></td>
          </td>
</tr>
</table></div>

<div class="content">
<th><h3></h3></th>
<h2>SMS Send History</h2>
<table style="width: 100%; margin-left: 1em; text-align: left; font-size: 1em;" border="0" cellpadding="2" cellspacing="2" summary="Settings">
<tr>
	<td width=12%>Send To</td>
	<td width=23%>Send Time</td>
	<td width=30%>Content</td>
	<td>Result</td>
</tr>
	
EOF

cat /etc/smssend.history <<EOF
EOF

cat <<EOF
</table>
</form></div>
EOF


else
if  [ "$FORM_views" = "alertrecord" ];  then
#http://192.168.168.1/cgi-bin/webif/carrier-sms.sh?replyid=0#92466601
#####################################################################
header "Carrier" "SMS" "@TR<<SMS Alert Sending History>>"  "$SCRIPT_NAME"
#####################################################################


cat <<EOF
<div class="settings"> 
<table style="width: 100%; margin-left: 0em; text-align: left; font-size: 1em;" border="0" cellpadding="4" cellspacing="2" summary="Settings">
<tr>
	<td width=7%>Item</td>
	<td width=23%>Send Time</td>
	<td width=20%>Content</td>
	<td>Phone Lists</td>
</tr>
EOF

cat /etc/smsalert.historybak <<EOF
EOF

cat /etc/smsalert.history <<EOF
EOF


cat <<EOF
</table>
</div>
EOF

else


uci_load "msmscomd"
       config_get FORM_System_SMS_Commad "msms_conf" System_SMS_Commad
if [ "$FORM_System_SMS_Commad" = "B" ]; then
   conf="msms_conf"    
else
   if [ "$FORM_refresh" = "read" ]; then
       conf="msms_conf"    
   else
    /bin/msmsif readsms 4 &
    header_inject_head="<meta http-equiv=\"refresh\" content=\"10;url=carrier-sms.sh?refresh=read\" />"
    tmp_info="<tr><td colspan=4>Reading sms in sim card. Waiting to refresh...</font></td></tr>"
   fi
fi

#####################################################################
header "Carrier" "SMS" "@TR<<SMS Command History>>"  "$SCRIPT_NAME"
#####################################################################

# modechange script
#
cat <<EOF
<div class="settings">
<form name='carriersms' method='post' action='carrier-sms.sh'>
<table style="width: 100%; margin-left: 0em; text-align: left; font-size: 1em;" border="0" cellpadding="4" cellspacing="2" summary="Settings">
<tr>
	<td width=12%>From</td>
	<td width=23%>Send Time</td>
	<td width=15%>Content</td>
	<td>Result</td>
</tr>
EOF


cat /etc/smscmd.history <<EOF
EOF


cat <<EOF
</table>
</div>
<div class="settings">
<th><h3></h3></th>
<h2>SMS Untreated In SIM Card</h2>
<table style="width: 100%; margin-left: 0em; text-align: left; font-size: 1em;" border="0" cellpadding="4" cellspacing="2" summary="Settings">
<tr>
	<td width=3%>No.</td>
	<td width=12%>From</td>
	<td width=23%>Time</td>
	<td>Content</td>
</tr>
	
EOF

if [ "$tmp_info" ]; then
cat <<EOF
$tmp_info
<tr><td></td><td></td><td></td>
<td><input type=submit name=replyid value="Send New SMS"></td></tr>
</table>
</form>
</div>

EOF
else
cat /var/run/msmsread <<EOF
EOF
cat <<EOF
<tr><td></td><td></td><td></td>
<td><input type=submit name=delall value="Delete All Above SMS">&nbsp;&nbsp;  &nbsp;&nbsp; <input type=submit name=replyid value="Send New SMS"></td></tr>
</table>
</form>
</div>
EOF
fi


fi
fi

footer_nosubmit 
?>


<!--
##WEBIF:name:Carrier:700:SMS
-->
