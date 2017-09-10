#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

header "System" "Command" "@TR<<Output>>:" '' "$SCRIPT_NAME"

if ! empty "$FORM_command"; then
   echo "<pre>#" $FORM_command
      $FORM_command
         echo "</pre></br>"
fi
         
display_form <<EOF
start_form|
field|@TR<<Command>>:
text|command
end_form
EOF

footer

?>
#<!--
###WEBIF:name:System:225:Command
#-->
