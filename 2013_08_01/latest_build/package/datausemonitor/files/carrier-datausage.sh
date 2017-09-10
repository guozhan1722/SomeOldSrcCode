#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"


conf="datausage_conf"
carrier_if="br-wan2"


if empty "$FORM_submit"; then

    conf="datausage_conf"

else


if [ "$FORM_reset2zero" = "ResetToZero" ]; then

    if [ "$FORM_resethidden" = "confirm" ]; then
        /bin/datausagescript reset2zero $carrier_if &
    fi
else
 	FORM_DataUseMonitor_Set="$FORM_DataUseMonitor_Set_dm"
 	#FORM_DataUseMonitor_SetDate="$FORM_DataUseMonitor_SetDate_dm"
 	FORM_M_DataUnit_Type="$FORM_M_DataUnit_Type_dm"
 	FORM_D_DataUnit_Type="$FORM_D_DataUnit_Type_dm"
 	FORM_M_Notice_Set="$FORM_M_Notice_Set_dm"
 	FORM_M_Period_Day="$FORM_M_Period_Day_dm"
 	FORM_M_Notice_Limit="$FORM_M_Notice_Limit_dm"
 	FORM_M_SMS_Phone="$FORM_M_SMS_Phone_dm"
 	FORM_M_SMTP_Mail_Subject="$FORM_M_SMTP_Mail_Subject_dm"
 	FORM_M_SMTP_Server="$FORM_M_SMTP_Server_dm"
 	FORM_M_SMTP_User_Name="$FORM_M_SMTP_User_Name_dm"
 	FORM_M_SMTP_Password="$FORM_M_SMTP_Password_dm"
 	FORM_M_SMTP_Recipient="$FORM_M_SMTP_Recipient_dm"
 	FORM_D_Notice_Set="$FORM_D_Notice_Set_dm"
 	FORM_D_Notice_Limit="$FORM_D_Notice_Limit_dm"
 	FORM_D_SMS_Phone="$FORM_D_SMS_Phone_dm"
 	FORM_D_SMTP_Mail_Subject="$FORM_D_SMTP_Mail_Subject_dm"
 	FORM_D_SMTP_Server="$FORM_D_SMTP_Server_dm"
 	FORM_D_SMTP_User_Name="$FORM_D_SMTP_User_Name_dm"
 	FORM_D_SMTP_Password="$FORM_D_SMTP_Password_dm"
 	FORM_D_SMTP_Recipient="$FORM_D_SMTP_Recipient_dm"

validate <<EOF
int|$FORMM_M_Notice_Limit_dm|@TR<<monthly data limit>>||$FORM_M_Notice_Limit
int|$FORMM_D_Notice_Limit_dm|@TR<<daily data limit>>||$FORM_D_Notice_Limit
int|$FORMM_M_M_Period_Day_dm|@TR<<monthly period day>>||$FORM_M_Period_Day
EOF

       [ "$?" = "0" ] && {

            uci_set "datausemonitor" "$conf" "DataUseMonitor_Set" "$FORM_DataUseMonitor_Set"
            uci_set "datausemonitor" "$conf" "M_DataUnit_Type" "$FORM_M_DataUnit_Type"
            uci_set "datausemonitor" "$conf" "D_DataUnit_Type" "$FORM_D_DataUnit_Type"
            uci_set "datausemonitor" "$conf" "M_Period_Day" "$FORM_M_Period_Day"
            if [ "$FORM_DataUseMonitor_Set" = "B" ]; then
                uci_set "datausemonitor" "$conf" "M_Notice_Set" "$FORM_M_Notice_Set"
                if [ "$FORM_M_Notice_Set" = "B" ]; then
                    uci_set "datausemonitor" "$conf" "M_Notice_Limit" "$FORM_M_Notice_Limit"
                    uci_set "datausemonitor" "$conf" "M_SMS_Phone" "$FORM_M_SMS_Phone"
                fi
                if [ "$FORM_M_Notice_Set" = "C" ]; then
                    uci_set "datausemonitor" "$conf" "M_Period_Day" "$FORM_M_Period_Day"
                    uci_set "datausemonitor" "$conf" "M_Notice_Limit" "$FORM_M_Notice_Limit"
                    uci_set "datausemonitor" "$conf" "M_SMTP_Mail_Subject" "$FORM_M_SMTP_Mail_Subject"
                    uci_set "datausemonitor" "$conf" "M_SMTP_Server" "$FORM_M_SMTP_Server"
                    uci_set "datausemonitor" "$conf" "M_SMTP_User_Name" "$FORM_M_SMTP_User_Name"
                    uci_set "datausemonitor" "$conf" "M_SMTP_Password" "$FORM_M_SMTP_Password"
                    uci_set "datausemonitor" "$conf" "M_SMTP_Recipient" "$FORM_M_SMTP_Recipient"
                fi

                uci_set "datausemonitor" "$conf" "D_Notice_Set" "$FORM_D_Notice_Set"
                if [ "$FORM_D_Notice_Set" = "B" ]; then
                    uci_set "datausemonitor" "$conf" "D_Notice_Limit" "$FORM_D_Notice_Limit"
                    uci_set "datausemonitor" "$conf" "D_SMS_Phone" "$FORM_D_SMS_Phone"
                fi
                if [ "$FORM_D_Notice_Set" = "C" ]; then
                    uci_set "datausemonitor" "$conf" "D_Notice_Limit" "$FORM_D_Notice_Limit"
                    uci_set "datausemonitor" "$conf" "D_SMTP_Mail_Subject" "$FORM_D_SMTP_Mail_Subject"
                    uci_set "datausemonitor" "$conf" "D_SMTP_Server" "$FORM_D_SMTP_Server"
                    uci_set "datausemonitor" "$conf" "D_SMTP_User_Name" "$FORM_D_SMTP_User_Name"
                    uci_set "datausemonitor" "$conf" "D_SMTP_Password" "$FORM_D_SMTP_Password"
                    uci_set "datausemonitor" "$conf" "D_SMTP_Recipient" "$FORM_D_SMTP_Recipient"
                fi
            fi

            FORM_DataUseMonitor_SetDate=$(date)
            uci_set "datausemonitor" "$conf" "DataUseMonitor_SetDate" "$FORM_DataUseMonitor_SetDate"
         }
fi
fi

uci_load datausemonitor
conf="datausage_conf"

       config_get FORM_DataUseMonitor_Set $conf DataUseMonitor_Set "A"
       config_get FORM_DataUseMonitor_SetDate $conf DataUseMonitor_SetDate "2013-6-20"
       config_get FORM_M_DataUnit_Type $conf M_DataUnit_Type "M"
       config_get FORM_D_DataUnit_Type $conf D_DataUnit_Type "M"
        
       config_get FORM_M_Notice_Set $conf M_Notice_Set "A"
       config_get FORM_M_Period_Day $conf M_Period_Day "1"
       config_get FORM_M_Notice_Limit $conf M_Notice_Limit "1000"
       config_get FORM_M_SMS_Phone $conf M_SMS_Phone "+1403"
       config_get FORM_M_SMTP_Mail_Subject $conf M_SMTP_Mail_Subject "Monthly Data Usage Notice"
       config_get FORM_M_SMTP_Server $conf M_SMTP_Server "smtp.gmail.com:465"
       config_get FORM_M_SMTP_User_Name $conf M_SMTP_User_Name "@gmail.com"
       config_get FORM_M_SMTP_Password $conf M_SMTP_Password "xxx"
       config_get FORM_M_SMTP_Recipient $conf M_SMTP_Recipient "host@"

       config_get FORM_D_Notice_Set $conf D_Notice_Set "A"
       config_get FORM_D_Notice_Limit $conf D_Notice_Limit "50"
       config_get FORM_D_SMS_Phone $conf D_SMS_Phone "+1403"
       config_get FORM_D_SMTP_Mail_Subject $conf D_SMTP_Mail_Subject "Monthly Data Usage Notice"
       config_get FORM_D_SMTP_Server $conf D_SMTP_Server "smtp.gmail.com:465"
       config_get FORM_D_SMTP_User_Name $conf D_SMTP_User_Name "@gmail.com"
       config_get FORM_D_SMTP_Password $conf D_SMTP_Password "xxx"
       config_get FORM_D_SMTP_Recipient $conf D_SMTP_Recipient "host@"

config_options="
start_form| @TR<<Data Usage Monitor>>
field|@TR<< <strong>Status</strong> >>|DataUseMonitor_Set_dm_field|1
select|DataUseMonitor_Set_dm|$FORM_DataUseMonitor_Set
option|A|@TR<<Disable>>
option|B|@TR<<Enable Data Usage Monitor>>

field|@TR<< &nbsp;&nbsp;Last Config Time>>|DataUseMonitor_SetDate_dm_field|hidden
string| $FORM_DataUseMonitor_SetDate 

field|@TR<< <strong>Monthly Over Limit</strong> >>|M_Notice_Set_dm_field|hidden
select|M_Notice_Set_dm|$FORM_M_Notice_Set
option|A|@TR<<None>>
option|B|@TR<<Send Notice SMS>>
option|C|@TR<<Send Notice Email>>

field|@TR<< &nbsp;&nbsp;Monthly Data Units>>|M_DataUnit_Type_dm_field|hidden
select|M_DataUnit_Type_dm|$FORM_M_DataUnit_Type
option|B|@TR<<Bytes>>
option|K|@TR<<K Bytes>>
option|M|@TR<<M Bytes>>
option|G|@TR<<G Bytes>>

field|@TR<<&nbsp;&nbsp;Data Limit>>|M_Notice_Limit_dm_field|hidden
text|M_Notice_Limit_dm|$FORM_M_Notice_Limit|@TR<<[1~65535]>>

field|@TR<<&nbsp;&nbsp;Period Start Day>>|M_Period_Day_dm_field|hidden
text|M_Period_Day_dm|$FORM_M_Period_Day|@TR<<[1~31](day of month)>>

field|@TR<<&nbsp;&nbsp;Phone Number>>|M_SMS_Phone_dm_field|hidden
text|M_SMS_Phone_dm|$FORM_M_SMS_Phone|@TR<< >>

field|@TR<<&nbsp;&nbsp;Mail Subject>>|M_SMTP_Mail_Subject_dm_field|hidden
text|M_SMTP_Mail_Subject_dm|$FORM_M_SMTP_Mail_Subject|@TR<<>>

field|@TR<<&nbsp;&nbsp;Mail Server(IP/Name)>>|M_SMTP_Server_dm_field|hidden
text|M_SMTP_Server_dm|$FORM_M_SMTP_Server|@TR<<(xxx:port)>>

field|@TR<<&nbsp;&nbsp;User Name>>|M_SMTP_User_Name_dm_field|hidden
text|M_SMTP_User_Name_dm|$FORM_M_SMTP_User_Name|@TR<<>>

field|@TR<<&nbsp;&nbsp;Password>>|M_SMTP_Password_dm_field|hidden
password|M_SMTP_Password_dm|$FORM_M_SMTP_Password|@TR<<>>

field|@TR<<&nbsp;&nbsp;Mail Recipient>>|M_SMTP_Recipient_dm_field|hidden
text|M_SMTP_Recipient_dm|$FORM_M_SMTP_Recipient|@TR<<(xx@xx.xx)>>


field|@TR<< <strong>Daily Over Limit</strong> >>|D_Notice_Set_dm_field|hidden
select|D_Notice_Set_dm|$FORM_D_Notice_Set
option|A|@TR<<None>>
option|B|@TR<<Send Notice SMS>>
option|C|@TR<<Send Notice Email>>

field|@TR<< &nbsp;&nbsp;Daily Data Units>>|D_DataUnit_Type_dm_field|hidden
select|D_DataUnit_Type_dm|$FORM_D_DataUnit_Type
option|B|@TR<<Bytes>>
option|K|@TR<<K Bytes>>
option|M|@TR<<M Bytes>>
option|G|@TR<<G Bytes>>

field|@TR<<&nbsp;&nbsp;Data Limit>>|D_Notice_Limit_dm_field|hidden
text|D_Notice_Limit_dm|$FORM_D_Notice_Limit|@TR<<[1~65535]>>

field|@TR<<&nbsp;&nbsp;Phone Number>>|D_SMS_Phone_dm_field|hidden
text|D_SMS_Phone_dm|$FORM_D_SMS_Phone|@TR<< >>

field|@TR<<&nbsp;&nbsp;Mail Subject>>|D_SMTP_Mail_Subject_dm_field|hidden
text|D_SMTP_Mail_Subject_dm|$FORM_D_SMTP_Mail_Subject|@TR<<>>

field|@TR<<&nbsp;&nbsp;Mail Server(IP/Name)>>|D_SMTP_Server_dm_field|hidden
text|D_SMTP_Server_dm|$FORM_D_SMTP_Server|@TR<<(xxx:port)>>

field|@TR<<&nbsp;&nbsp;User Name>>|D_SMTP_User_Name_dm_field|hidden
text|D_SMTP_User_Name_dm|$FORM_D_SMTP_User_Name|@TR<<>>

field|@TR<<&nbsp;&nbsp;Password>>|D_SMTP_Password_dm_field|hidden
password|D_SMTP_Password_dm|$FORM_D_SMTP_Password|@TR<<>>

field|@TR<<&nbsp;&nbsp;Mail Recipient>>|D_SMTP_Recipient_dm_field|hidden
text|D_SMTP_Recipient_dm|$FORM_D_SMTP_Recipient|@TR<<(xx@xx.xx)>>

end_form
"

append forms "$config_options" "$N"


######################################################################################
# set JavaScript
# set_visible('Pos_Record_NUMS_dm_field', 0);

javascript_forms="
    v_en = isset('DataUseMonitor_Set_dm','B');

    set_visible('DataUseMonitor_SetDate_dm_field', v_en);
    set_visible('M_DataUnit_Type_dm_field', v_en);
    set_visible('D_DataUnit_Type_dm_field', v_en);
    set_visible('M_Period_Day_dm_field', v_en);
    set_visible('M_Notice_Set_dm_field', v_en);
    v_M_comm=0;
    v_M_sms=0;
    v_M_email=0;
    v_M_set=isset('M_Notice_Set_dm','B');
    if(v_en&&v_M_set)
    {
        v_M_comm=1;
        v_M_sms=1;
    }
    v_M_set=isset('M_Notice_Set_dm','C');
    if(v_en&&v_M_set)
    {
        v_M_comm=1;
        v_M_email=1;
    }
    set_visible('M_Notice_Limit_dm_field', v_M_comm);
    set_visible('M_SMS_Phone_dm_field', v_M_sms);
    set_visible('M_SMTP_Mail_Subject_dm_field', v_M_email);
    set_visible('M_SMTP_Server_dm_field', v_M_email);
    set_visible('M_SMTP_User_Name_dm_field', v_M_email);
    set_visible('M_SMTP_Password_dm_field', v_M_email);
    set_visible('M_SMTP_Recipient_dm_field', v_M_email);

    set_visible('D_Notice_Set_dm_field', v_en);
    v_D_comm=0;
    v_D_sms=0;
    v_D_email=0;
    v_D_set=isset('D_Notice_Set_dm','B');
    if(v_en&&v_D_set)
    {
        v_D_comm=1;
        v_D_sms=1;
    }
    v_D_set=isset('D_Notice_Set_dm','C');
    if(v_en&&v_D_set)
    {
        v_D_comm=1;
        v_D_email=1;
    }
    set_visible('D_Notice_Limit_dm_field', v_D_comm);
    set_visible('D_SMS_Phone_dm_field', v_D_sms);
    set_visible('D_SMTP_Mail_Subject_dm_field', v_D_email);
    set_visible('D_SMTP_Server_dm_field', v_D_email);
    set_visible('D_SMTP_User_Name_dm_field', v_D_email);
    set_visible('D_SMTP_Password_dm_field', v_D_email);
    set_visible('D_SMTP_Recipient_dm_field', v_D_email);

"

append js "$javascript_forms" "$N"



#####################################################################
header "Carrier" "Data Usage" "@TR<<Data Usage Monitor>>" ' onload="modechange()" ' "$SCRIPT_NAME"
#####################################################################

# modechange script
#
cat <<EOF
<script type="text/javascript" src="/webif.js"></script>
<script type="text/javascript">
<!--
function modechange()
{
	var v;
	$js

	hide('save');
	show('save');
}
-->
</script>

<script type="text/javascript">
<!--
function confirmation() {
var answer = confirm("Are you sure reset all datausage record to ZERO?\nIt is not a recoverable process.")
if (answer){
document.getElementById('resethidden').value="confirm";
document.getElementById('reset2zero').click();
return true;
}
return false;
}
-->
</script>

<div class="settings">
<h3><strong>Data Usage Statistic</strong></h3>
<div class="settings-content">
<table width="100%" summary="Settings">
<tr id="fielden">
<td width="40%"></td><td width="60%"></td></tr>
EOF

/bin/datausemonitor $carrier_if readout

cat /var/run/datausagestat <<EOF
EOF

#if [ "$FORM_DataUseMonitor_Set" = "A" ]; then
cat <<EOF
<tr><td>Reset and Clear all Record:</td><td>
<fieldset id="resetset"  style="display: none">
<div class="page-save"><input id="reset2zero" type="submit" name="reset2zero" value="ResetToZero" /></div></fieldset>
<input type=hidden id=resethidden name=resethidden value="null">
<input type="button" name=resetzero onclick="confirmation()" value=" Reset Record To Zero ">
</td></tr>
EOF


cat <<EOF
<tr><td colspan=2>
Attention:Data usage statistic is not exact same to your carrier's caculation on your monthly bill with different systems.
</td></tr>
</table>
</div>
<div class="clearfix"></div></div>
EOF


display_form <<EOF
onchange|modechange
$validate_error
$forms
EOF



footer ?>
<!--
##WEBIF:name:Carrier:800:Data Usage
-->
