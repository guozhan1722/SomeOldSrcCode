#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"

uci_load gpsd
conf="gpsd_conf"
config_get FORM_status $conf status
if [ "$FORM_status" = "1" ]; then



uci_load gpsr
conf="gpsr_conf"

if empty "$FORM_submit"; then

    conf="gpsr_conf"

else

 	FORM_AGCR_Remote_IP_address0="$FORM_AGCR_Remote_IP_address0_gpsr"
 	FORM_AGCR_Remote_IP_address1="$FORM_AGCR_Remote_IP_address1_gpsr"
 	FORM_AGCR_Remote_IP_address2="$FORM_AGCR_Remote_IP_address2_gpsr"
 	FORM_AGCR_Remote_IP_address3="$FORM_AGCR_Remote_IP_address3_gpsr"
 	FORM_AGCR_Remote_PORT0="$FORM_AGCR_Remote_PORT0_gpsr"
 	FORM_AGCR_Remote_PORT1="$FORM_AGCR_Remote_PORT1_gpsr"
 	FORM_AGCR_Remote_PORT2="$FORM_AGCR_Remote_PORT2_gpsr"
 	FORM_AGCR_Remote_PORT3="$FORM_AGCR_Remote_PORT3_gpsr"
 	FORM_AGCR_Timer0="$FORM_AGCR_Timer0_gpsr"
 	FORM_AGCR_Timer1="$FORM_AGCR_Timer1_gpsr"
 	FORM_AGCR_Timer2="$FORM_AGCR_Timer2_gpsr"
 	FORM_AGCR_Timer3="$FORM_AGCR_Timer3_gpsr"
 	FORM_AGCR_Distance0="$FORM_AGCR_Distance0_gpsr"
 	FORM_AGCR_Distance1="$FORM_AGCR_Distance1_gpsr"
 	FORM_AGCR_Distance2="$FORM_AGCR_Distance2_gpsr"
 	FORM_AGCR_Distance3="$FORM_AGCR_Distance3_gpsr"
 	FORM_SMTP0_Mail_Subject="$FORM_SMTP0_Mail_Subject_gpsr"
 	FORM_SMTP0_Server="$FORM_SMTP0_Server_gpsr"
 	FORM_SMTP0_User_Name="$FORM_SMTP0_User_Name_gpsr"
 	FORM_SMTP0_Password="$FORM_SMTP0_Password_gpsr"
 	FORM_SMTP0_Recipient="$FORM_SMTP0_Recipient_gpsr"
# 	FORM_SMTP0_Buffer="$FORM_SMTP0_Buffer_gpsr"
 	FORM_SMTP1_Mail_Subject="$FORM_SMTP1_Mail_Subject_gpsr"
 	FORM_SMTP1_Server="$FORM_SMTP1_Server_gpsr"
 	FORM_SMTP1_User_Name="$FORM_SMTP1_User_Name_gpsr"
 	FORM_SMTP1_Password="$FORM_SMTP1_Password_gpsr"
 	FORM_SMTP1_Recipient="$FORM_SMTP1_Recipient_gpsr"
# 	FORM_SMTP1_Buffer="$FORM_SMTP1_Buffer_gpsr"
 	FORM_SMTP2_Mail_Subject="$FORM_SMTP2_Mail_Subject_gpsr"
 	FORM_SMTP2_Server="$FORM_SMTP2_Server_gpsr"
 	FORM_SMTP2_User_Name="$FORM_SMTP2_User_Name_gpsr"
 	FORM_SMTP2_Password="$FORM_SMTP2_Password_gpsr"
 	FORM_SMTP2_Recipient="$FORM_SMTP2_Recipient_gpsr"
# 	FORM_SMTP2_Buffer="$FORM_SMTP2_Buffer_gpsr"
 	FORM_SMTP3_Mail_Subject="$FORM_SMTP3_Mail_Subject_gpsr"
 	FORM_SMTP3_Server="$FORM_SMTP3_Server_gpsr"
 	FORM_SMTP3_User_Name="$FORM_SMTP3_User_Name_gpsr"
 	FORM_SMTP3_Password="$FORM_SMTP3_Password_gpsr"
 	FORM_SMTP3_Recipient="$FORM_SMTP3_Recipient_gpsr"
# 	FORM_SMTP3_Buffer="$FORM_SMTP3_Buffer_gpsr"


 	FORM_AGCR_Remote_Reporting_Status_0="$FORM_AGCR_Remote_Reporting_Status_0_gpsr"
 	FORM_AGCR_Remote_Reporting_Status_1="$FORM_AGCR_Remote_Reporting_Status_1_gpsr"
 	FORM_AGCR_Remote_Reporting_Status_2="$FORM_AGCR_Remote_Reporting_Status_2_gpsr"
 	FORM_AGCR_Remote_Reporting_Status_3="$FORM_AGCR_Remote_Reporting_Status_3_gpsr"
        FORM_AGCR_Remote_Reporting_Status=$FORM_AGCR_Remote_Reporting_Status_0$FORM_AGCR_Remote_Reporting_Status_1$FORM_AGCR_Remote_Reporting_Status_2$FORM_AGCR_Remote_Reporting_Status_3

# 	FORM_AGCR_Timer_trigger_0="$FORM_AGCR_Timer_trigger_0_gpsr"
# 	FORM_AGCR_Timer_trigger_1="$FORM_AGCR_Timer_trigger_1_gpsr"
# 	FORM_AGCR_Timer_trigger_2="$FORM_AGCR_Timer_trigger_2_gpsr"
# 	FORM_AGCR_Timer_trigger_3="$FORM_AGCR_Timer_trigger_3_gpsr"
# 	FORM_AGCR_Timer_trigger=$FORM_AGCR_Timer_trigger_0$FORM_AGCR_Timer_trigger_1$FORM_AGCR_Timer_trigger_2$FORM_AGCR_Timer_trigger_3
#       #same to report status's setting.
        if [ "$FORM_AGCR_Remote_Reporting_Status_0" = "A" ]; then
                    FORM_AGCR_Remote_Reporting_Status_t0="A"
        else
                    FORM_AGCR_Remote_Reporting_Status_t0="B"
        fi
        if [ "$FORM_AGCR_Remote_Reporting_Status_1" = "A" ]; then
                    FORM_AGCR_Remote_Reporting_Status_t1="A"
        else
                    FORM_AGCR_Remote_Reporting_Status_t1="B"
        fi
        if [ "$FORM_AGCR_Remote_Reporting_Status_2" = "A" ]; then
                    FORM_AGCR_Remote_Reporting_Status_t2="A"
        else
                    FORM_AGCR_Remote_Reporting_Status_t2="B"
        fi
        if [ "$FORM_AGCR_Remote_Reporting_Status_3" = "A" ]; then
                    FORM_AGCR_Remote_Reporting_Status_t3="A"
        else
                    FORM_AGCR_Remote_Reporting_Status_t3="B"
        fi
        FORM_AGCR_Timer_trigger="${FORM_AGCR_Remote_Reporting_Status_t0}${FORM_AGCR_Remote_Reporting_Status_t1}${FORM_AGCR_Remote_Reporting_Status_t2}${FORM_AGCR_Remote_Reporting_Status_t3}"

 	FORM_AGCR_Trigger_condition_0="$FORM_AGCR_Trigger_condition_0_gpsr"
 	FORM_AGCR_Trigger_condition_1="$FORM_AGCR_Trigger_condition_1_gpsr"
 	FORM_AGCR_Trigger_condition_2="$FORM_AGCR_Trigger_condition_2_gpsr"
 	FORM_AGCR_Trigger_condition_3="$FORM_AGCR_Trigger_condition_3_gpsr"
 	FORM_AGCR_Trigger_condition=$FORM_AGCR_Trigger_condition_0$FORM_AGCR_Trigger_condition_1$FORM_AGCR_Trigger_condition_2$FORM_AGCR_Trigger_condition_3

# 	FORM_AGCR_Distance_trigger_0="$FORM_AGCR_Distance_trigger_0_gpsr"
# 	FORM_AGCR_Distance_trigger_1="$FORM_AGCR_Distance_trigger_1_gpsr"
# 	FORM_AGCR_Distance_trigger_2="$FORM_AGCR_Distance_trigger_2_gpsr"
# 	FORM_AGCR_Distance_trigger_3="$FORM_AGCR_Distance_trigger_3_gpsr"
# 	FORM_AGCR_Distance_trigger=$FORM_AGCR_Distance_trigger_0$FORM_AGCR_Distance_trigger_1$FORM_AGCR_Distance_trigger_2$FORM_AGCR_Distance_trigger_3
        if [ "$FORM_AGCR_Remote_Reporting_Status_0" = "A" ]; then
            FORM_AGCR_Trigger_condition_t0="A"
        else
            if [ "$FORM_AGCR_Trigger_condition_0" = "C" ]; then
                        FORM_AGCR_Trigger_condition_t0="B"
            else
                        FORM_AGCR_Trigger_condition_t0="$FORM_AGCR_Trigger_condition_0"
            fi
        fi

        if [ "$FORM_AGCR_Remote_Reporting_Status_1" = "A" ]; then
            FORM_AGCR_Trigger_condition_t1="A"
        else
            if [ "$FORM_AGCR_Trigger_condition_1" = "C" ]; then
                        FORM_AGCR_Trigger_condition_t1="B"
            else
                        FORM_AGCR_Trigger_condition_t1="$FORM_AGCR_Trigger_condition_1"
            fi
        fi

        if [ "$FORM_AGCR_Remote_Reporting_Status_2" = "A" ]; then
            FORM_AGCR_Trigger_condition_t2="A"
        else
            if [ "$FORM_AGCR_Trigger_condition_2" = "C" ]; then
                        FORM_AGCR_Trigger_condition_t2="B"
            else
                        FORM_AGCR_Trigger_condition_t2="$FORM_AGCR_Trigger_condition_2"
            fi
        fi

        if [ "$FORM_AGCR_Remote_Reporting_Status_3" = "A" ]; then
            FORM_AGCR_Trigger_condition_t3="A"
        else
            if [ "$FORM_AGCR_Trigger_condition_3" = "C" ]; then
                        FORM_AGCR_Trigger_condition_t3="B"
            else
                        FORM_AGCR_Trigger_condition_t3="$FORM_AGCR_Trigger_condition_3"
            fi
        fi

 	FORM_AGCR_Distance_trigger="${FORM_AGCR_Trigger_condition_t0}${FORM_AGCR_Trigger_condition_t1}${FORM_AGCR_Trigger_condition_t2}${FORM_AGCR_Trigger_condition_t3}"

        if [ "$FORM_AGCR_Local_Stream_Set0_gpsr" = "B" ]; then
                    FORM_AGCR_Local_Stream_Set0="B"
        else
                    FORM_AGCR_Local_Stream_Set0="A"
        fi
        if [ "$FORM_AGCR_Local_Stream_Set1_gpsr" = "B" ]; then
                    FORM_AGCR_Local_Stream_Set1="B"
        else
                    FORM_AGCR_Local_Stream_Set1="A"
        fi
        if [ "$FORM_AGCR_Local_Stream_Set2_gpsr" = "B" ]; then
                    FORM_AGCR_Local_Stream_Set2="B"
        else
                    FORM_AGCR_Local_Stream_Set2="A"
        fi
        if [ "$FORM_AGCR_Local_Stream_Set3_gpsr" = "B" ]; then
                    FORM_AGCR_Local_Stream_Set3="B"
        else
                    FORM_AGCR_Local_Stream_Set3="A"
        fi

 	FORM_AGCR_Message_type0_0="$FORM_AGCR_Message_type0_0_gpsr"
 	FORM_AGCR_Message_type0_1="$FORM_AGCR_Message_type0_1_gpsr"
 	FORM_AGCR_Message_type0_2="$FORM_AGCR_Message_type0_2_gpsr"
 	FORM_AGCR_Message_type0_3="$FORM_AGCR_Message_type0_3_gpsr"
 	FORM_AGCR_Message_type0=$FORM_AGCR_Message_type0_0$FORM_AGCR_Message_type0_1$FORM_AGCR_Message_type0_2$FORM_AGCR_Message_type0_3

 	FORM_AGCR_Message_type1_0="$FORM_AGCR_Message_type1_0_gpsr"
 	FORM_AGCR_Message_type1_1="$FORM_AGCR_Message_type1_1_gpsr"
 	FORM_AGCR_Message_type1_2="$FORM_AGCR_Message_type1_2_gpsr"
 	FORM_AGCR_Message_type1_3="$FORM_AGCR_Message_type1_3_gpsr"
 	FORM_AGCR_Message_type1=$FORM_AGCR_Message_type1_0$FORM_AGCR_Message_type1_1$FORM_AGCR_Message_type1_2$FORM_AGCR_Message_type1_3

 	FORM_AGCR_Message_type2_0="$FORM_AGCR_Message_type2_0_gpsr"
 	FORM_AGCR_Message_type2_1="$FORM_AGCR_Message_type2_1_gpsr"
 	FORM_AGCR_Message_type2_2="$FORM_AGCR_Message_type2_2_gpsr"
 	FORM_AGCR_Message_type2_3="$FORM_AGCR_Message_type2_3_gpsr"
 	FORM_AGCR_Message_type2=$FORM_AGCR_Message_type2_0$FORM_AGCR_Message_type2_1$FORM_AGCR_Message_type2_2$FORM_AGCR_Message_type2_3

 	FORM_AGCR_Message_type3_0="$FORM_AGCR_Message_type3_0_gpsr"
 	FORM_AGCR_Message_type3_1="$FORM_AGCR_Message_type3_1_gpsr"
 	FORM_AGCR_Message_type3_2="$FORM_AGCR_Message_type3_2_gpsr"
 	FORM_AGCR_Message_type3_3="$FORM_AGCR_Message_type3_3_gpsr"
 	FORM_AGCR_Message_type3=$FORM_AGCR_Message_type3_0$FORM_AGCR_Message_type3_1$FORM_AGCR_Message_type3_2$FORM_AGCR_Message_type3_3

validate <<EOF
ip|FORM_AGCR_Remote_IP_address0_gpsr|@TR<<Remote IP0>>||$FORM_AGCR_Remote_IP_address0
ip|FORM_AGCR_Remote_IP_address1_gpsr|@TR<<Remote IP1>>||$FORM_AGCR_Remote_IP_address1
ip|FORM_AGCR_Remote_IP_address2_gpsr|@TR<<Remote IP2>>||$FORM_AGCR_Remote_IP_address2
ip|FORM_AGCR_Remote_IP_address3_gpsr|@TR<<Remote IP3>>||$FORM_AGCR_Remote_IP_address3
int|FORM_AGCR_Remote_PORT0_gpsr|@TR<<Remote Port0>>||$FORM_AGCR_Remote_PORT0
int|FORM_AGCR_Remote_PORT1_gpsr|@TR<<Remote Port1>>||$FORM_AGCR_Remote_PORT1
int|FORM_AGCR_Remote_PORT2_gpsr|@TR<<Remote Port2>>||$FORM_AGCR_Remote_PORT2
int|FORM_AGCR_Remote_PORT3_gpsr|@TR<<Remote Port3>>||$FORM_AGCR_Remote_PORT3
int|FORM_AGCR_Timer0_gpsr|@TR<<Intervals Timer0>>||$FORM_AGCR_Timer0
int|FORM_AGCR_Timer1_gpsr|@TR<<Intervals Timer1>>||$FORM_AGCR_Timer1
int|FORM_AGCR_Timer2_gpsr|@TR<<Intervals Timer2>>||$FORM_AGCR_Timer2
int|FORM_AGCR_Timer3_gpsr|@TR<<Intervals Timer3>>||$FORM_AGCR_Timer3
int|FORM_AGCR_Distance0_gpsr|@TR<<Trigger Set0>>||$FORM_AGCR_Distance0
int|FORM_AGCR_Distance1_gpsr|@TR<<Trigger Set1>>||$FORM_AGCR_Distance1
int|FORM_AGCR_Distance2_gpsr|@TR<<Trigger Set2>>||$FORM_AGCR_Distance2
int|FORM_AGCR_Distance3_gpsr|@TR<<Trigger Set3>>||$FORM_AGCR_Distance3
EOF


       [ "$?" = "0" ] && {

            uci_set "gpsr" "$conf" "AGCR_Remote_IP_address0" "$FORM_AGCR_Remote_IP_address0"
            uci_set "gpsr" "$conf" "AGCR_Remote_IP_address1" "$FORM_AGCR_Remote_IP_address1"
            uci_set "gpsr" "$conf" "AGCR_Remote_IP_address2" "$FORM_AGCR_Remote_IP_address2"
            uci_set "gpsr" "$conf" "AGCR_Remote_IP_address3" "$FORM_AGCR_Remote_IP_address3"
            uci_set "gpsr" "$conf" "AGCR_Remote_PORT0" "$FORM_AGCR_Remote_PORT0"
            uci_set "gpsr" "$conf" "AGCR_Remote_PORT1" "$FORM_AGCR_Remote_PORT1"
            uci_set "gpsr" "$conf" "AGCR_Remote_PORT2" "$FORM_AGCR_Remote_PORT2"
            uci_set "gpsr" "$conf" "AGCR_Remote_PORT3" "$FORM_AGCR_Remote_PORT3"
            uci_set "gpsr" "$conf" "AGCR_Timer0" "$FORM_AGCR_Timer0"
            uci_set "gpsr" "$conf" "AGCR_Timer1" "$FORM_AGCR_Timer1"
            uci_set "gpsr" "$conf" "AGCR_Timer2" "$FORM_AGCR_Timer2"
            uci_set "gpsr" "$conf" "AGCR_Timer3" "$FORM_AGCR_Timer3"
            uci_set "gpsr" "$conf" "AGCR_Distance0" "$FORM_AGCR_Distance0"
            uci_set "gpsr" "$conf" "AGCR_Distance1" "$FORM_AGCR_Distance1"
            uci_set "gpsr" "$conf" "AGCR_Distance2" "$FORM_AGCR_Distance2"
            uci_set "gpsr" "$conf" "AGCR_Distance3" "$FORM_AGCR_Distance3"
            uci_set "gpsr" "$conf" "SMTP0_Mail_Subject" "$FORM_SMTP0_Mail_Subject"
            uci_set "gpsr" "$conf" "SMTP0_Server" "$FORM_SMTP0_Server"
            uci_set "gpsr" "$conf" "SMTP0_User_Name" "$FORM_SMTP0_User_Name"
            uci_set "gpsr" "$conf" "SMTP0_Password" "$FORM_SMTP0_Password"
            uci_set "gpsr" "$conf" "SMTP0_Recipient" "$FORM_SMTP0_Recipient"
            uci_set "gpsr" "$conf" "SMTP1_Mail_Subject" "$FORM_SMTP1_Mail_Subject"
            uci_set "gpsr" "$conf" "SMTP1_Server" "$FORM_SMTP1_Server"
            uci_set "gpsr" "$conf" "SMTP1_User_Name" "$FORM_SMTP1_User_Name"
            uci_set "gpsr" "$conf" "SMTP1_Password" "$FORM_SMTP1_Password"
            uci_set "gpsr" "$conf" "SMTP1_Recipient" "$FORM_SMTP1_Recipient"
            uci_set "gpsr" "$conf" "SMTP2_Mail_Subject" "$FORM_SMTP2_Mail_Subject"
            uci_set "gpsr" "$conf" "SMTP2_Server" "$FORM_SMTP2_Server"
            uci_set "gpsr" "$conf" "SMTP2_User_Name" "$FORM_SMTP2_User_Name"
            uci_set "gpsr" "$conf" "SMTP2_Password" "$FORM_SMTP2_Password"
            uci_set "gpsr" "$conf" "SMTP2_Recipient" "$FORM_SMTP2_Recipient"
            uci_set "gpsr" "$conf" "SMTP3_Mail_Subject" "$FORM_SMTP3_Mail_Subject"
            uci_set "gpsr" "$conf" "SMTP3_Server" "$FORM_SMTP3_Server"
            uci_set "gpsr" "$conf" "SMTP3_User_Name" "$FORM_SMTP3_User_Name"
            uci_set "gpsr" "$conf" "SMTP3_Password" "$FORM_SMTP3_Password"
            uci_set "gpsr" "$conf" "SMTP3_Recipient" "$FORM_SMTP3_Recipient"

            uci_set "gpsr" "$conf" "AGCR_Remote_Reporting_Status" "$FORM_AGCR_Remote_Reporting_Status"
            uci_set "gpsr" "$conf" "AGCR_Timer_trigger" "$FORM_AGCR_Timer_trigger"
            uci_set "gpsr" "$conf" "AGCR_Distance_trigger" "$FORM_AGCR_Distance_trigger"
            uci_set "gpsr" "$conf" "AGCR_Trigger_condition" "$FORM_AGCR_Trigger_condition"
            uci_set "gpsr" "$conf" "AGCR_Message_type0" "$FORM_AGCR_Message_type0"
            uci_set "gpsr" "$conf" "AGCR_Message_type1" "$FORM_AGCR_Message_type1"
            uci_set "gpsr" "$conf" "AGCR_Message_type2" "$FORM_AGCR_Message_type2"
            uci_set "gpsr" "$conf" "AGCR_Message_type3" "$FORM_AGCR_Message_type3"

            uci_set "gpsr" "$conf" "AGCR_Local_Stream_Set0" "$FORM_AGCR_Local_Stream_Set0"
            uci_set "gpsr" "$conf" "AGCR_Local_Stream_Set1" "$FORM_AGCR_Local_Stream_Set1"
            uci_set "gpsr" "$conf" "AGCR_Local_Stream_Set2" "$FORM_AGCR_Local_Stream_Set2"
            uci_set "gpsr" "$conf" "AGCR_Local_Stream_Set3" "$FORM_AGCR_Local_Stream_Set3"

         }
fi



uci_load "gpsr"
       config_get FORM_AGCR_Remote_IP_address0 $conf AGCR_Remote_IP_address0
       config_get FORM_AGCR_Remote_IP_address1 $conf AGCR_Remote_IP_address1 
       config_get FORM_AGCR_Remote_IP_address2 $conf AGCR_Remote_IP_address2
       config_get FORM_AGCR_Remote_IP_address3 $conf AGCR_Remote_IP_address3
       config_get FORM_AGCR_Remote_PORT0 $conf AGCR_Remote_PORT0
       config_get FORM_AGCR_Remote_PORT1 $conf AGCR_Remote_PORT1
       config_get FORM_AGCR_Remote_PORT2 $conf AGCR_Remote_PORT2
       config_get FORM_AGCR_Remote_PORT3 $conf AGCR_Remote_PORT3
       config_get FORM_AGCR_Timer0 $conf AGCR_Timer0
       config_get FORM_AGCR_Timer1 $conf AGCR_Timer1
       config_get FORM_AGCR_Timer2 $conf AGCR_Timer2
       config_get FORM_AGCR_Timer3 $conf AGCR_Timer3
       config_get FORM_AGCR_Distance0 $conf AGCR_Distance0 
       config_get FORM_AGCR_Distance1 $conf AGCR_Distance1
       config_get FORM_AGCR_Distance2 $conf AGCR_Distance2
       config_get FORM_AGCR_Distance3 $conf AGCR_Distance3
       config_get FORM_SMTP0_Mail_Subject $conf SMTP0_Mail_Subject
       config_get FORM_SMTP0_Server $conf SMTP0_Server
       config_get FORM_SMTP0_User_Name $conf SMTP0_User_Name
       config_get FORM_SMTP0_Password $conf SMTP0_Password
       config_get FORM_SMTP0_Recipient $conf SMTP0_Recipient
       config_get FORM_SMTP1_Mail_Subject $conf SMTP1_Mail_Subject
       config_get FORM_SMTP1_Server $conf SMTP1_Server
       config_get FORM_SMTP1_User_Name $conf SMTP1_User_Name
       config_get FORM_SMTP1_Password $conf SMTP1_Password
       config_get FORM_SMTP1_Recipient $conf SMTP1_Recipient
       config_get FORM_SMTP2_Mail_Subject $conf SMTP2_Mail_Subject
       config_get FORM_SMTP2_Server $conf SMTP2_Server
       config_get FORM_SMTP2_User_Name $conf SMTP2_User_Name
       config_get FORM_SMTP2_Password $conf SMTP2_Password
       config_get FORM_SMTP2_Recipient $conf SMTP2_Recipient
       config_get FORM_SMTP3_Mail_Subject $conf SMTP3_Mail_Subject
       config_get FORM_SMTP3_Server $conf SMTP3_Server
       config_get FORM_SMTP3_User_Name $conf SMTP3_User_Name
       config_get FORM_SMTP3_Password $conf SMTP3_Password
       config_get FORM_SMTP3_Recipient $conf SMTP3_Recipient

       config_get FORM_AGCR_Local_Stream_Set0 $conf AGCR_Local_Stream_Set0
       config_get FORM_AGCR_Local_Stream_Set1 $conf AGCR_Local_Stream_Set1
       config_get FORM_AGCR_Local_Stream_Set2 $conf AGCR_Local_Stream_Set2
       config_get FORM_AGCR_Local_Stream_Set3 $conf AGCR_Local_Stream_Set3

       config_get FORM_AGCR_Remote_Reporting_Status $conf AGCR_Remote_Reporting_Status
#       config_get FORM_AGCR_Timer_trigger $conf AGCR_Timer_trigger
#       config_get FORM_AGCR_Distance_trigger $conf AGCR_Distance_trigger
       config_get FORM_AGCR_Trigger_condition $conf AGCR_Trigger_condition
       config_get FORM_AGCR_Message_type0 $conf AGCR_Message_type0
       config_get FORM_AGCR_Message_type1 $conf AGCR_Message_type1
       config_get FORM_AGCR_Message_type2 $conf AGCR_Message_type2
       config_get FORM_AGCR_Message_type3 $conf AGCR_Message_type3
#       uci_commit "gpsr"

FORM_AGCR_Remote_Reporting_Status_0=$(echo $FORM_AGCR_Remote_Reporting_Status | cut -c1)
FORM_AGCR_Remote_Reporting_Status_1=$(echo $FORM_AGCR_Remote_Reporting_Status | cut -c2)
FORM_AGCR_Remote_Reporting_Status_2=$(echo $FORM_AGCR_Remote_Reporting_Status | cut -c3)
FORM_AGCR_Remote_Reporting_Status_3=$(echo $FORM_AGCR_Remote_Reporting_Status | cut -c4)
#FORM_AGCR_Timer_trigger_0=$(echo $FORM_AGCR_Timer_trigger | cut -c1)
#FORM_AGCR_Timer_trigger_1=$(echo $FORM_AGCR_Timer_trigger | cut -c2)
#FORM_AGCR_Timer_trigger_2=$(echo $FORM_AGCR_Timer_trigger | cut -c3)
#FORM_AGCR_Timer_trigger_3=$(echo $FORM_AGCR_Timer_trigger | cut -c4)
#FORM_AGCR_Distance_trigger_0=$(echo $FORM_AGCR_Distance_trigger | cut -c1)
#FORM_AGCR_Distance_trigger_1=$(echo $FORM_AGCR_Distance_trigger | cut -c2)
#FORM_AGCR_Distance_trigger_2=$(echo $FORM_AGCR_Distance_trigger | cut -c3)
#FORM_AGCR_Distance_trigger_3=$(echo $FORM_AGCR_Distance_trigger | cut -c4)
FORM_AGCR_Trigger_condition_0=$(echo $FORM_AGCR_Trigger_condition | cut -c1)
FORM_AGCR_Trigger_condition_1=$(echo $FORM_AGCR_Trigger_condition | cut -c2)
FORM_AGCR_Trigger_condition_2=$(echo $FORM_AGCR_Trigger_condition | cut -c3)
FORM_AGCR_Trigger_condition_3=$(echo $FORM_AGCR_Trigger_condition | cut -c4)
FORM_AGCR_Message_type0_0=$(echo $FORM_AGCR_Message_type0 | cut -c1)
FORM_AGCR_Message_type0_1=$(echo $FORM_AGCR_Message_type0 | cut -c2)
FORM_AGCR_Message_type0_2=$(echo $FORM_AGCR_Message_type0 | cut -c3)
FORM_AGCR_Message_type0_3=$(echo $FORM_AGCR_Message_type0 | cut -c4)
FORM_AGCR_Message_type1_0=$(echo $FORM_AGCR_Message_type1 | cut -c1)
FORM_AGCR_Message_type1_1=$(echo $FORM_AGCR_Message_type1 | cut -c2)
FORM_AGCR_Message_type1_2=$(echo $FORM_AGCR_Message_type1 | cut -c3)
FORM_AGCR_Message_type1_3=$(echo $FORM_AGCR_Message_type1 | cut -c4)
FORM_AGCR_Message_type2_0=$(echo $FORM_AGCR_Message_type2 | cut -c1)
FORM_AGCR_Message_type2_1=$(echo $FORM_AGCR_Message_type2 | cut -c2)
FORM_AGCR_Message_type2_2=$(echo $FORM_AGCR_Message_type2 | cut -c3)
FORM_AGCR_Message_type2_3=$(echo $FORM_AGCR_Message_type2 | cut -c4)
FORM_AGCR_Message_type3_0=$(echo $FORM_AGCR_Message_type3 | cut -c1)
FORM_AGCR_Message_type3_1=$(echo $FORM_AGCR_Message_type3 | cut -c2)
FORM_AGCR_Message_type3_2=$(echo $FORM_AGCR_Message_type3 | cut -c3)
FORM_AGCR_Message_type3_3=$(echo $FORM_AGCR_Message_type3 | cut -c4)


#field|@TR<<&nbsp;&nbsp;AAA>>|_gpsr_field|hidden
#text|_gpsr|$FORM_|@TR<<>>
#field|@TR<<&nbsp;&nbsp;AAA>>|_gpsr_field|hidden
#select|_gpsr|$FORM_
#option|A|@TR<<Disable>>
#option|D|@TR<<GpsGate Protocol UDP Report>>

config_options="
start_form| @TR<<GPS Report No.1>>
field|@TR<< <strong>Report Define<strong> >>|AGCR_Remote_Reporting_Status_0_gpsr_field|1
select|AGCR_Remote_Reporting_Status_0_gpsr|$FORM_AGCR_Remote_Reporting_Status_0
option|A|@TR<<Disable>>
option|B|@TR<<UDP Report>>
option|C|@TR<<Email Report>>

field|@TR<<&nbsp;&nbsp;Time Interval>>|AGCR_Timer0_gpsr_field|hidden
text|AGCR_Timer0_gpsr|$FORM_AGCR_Timer0|@TR<<(s)>>

field|@TR<<&nbsp;&nbsp;Message 1>>|AGCR_Message_type0_0_gpsr_field|hidden
select|AGCR_Message_type0_0_gpsr|$FORM_AGCR_Message_type0_0
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Message 2>>|AGCR_Message_type0_1_gpsr_field|hidden
select|AGCR_Message_type0_1_gpsr|$FORM_AGCR_Message_type0_1
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Message 3>>|AGCR_Message_type0_2_gpsr_field|hidden
select|AGCR_Message_type0_2_gpsr|$FORM_AGCR_Message_type0_2
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Message 4>>|AGCR_Message_type0_3_gpsr_field|hidden
select|AGCR_Message_type0_3_gpsr|$FORM_AGCR_Message_type0_3
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Trigger Set>>|AGCR_Trigger_condition_0_gpsr_field|hidden
select|AGCR_Trigger_condition_0_gpsr|$FORM_AGCR_Trigger_condition_0
option|A|@TR<< Only Timer >>
option|B|@TR<< Timer AND Distance >>
option|C|@TR<< Timer OR Distance >>

field|@TR<<&nbsp;&nbsp;Distance Set>>|AGCR_Distance0_gpsr_field|hidden
text|AGCR_Distance0_gpsr|$FORM_AGCR_Distance0|@TR<<(meters)>>

field|@TR<<&nbsp;&nbsp;Local Streaming>>|AGCR_Local_Stream_Set0_gpsr_field|hidden
select|AGCR_Local_Stream_Set0_gpsr|$FORM_AGCR_Local_Stream_Set0
option|A|@TR<< Disable >>
option|B|@TR<< Enable For Lan Attached IP >>

field|@TR<<&nbsp;&nbsp;UDP Remote IP>>|AGCR_Remote_IP_address0_gpsr_field|hidden
text|AGCR_Remote_IP_address0_gpsr|$FORM_AGCR_Remote_IP_address0|@TR<<(x.x.x.x)>>

field|@TR<<&nbsp;&nbsp;UDP Remote PORT>>|AGCR_Remote_PORT0_gpsr_field|hidden
text|AGCR_Remote_PORT0_gpsr|$FORM_AGCR_Remote_PORT0|@TR<<[0~65535]>>

field|@TR<<&nbsp;&nbsp;Mail Subject>>|SMTP0_Mail_Subject_gpsr_field|hidden
text|SMTP0_Mail_Subject_gpsr|$FORM_SMTP0_Mail_Subject|@TR<<>>

field|@TR<<&nbsp;&nbsp;Mail Server(IP/Name)>>|SMTP0_Server_gpsr_field|hidden
text|SMTP0_Server_gpsr|$FORM_SMTP0_Server|@TR<<(xxx:port)>>

field|@TR<<&nbsp;&nbsp;User Name>>|SMTP0_User_Name_gpsr_field|hidden
text|SMTP0_User_Name_gpsr|$FORM_SMTP0_User_Name|@TR<<>>

field|@TR<<&nbsp;&nbsp;Password>>|SMTP0_Password_gpsr_field|hidden
password|SMTP0_Password_gpsr|$FORM_SMTP0_Password|@TR<<>>

field|@TR<<&nbsp;&nbsp;Mail Recipient>>|SMTP0_Recipient_gpsr_field|hidden
text|SMTP0_Recipient_gpsr|$FORM_SMTP0_Recipient|@TR<<(xx@xx.xx)>>
end_form


start_form| @TR<<GPS Report No.2>>
field|@TR<< <strong>Report Define<strong> >>|AGCR_Remote_Reporting_Status_1_gpsr_field|1
select|AGCR_Remote_Reporting_Status_1_gpsr|$FORM_AGCR_Remote_Reporting_Status_1
option|A|@TR<<Disable>>
option|B|@TR<<UDP Report>>
option|C|@TR<<Email Report>>

field|@TR<<&nbsp;&nbsp;Time Interval>>|AGCR_Timer1_gpsr_field|hidden
text|AGCR_Timer1_gpsr|$FORM_AGCR_Timer1|@TR<<(s)>>

field|@TR<<&nbsp;&nbsp;Message 1>>|AGCR_Message_type1_0_gpsr_field|hidden
select|AGCR_Message_type1_0_gpsr|$FORM_AGCR_Message_type1_0
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Message 2>>|AGCR_Message_type1_1_gpsr_field|hidden
select|AGCR_Message_type1_1_gpsr|$FORM_AGCR_Message_type1_1
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Message 3>>|AGCR_Message_type1_2_gpsr_field|hidden
select|AGCR_Message_type1_2_gpsr|$FORM_AGCR_Message_type1_2
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Message 4>>|AGCR_Message_type1_3_gpsr_field|hidden
select|AGCR_Message_type1_3_gpsr|$FORM_AGCR_Message_type1_3
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Trigger Set>>|AGCR_Trigger_condition_1_gpsr_field|hidden
select|AGCR_Trigger_condition_1_gpsr|$FORM_AGCR_Trigger_condition_1
option|A|@TR<< Only Timer >>
option|B|@TR<< Timer AND Distance >>
option|C|@TR<< Timer OR Distance >>

field|@TR<<&nbsp;&nbsp;Distance Set>>|AGCR_Distance1_gpsr_field|hidden
text|AGCR_Distance1_gpsr|$FORM_AGCR_Distance1|@TR<<(meters)>>

field|@TR<<&nbsp;&nbsp;Local Streaming>>|AGCR_Local_Stream_Set1_gpsr_field|hidden
select|AGCR_Local_Stream_Set1_gpsr|$FORM_AGCR_Local_Stream_Set1
option|A|@TR<< Disable >>
option|B|@TR<< Enable For Lan Attached IP >>

field|@TR<<&nbsp;&nbsp;UDP Remote IP>>|AGCR_Remote_IP_address1_gpsr_field|hidden
text|AGCR_Remote_IP_address1_gpsr|$FORM_AGCR_Remote_IP_address1|@TR<<(x.x.x.x)>>

field|@TR<<&nbsp;&nbsp;UDP Remote PORT>>|AGCR_Remote_PORT1_gpsr_field|hidden
text|AGCR_Remote_PORT1_gpsr|$FORM_AGCR_Remote_PORT1|@TR<<[0~65535]>>

field|@TR<<&nbsp;&nbsp;Mail Subject>>|SMTP1_Mail_Subject_gpsr_field|hidden
text|SMTP1_Mail_Subject_gpsr|$FORM_SMTP1_Mail_Subject|@TR<<>>

field|@TR<<&nbsp;&nbsp;Mail Server(IP/Name)>>|SMTP1_Server_gpsr_field|hidden
text|SMTP1_Server_gpsr|$FORM_SMTP1_Server|@TR<<(xxx:port)>>

field|@TR<<&nbsp;&nbsp;User Name>>|SMTP1_User_Name_gpsr_field|hidden
text|SMTP1_User_Name_gpsr|$FORM_SMTP1_User_Name|@TR<<>>

field|@TR<<&nbsp;&nbsp;Password>>|SMTP1_Password_gpsr_field|hidden
password|SMTP1_Password_gpsr|$FORM_SMTP1_Password|@TR<<>>

field|@TR<<&nbsp;&nbsp;Mail Recipient>>|SMTP1_Recipient_gpsr_field|hidden
text|SMTP1_Recipient_gpsr|$FORM_SMTP1_Recipient|@TR<<(xx@xx.xx)>>
end_form


start_form| @TR<<GPS Report No.3>>
field|@TR<< <strong>Report Define<strong> >>|AGCR_Remote_Reporting_Status_2_gpsr_field|1
select|AGCR_Remote_Reporting_Status_2_gpsr|$FORM_AGCR_Remote_Reporting_Status_2
option|A|@TR<<Disable>>
option|B|@TR<<UDP Report>>
option|C|@TR<<Email Report>>

field|@TR<<&nbsp;&nbsp;Time Interval>>|AGCR_Timer2_gpsr_field|hidden
text|AGCR_Timer2_gpsr|$FORM_AGCR_Timer2|@TR<<(s)>>

field|@TR<<&nbsp;&nbsp;Message 1>>|AGCR_Message_type2_0_gpsr_field|hidden
select|AGCR_Message_type2_0_gpsr|$FORM_AGCR_Message_type2_0
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Message 2>>|AGCR_Message_type2_1_gpsr_field|hidden
select|AGCR_Message_type2_1_gpsr|$FORM_AGCR_Message_type2_1
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Message 3>>|AGCR_Message_type2_2_gpsr_field|hidden
select|AGCR_Message_type2_2_gpsr|$FORM_AGCR_Message_type2_2
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Message 4>>|AGCR_Message_type2_3_gpsr_field|hidden
select|AGCR_Message_type2_3_gpsr|$FORM_AGCR_Message_type2_3
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Trigger Set>>|AGCR_Trigger_condition_2_gpsr_field|hidden
select|AGCR_Trigger_condition_2_gpsr|$FORM_AGCR_Trigger_condition_2
option|A|@TR<< Only Timer >>
option|B|@TR<< Timer AND Distance >>
option|C|@TR<< Timer OR Distance >>

field|@TR<<&nbsp;&nbsp;Distance Set>>|AGCR_Distance2_gpsr_field|hidden
text|AGCR_Distance2_gpsr|$FORM_AGCR_Distance2|@TR<<(meters)>>

field|@TR<<&nbsp;&nbsp;Local Streaming>>|AGCR_Local_Stream_Set2_gpsr_field|hidden
select|AGCR_Local_Stream_Set2_gpsr|$FORM_AGCR_Local_Stream_Set2
option|A|@TR<< Disable >>
option|B|@TR<< Enable For Lan Attached IP >>

field|@TR<<&nbsp;&nbsp;UDP Remote IP>>|AGCR_Remote_IP_address2_gpsr_field|hidden
text|AGCR_Remote_IP_address2_gpsr|$FORM_AGCR_Remote_IP_address2|@TR<<(x.x.x.x)>>

field|@TR<<&nbsp;&nbsp;UDP Remote PORT>>|AGCR_Remote_PORT2_gpsr_field|hidden
text|AGCR_Remote_PORT2_gpsr|$FORM_AGCR_Remote_PORT2|@TR<<[0~65535]>>

field|@TR<<&nbsp;&nbsp;Mail Subject>>|SMTP2_Mail_Subject_gpsr_field|hidden
text|SMTP2_Mail_Subject_gpsr|$FORM_SMTP2_Mail_Subject|@TR<<>>

field|@TR<<&nbsp;&nbsp;Mail Server(IP/Name)>>|SMTP2_Server_gpsr_field|hidden
text|SMTP2_Server_gpsr|$FORM_SMTP2_Server|@TR<<(xxx:port)>>

field|@TR<<&nbsp;&nbsp;User Name>>|SMTP2_User_Name_gpsr_field|hidden
text|SMTP2_User_Name_gpsr|$FORM_SMTP2_User_Name|@TR<<>>

field|@TR<<&nbsp;&nbsp;Password>>|SMTP2_Password_gpsr_field|hidden
password|SMTP2_Password_gpsr|$FORM_SMTP2_Password|@TR<<>>

field|@TR<<&nbsp;&nbsp;Mail Recipient>>|SMTP2_Recipient_gpsr_field|hidden
text|SMTP2_Recipient_gpsr|$FORM_SMTP2_Recipient|@TR<<(xx@xx.xx)>>
end_form


start_form| @TR<<GPS Report No.4 $gps_status>>
field|@TR<< <strong>Report Define<strong> >>|AGCR_Remote_Reporting_Status_3_gpsr_field|1
select|AGCR_Remote_Reporting_Status_3_gpsr|$FORM_AGCR_Remote_Reporting_Status_3
option|A|@TR<<Disable>>
option|B|@TR<<UDP Report>>
option|C|@TR<<Email Report>>

field|@TR<<&nbsp;&nbsp;Time Interval>>|AGCR_Timer3_gpsr_field|hidden
text|AGCR_Timer3_gpsr|$FORM_AGCR_Timer3|@TR<<(s)>>

field|@TR<<&nbsp;&nbsp;Message 1>>|AGCR_Message_type3_0_gpsr_field|hidden
select|AGCR_Message_type3_0_gpsr|$FORM_AGCR_Message_type3_0
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Message 2>>|AGCR_Message_type3_1_gpsr_field|hidden
select|AGCR_Message_type3_1_gpsr|$FORM_AGCR_Message_type3_1
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Message 3>>|AGCR_Message_type3_2_gpsr_field|hidden
select|AGCR_Message_type3_2_gpsr|$FORM_AGCR_Message_type3_2
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Message 4>>|AGCR_Message_type3_3_gpsr_field|hidden
select|AGCR_Message_type3_3_gpsr|$FORM_AGCR_Message_type3_3
option|A|@TR<<None>>
option|B|@TR<<ALL NMEA>>
option|C|@TR<<GGA>>
option|D|@TR<<GSA>>
option|E|@TR<<GSV>>
option|F|@TR<<RMC>>
option|G|@TR<<VTG>>
option|J|@TR<<Latitude/Longitude>>
option|K|@TR<<GpsGate UDP Protocol>>
field|@TR<<&nbsp;&nbsp;Trigger Set>>|AGCR_Trigger_condition_3_gpsr_field|hidden
select|AGCR_Trigger_condition_3_gpsr|$FORM_AGCR_Trigger_condition_3
option|A|@TR<< Only Timer >>
option|B|@TR<< Timer AND Distance >>
option|C|@TR<< Timer OR Distance >>

field|@TR<<&nbsp;&nbsp;Distance Set>>|AGCR_Distance3_gpsr_field|hidden
text|AGCR_Distance3_gpsr|$FORM_AGCR_Distance3|@TR<<(meters)>>

field|@TR<<&nbsp;&nbsp;Local Streaming>>|AGCR_Local_Stream_Set3_gpsr_field|hidden
select|AGCR_Local_Stream_Set3_gpsr|$FORM_AGCR_Local_Stream_Set3
option|A|@TR<< Disable >>
option|B|@TR<< Enable For Lan Attached IP >>

field|@TR<<&nbsp;&nbsp;UDP Remote IP>>|AGCR_Remote_IP_address3_gpsr_field|hidden
text|AGCR_Remote_IP_address3_gpsr|$FORM_AGCR_Remote_IP_address3|@TR<<(x.x.x.x)>>

field|@TR<<&nbsp;&nbsp;UDP Remote PORT>>|AGCR_Remote_PORT3_gpsr_field|hidden
text|AGCR_Remote_PORT3_gpsr|$FORM_AGCR_Remote_PORT3|@TR<<[0~65535]>>

field|@TR<<&nbsp;&nbsp;Mail Subject>>|SMTP3_Mail_Subject_gpsr_field|hidden
text|SMTP3_Mail_Subject_gpsr|$FORM_SMTP3_Mail_Subject|@TR<<>>

field|@TR<<&nbsp;&nbsp;Mail Server(IP/Name)>>|SMTP3_Server_gpsr_field|hidden
text|SMTP3_Server_gpsr|$FORM_SMTP3_Server|@TR<<(xxx:port)>>

field|@TR<<&nbsp;&nbsp;User Name>>|SMTP3_User_Name_gpsr_field|hidden
text|SMTP3_User_Name_gpsr|$FORM_SMTP3_User_Name|@TR<<>>

field|@TR<<&nbsp;&nbsp;Password>>|SMTP3_Password_gpsr_field|hidden
password|SMTP3_Password_gpsr|$FORM_SMTP3_Password|@TR<<>>

field|@TR<<&nbsp;&nbsp;Mail Recipient>>|SMTP3_Recipient_gpsr_field|hidden
text|SMTP3_Recipient_gpsr|$FORM_SMTP3_Recipient|@TR<<(xx@xx.xx)>>
end_form


"

append forms "$config_options" "$N"


######################################################################################
# set JavaScript
javascript_forms="
"
for info_i in 0 1 2 3; do
javascript_forms=$javascript_forms"

    v_none = isset('AGCR_Remote_Reporting_Status_${info_i}_gpsr', 'A');
    v_udp = isset('AGCR_Remote_Reporting_Status_${info_i}_gpsr', 'B');
    v_email = isset('AGCR_Remote_Reporting_Status_${info_i}_gpsr', 'C');
    set_visible('AGCR_Timer${info_i}_gpsr_field', !v_none);
    set_visible('AGCR_Message_type${info_i}_0_gpsr_field', !v_none);
    set_visible('AGCR_Message_type${info_i}_1_gpsr_field', !v_none);
    set_visible('AGCR_Message_type${info_i}_2_gpsr_field', !v_none);
    set_visible('AGCR_Message_type${info_i}_3_gpsr_field', !v_none);
    set_visible('AGCR_Trigger_condition_${info_i}_gpsr_field', !v_none);

    v_dis_none = isset('AGCR_Trigger_condition_${info_i}_gpsr', 'A');
    v_dis=0;
    if((!v_dis_none)&&(!v_none))v_dis=1;
    
    set_visible('AGCR_Distance${info_i}_gpsr_field',v_dis);

    set_visible('AGCR_Local_Stream_Set${info_i}_gpsr_field',v_udp);

    v_local = isset('AGCR_Local_Stream_Set${info_i}_gpsr', 'B');
    if(v_udp && !v_local)set_visible('AGCR_Remote_IP_address${info_i}_gpsr_field', 1);
    else set_visible('AGCR_Remote_IP_address${info_i}_gpsr_field', 0);

    set_visible('AGCR_Remote_PORT${info_i}_gpsr_field', v_udp);

    set_visible('SMTP${info_i}_Mail_Subject_gpsr_field', v_email);
    set_visible('SMTP${info_i}_Server_gpsr_field', v_email);
    set_visible('SMTP${info_i}_User_Name_gpsr_field', v_email);
    set_visible('SMTP${info_i}_Password_gpsr_field', v_email);
    set_visible('SMTP${info_i}_Recipient_gpsr_field', v_email);

"
done

append js "$javascript_forms" "$N"


#####################################################################
header "GPS" "Report" "@TR<<GPS Report Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"
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

EOF



display_form <<EOF
onchange|modechange
$validate_error
$forms
EOF

#####################################################################
else  
#####################################################################
header "GPS" "Report" "@TR<<GPS Report Configuration>>"  "$SCRIPT_NAME"
#####################################################################
cat <<EOF
Sorry, GPS service is disable. No valid GPS report setting data.
<br>Please config GPS Settings firstly.
EOF

fi




footer ?>
<!--
##WEBIF:name:GPS:300:Report
-->
