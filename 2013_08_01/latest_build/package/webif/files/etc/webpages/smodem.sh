#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

uci_load smodem

if [ "$FORM_submit" = "" ]; then
    config_get FORM_enable port Enable
    config_get FORM_Data_Port_Number port Data_Port_Number
    config_get FORM_Diag_Port_Number port Diag_Port_Number
else
    eval FORM_enable="$FORM_enable_g"
    eval FORM_Data_Port_Number="$FORM_Data_Port_Number_g"
    eval FORM_Diag_Port_Number="$FORM_Diag_Port_Number_g"

validate <<EOF
int|FORM_Data_Port_Number|@TR<<Data Port>>|min=30000 max=65535|$FORM_Data_Port_Number
int|FORM_Diag_Port_Number|@TR<<Diag Port>>|min=30000 max=65535|$FORM_Diag_Port_Number
EOF
    equal "$?" 0 && {
        uci_set smodem port Enable "$FORM_enable"
        uci_set smodem port Data_Port_Number "$FORM_Data_Port_Number"
        uci_set smodem port Diag_Port_Number "$FORM_Diag_Port_Number"
    }
fi
   
form="start_form|SModem Configuration

            field|@TR<<Mode>>
            select|enable_g|$FORM_enable
            option|1|Enable
            option|0|Disable

            field|@TR<<Data Port>>|dataport_field_g|hidden
            text|Data_Port_Number_g|$FORM_Data_Port_Number

            field|@TR<<Diagnostic Port>>|diagport_field_g|hidden
            text|Diag_Port_Number_g|$FORM_Diag_Port_Number

            end_form"
append forms "$form" "$N"

javascript_forms="
       v = isset('enable_g', '1');
       set_visible('dataport_field_g', v);
       set_visible('diagport_field_g', v);
"
append js "$javascript_forms" "$N"


header "SModem" "SModem" "@TR<<Configuration>>" 'onload="modechange()"' "$SCRIPT_NAME"

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
$forms
EOF

footer ?>
<!--
##WEBIF:name:SModem:100:SModem
-->
