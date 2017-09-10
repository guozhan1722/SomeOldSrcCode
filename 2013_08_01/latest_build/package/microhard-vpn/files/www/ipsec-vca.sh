#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

#save a vpn client access
if ! empty "$FORM_submit"; then
    form_valid="false"
    FORM_usr_name="$FORM_tunnelname"
    FORM_pwd1="$FORM_tunnelpwd1"
    FORM_pwd2="$FORM_tunnelpwd2"
    res=$(uci get ipsec.${FORM_usr_name})

    [ "$res" = "vca" ] && {
              name_error="@TR<<Conflict name>>"
    } || {
              name_error=""
    }

    [ "$FORM_pwd1" != "$FORM_pwd2" ] && {
              pwd_error="@TR<<Mismatch password>>"
    } || {
              pwd_error=""
    }

validate <<EOF
string|FORM_tunnelname|@TR<<Username>>|required|$FORM_usr_name
merror|FORM_tunnelname|@TR<<Username>>||$name_error
string|FORM_tunnelpwd1|@TR<<New Password>>|required min=5|$FORM_pwd1
merror|FORM_tunnelpwd2|@TR<<Confirm New Password>>||$pwd_error
EOF
            
        equal "$?" 0 && {
	    uci_add "ipsec" "vca" "$FORM_usr_name"
            uci_set "ipsec" "$FORM_usr_name" "pwd" "$FORM_pwd2"
        }

fi

header_vpn "VPN" "VPN Client Access" "@TR<<VPN Client Access>>" '' "$SCRIPT_NAME"

display_form <<EOF
start_form|
field|@TR<<Username>>
text|tunnelname||
field|@TR<<New Password>>
password|tunnelpwd1|$FORM_pwd1
field|@TR<<Confirm New Password>>
password|tunnelpwd2|$FORM_pwd2
end_form
EOF

footer_vpn "ipsec-s2s-summary.sh" "update_vca" "$FORM_usr_name" ?> 

<!--
##WEBIF:name:VPN:350:VPN Client Access
-->
