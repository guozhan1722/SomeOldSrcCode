#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"

if ! empty "$FORM_submit"; then

	FORM_Netflow_Remote_IF0="$FORM_Netflow_Remote_IF0_add"
        FORM_Netflow_Remote_IF1="$FORM_Netflow_Remote_IF1_add"
        FORM_Netflow_Remote_IF2="$FORM_Netflow_Remote_IF2_add"
 	FORM_Netflow_Remote_IP0="$FORM_Netflow_Remote_IP0_add"
        FORM_Netflow_Remote_IP1="$FORM_Netflow_Remote_IP1_add"
        FORM_Netflow_Remote_IP2="$FORM_Netflow_Remote_IP2_add"
 	FORM_Netflow_Remote_PORT0="$FORM_Netflow_Remote_PORT0_add"
        FORM_Netflow_Remote_PORT1="$FORM_Netflow_Remote_PORT1_add"
        FORM_Netflow_Remote_PORT2="$FORM_Netflow_Remote_PORT2_add"
 	FORM_Netflow_Remote_Status0="$FORM_Netflow_Remote_Status0_add"
        FORM_Netflow_Remote_Status1="$FORM_Netflow_Remote_Status1_add"
        FORM_Netflow_Remote_Status2="$FORM_Netflow_Remote_Status2_add"
 	FORM_Netflow_Remote_Exp0="$FORM_Netflow_Remote_Exp0_add"
        FORM_Netflow_Remote_Exp1="$FORM_Netflow_Remote_Exp1_add"
        FORM_Netflow_Remote_Exp2="$FORM_Netflow_Remote_Exp2_add"
 	FORM_Netflow_Remote_SIP0="$FORM_Netflow_Remote_SIP0_add"
        FORM_Netflow_Remote_SIP1="$FORM_Netflow_Remote_SIP1_add"
        FORM_Netflow_Remote_SIP2="$FORM_Netflow_Remote_SIP2_add"
 	FORM_Netflow_Version0="$FORM_Netflow_Version0_add"
        FORM_Netflow_Version1="$FORM_Netflow_Version1_add"
        FORM_Netflow_Version2="$FORM_Netflow_Version2_add"

validate <<EOF
ip|FORM_Netflow_Remote_IP0_add|@TR<<Remote IP0>>||$FORM_Netflow_Remote_IP0
ip|FORM_Netflow_Remote_IP1_add|@TR<<Remote IP1>>||$FORM_Netflow_Remote_IP1
ip|FORM_Netflow_Remote_IP2_add|@TR<<Remote IP2>>||$FORM_Netflow_Remote_IP2
int|FORM_Netflow_Remote_PORT0_add|@TR<<Remote Port0>>||$FORM_Netflow_Remote_PORT0
int|FORM_Netflow_Remote_PORT1_add|@TR<<Remote Port1>>||$FORM_Netflow_Remote_PORT1
int|FORM_Netflow_Remote_PORT2_add|@TR<<Remote Port2>>||$FORM_Netflow_Remote_PORT2
ip|FORM_Netflow_Remote_SIP0_add|@TR<<Source Address0>>||$FORM_Netflow_Remote_SIP0
ip|FORM_Netflow_Remote_SIP1_add|@TR<<Source Address1>>||$FORM_Netflow_Remote_SIP1
ip|FORM_Netflow_Remote_SIP2_add|@TR<<Source Address2>>||$FORM_Netflow_Remote_SIP2
EOF


       [ "$?" = "0" ] && {
            form_valid="true"
            uci_set "ntrd" "report0" "status" "$FORM_Netflow_Remote_Status0"
            uci_set "ntrd" "report0" "interface" "$FORM_Netflow_Remote_IF0"
            uci_set "ntrd" "report0" "ip" "$FORM_Netflow_Remote_IP0"
            uci_set "ntrd" "report0" "port" "$FORM_Netflow_Remote_PORT0"
            uci_set "ntrd" "report0" "express" "$FORM_Netflow_Remote_Exp0"
            uci_set "ntrd" "report0" "sip" "$FORM_Netflow_Remote_SIP0"
            uci_set "ntrd" "report0" "version" "$FORM_Netflow_Version0"

            uci_set "ntrd" "report1" "status" "$FORM_Netflow_Remote_Status1"
            uci_set "ntrd" "report1" "interface" "$FORM_Netflow_Remote_IF1"
            uci_set "ntrd" "report1" "ip" "$FORM_Netflow_Remote_IP1"
            uci_set "ntrd" "report1" "port" "$FORM_Netflow_Remote_PORT1"
            uci_set "ntrd" "report1" "express" "$FORM_Netflow_Remote_Exp1"
            uci_set "ntrd" "report1" "sip" "$FORM_Netflow_Remote_SIP1"
            uci_set "ntrd" "report1" "version" "$FORM_Netflow_Version1"

            uci_set "ntrd" "report2" "status" "$FORM_Netflow_Remote_Status2"
            uci_set "ntrd" "report2" "interface" "$FORM_Netflow_Remote_IF2"
            uci_set "ntrd" "report2" "ip" "$FORM_Netflow_Remote_IP2"
            uci_set "ntrd" "report2" "port" "$FORM_Netflow_Remote_PORT2"
            uci_set "ntrd" "report2" "express" "$FORM_Netflow_Remote_Exp2"
            uci_set "ntrd" "report2" "sip" "$FORM_Netflow_Remote_SIP2"
            uci_set "ntrd" "report2" "version" "$FORM_Netflow_Version2"
         }
else


uci_load ntrd
       config_get FORM_Netflow_Remote_Status0 report0 status
       config_get FORM_Netflow_Remote_IF0 report0 interface
       config_get FORM_Netflow_Remote_IP0 report0 ip
       config_get FORM_Netflow_Remote_PORT0 report0 port
       config_get FORM_Netflow_Remote_Exp0 report0 express
       config_get FORM_Netflow_Remote_SIP0 report0 sip
       config_get FORM_Netflow_Version0 report0 version

       config_get FORM_Netflow_Remote_Status1 report1 status
       config_get FORM_Netflow_Remote_IF1 report1 interface
       config_get FORM_Netflow_Remote_IP1 report1 ip
       config_get FORM_Netflow_Remote_PORT1 report1 port
       config_get FORM_Netflow_Remote_Exp1 report1 express
       config_get FORM_Netflow_Remote_SIP1 report1 sip
       config_get FORM_Netflow_Version1 report1 version

       config_get FORM_Netflow_Remote_Status2 report2 status
       config_get FORM_Netflow_Remote_IF2 report2 interface
       config_get FORM_Netflow_Remote_IP2 report2 ip
       config_get FORM_Netflow_Remote_PORT2 report2 port
       config_get FORM_Netflow_Remote_Exp2 report2 express
       config_get FORM_Netflow_Remote_SIP2 report2 sip
       config_get FORM_Netflow_Version2 report2 version
fi

config_options="
start_form| @TR<<Report Configuration No.1>>
field|@TR<< <strong>Status</strong> >>
select|Netflow_Remote_Status0_add|$FORM_Netflow_Remote_Status0
option|A|@TR<<Disable>>
option|B|@TR<<Enable>>

field|@TR<<&nbsp;&nbsp;Source Address>>|Netflow_Remote_SIP0_field|hidden
text|Netflow_Remote_SIP0_add|$FORM_Netflow_Remote_SIP0|@TR<<default 0.0.0.0>>

field|@TR<<&nbsp;&nbsp;Interface>>|Netflow_Remote_IF0_field|hidden
select|Netflow_Remote_IF0_add|$FORM_Netflow_Remote_IF0
option|br-lan|@TR<<LAN>>
option|br-wan|@TR<<WAN>>
option|br-wan2|@TR<<4G>>
option|any|@TR<<ALL>>

field|@TR<<&nbsp;&nbsp;Remote IP>>|Netflow_Remote_IP0_field|hidden
text|Netflow_Remote_IP0_add|$FORM_Netflow_Remote_IP0

field|@TR<<&nbsp;&nbsp;Remote Port>>|Netflow_Remote_PORT0_field|hidden
text|Netflow_Remote_PORT0_add|$FORM_Netflow_Remote_PORT0|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Filter expression>>|Netflow_Remote_Exp0_field|hidden
text|Netflow_Remote_Exp0_add|$FORM_Netflow_Remote_Exp0

field|@TR<<&nbsp;&nbsp;Version>>|Netflow_Remote_Version0_field|hidden
select|Netflow_Version0_add|$FORM_Netflow_Version0
option|n1|@TR<<V1>>
option|n5|@TR<<V5>>
option|n7|@TR<<V7>>

end_form

start_form| @TR<<Report Configuration No.2>>
field|@TR<< <strong>Status</strong> >>
select|Netflow_Remote_Status1_add|$FORM_Netflow_Remote_Status1
option|A|@TR<<Disable>>
option|B|@TR<<Enable>>

field|@TR<<&nbsp;&nbsp;Source Address>>|Netflow_Remote_SIP1_field|hidden
text|Netflow_Remote_SIP1_add|$FORM_Netflow_Remote_SIP1|@TR<<default 0.0.0.0>>

field|@TR<<&nbsp;&nbsp;Interface>>|Netflow_Remote_IF1_field|hidden
select|Netflow_Remote_IF1_add|$FORM_Netflow_Remote_IF1
option|br-lan|@TR<<LAN>>
option|br-wan|@TR<<WAN>>
option|br-wan2|@TR<<4G>>
option|any|@TR<<ALL>>

field|@TR<<&nbsp;&nbsp;Remote IP>>|Netflow_Remote_IP1_field|hidden
text|Netflow_Remote_IP1_add|$FORM_Netflow_Remote_IP1

field|@TR<<&nbsp;&nbsp;Remote Port>>|Netflow_Remote_PORT1_field|hidden
text|Netflow_Remote_PORT1_add|$FORM_Netflow_Remote_PORT1|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Filter expression>>|Netflow_Remote_Exp1_field|hidden
text|Netflow_Remote_Exp1_add|$FORM_Netflow_Remote_Exp1

field|@TR<<&nbsp;&nbsp;Version>>|Netflow_Remote_Version1_field|hidden
select|Netflow_Version1_add|$FORM_Netflow_Version1
option|n1|@TR<<V1>>
option|n5|@TR<<V5>>
option|n7|@TR<<V7>>

end_form

start_form| @TR<<Report Configuration No.3>>
field|@TR<< <strong>Status</strong> >>
select|Netflow_Remote_Status2_add|$FORM_Netflow_Remote_Status2
option|A|@TR<<Disable>>
option|B|@TR<<Enable>>

field|@TR<<&nbsp;&nbsp;Source Address>>|Netflow_Remote_SIP2_field|hidden
text|Netflow_Remote_SIP2_add|$FORM_Netflow_Remote_SIP2|@TR<<default 0.0.0.0>>

field|@TR<<&nbsp;&nbsp;Interface>>|Netflow_Remote_IF2_field|hidden
select|Netflow_Remote_IF2_add|$FORM_Netflow_Remote_IF2
option|br-lan|@TR<<LAN>>
option|br-wan|@TR<<WAN>>
option|br-wan2|@TR<<4G>>
option|any|@TR<<ALL>>

field|@TR<<&nbsp;&nbsp;Remote IP>>|Netflow_Remote_IP2_field|hidden
text|Netflow_Remote_IP2_add|$FORM_Netflow_Remote_IP2

field|@TR<<&nbsp;&nbsp;Remote Port>>|Netflow_Remote_PORT2_field|hidden
text|Netflow_Remote_PORT2_add|$FORM_Netflow_Remote_PORT2|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Filter expression>>|Netflow_Remote_Exp2_field|hidden
text|Netflow_Remote_Exp2_add|$FORM_Netflow_Remote_Exp2

field|@TR<<&nbsp;&nbsp;Version>>|Netflow_Remote_Version2_field|hidden
select|Netflow_Version2_add|$FORM_Netflow_Version2
option|n1|@TR<<V1>>
option|n5|@TR<<V5>>
option|n7|@TR<<V7>>

end_form"

append forms "$config_options" "$N"


#####################################################################
header "Tools" "Netflow Report" "@TR<<Netflow Report>>"  'onload="modechange()"' "$SCRIPT_NAME"
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
        v = isset('Netflow_Remote_Status0_add', 'A');
        if (v) {
           hide('Netflow_Remote_IF0_field');
           hide('Netflow_Remote_IP0_field');
           hide('Netflow_Remote_PORT0_field');
           hide('Netflow_Remote_Exp0_field');
           hide('Netflow_Remote_SIP0_field');
           hide('Netflow_Remote_Version0_field');
        }
        else {
           show('Netflow_Remote_IP0_field');
           show('Netflow_Remote_IF0_field');
           show('Netflow_Remote_PORT0_field');
           show('Netflow_Remote_Exp0_field');
           show('Netflow_Remote_SIP0_field');
           show('Netflow_Remote_Version0_field');
        }

        v = isset('Netflow_Remote_Status1_add', 'A');
        if (v) {
           hide('Netflow_Remote_IF1_field');
           hide('Netflow_Remote_IP1_field');
           hide('Netflow_Remote_PORT1_field');
           hide('Netflow_Remote_Exp1_field');
           hide('Netflow_Remote_SIP1_field');
           hide('Netflow_Remote_Version1_field');
        }
        else {
           show('Netflow_Remote_IP1_field');
           show('Netflow_Remote_IF1_field');
           show('Netflow_Remote_PORT1_field');
           show('Netflow_Remote_Exp1_field');
           show('Netflow_Remote_SIP1_field');
           show('Netflow_Remote_Version1_field');
        }

        v = isset('Netflow_Remote_Status2_add', 'A');
        if (v) {
           hide('Netflow_Remote_IF2_field');
           hide('Netflow_Remote_IP2_field');
           hide('Netflow_Remote_PORT2_field');
           hide('Netflow_Remote_Exp2_field');
           hide('Netflow_Remote_SIP2_field');
           hide('Netflow_Remote_Version2_field');
        }
        else {
           show('Netflow_Remote_IP2_field');
           show('Netflow_Remote_IF2_field');
           show('Netflow_Remote_PORT2_field');
           show('Netflow_Remote_Exp2_field');
           show('Netflow_Remote_SIP2_field');
           show('Netflow_Remote_Version2_field');
        }

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


footer ?>
<!--
##WEBIF:name:Tools:100:Netflow Report
-->
