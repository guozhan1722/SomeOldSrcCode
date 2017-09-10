#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"



uci_load ioports

if empty "$FORM_submit"; then
       config_get FORM_output1 output output1
       config_get FORM_output2 output output2
       config_get FORM_output3 output output3
       config_get FORM_output4 output output4
else
       FORM_output1="$FORM_output1"
       FORM_output2="$FORM_output2"
       FORM_output3="$FORM_output3"
       FORM_output4="$FORM_output4"


#validate <<EOF
#int|FORM_connect_timeout_carrier|@TR<<Connect Timeout>>||$FORM_connect_timeout
#EOF
       [ "$?" = "0" ] && {
            uci_set "ioports" "output" "output1" "$FORM_output1"
            uci_set "ioports" "output" "output2" "$FORM_output2"
            uci_set "ioports" "output" "output3" "$FORM_output3"
            uci_set "ioports" "output" "output4" "$FORM_output4"
         }
fi

#echo "hello $FORM_output1 $FORM_output2 $FORM_output3 $FORM_output4"
   
####### Rendering the webUI #############################   
output_options="start_form| 

field|@TR<<OUTPUT 1>>
radio|output1|$FORM_output1|0|@TR<<Open>>
radio|output1|$FORM_output1|1|@TR<<Close>>

field|@TR<<OUTPUT 2>>
radio|output2|$FORM_output2|0|@TR<<Open>>
radio|output2|$FORM_output2|1|@TR<<Close>>

field|@TR<<OUTPUT 3>>
radio|output3|$FORM_output3|0|@TR<<Open>>
radio|output3|$FORM_output3|1|@TR<<Close>>

field|@TR<<OUTPUT 4>>
radio|output4|$FORM_output4|0|@TR<<Open>>
radio|output4|$FORM_output4|1|@TR<<Close>>

end_form"
append forms "$output_options" "$N"


###################################################################
# set JavaScript
javascript_forms="
//    v = isset('disabled_carrier', '0')
//    set_visible('apn_carrier_field', v);

//    v = isset('icmp_alive_check_carrier', 'enable')
//    set_visible('icmp_hostname_field', v); 
      "

append js "$javascript_forms" "$N"

header "I\/O" "OUTPUT" "@TR<<OUTPUT Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"


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

footer ?>
<!--
##WEBIF:name:I/O:200:OUTPUT
-->
