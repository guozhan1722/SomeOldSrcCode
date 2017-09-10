#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

#save vpn client access
if ! empty "$FORM_submit"; then
    FORM_usr_name=$(cat /tmp/vca/editname)
    FORM_pwd1="$FORM_tunnelpwd1"
    FORM_pwd2="$FORM_tunnelpwd2"
    [ "$FORM_pwd1" != "$FORM_pwd2" ] && {
              pwd_error="@TR<<Mismatch password>>"
    } || {
              pwd_error=""
    }

validate <<EOF
string|FORM_tunnelpwd1|@TR<<New Password>>|required min=5|$FORM_pwd1
merror|FORM_tunnelpwd2|@TR<<Confirm New Password>>||$pwd_error
EOF
            
        equal "$?" 0 && {
            uci_set "ipsec" "$FORM_usr_name" "pwd" "$FORM_pwd2"
            rm -rf /tmp/vcaeditname >&- 2>&-
        }

fi

header_vpn "VPN" "VPN Client Access" "@TR<<VPN Client Access>>" '' "$SCRIPT_NAME"

if ! empty "$FORM_editname"; then
    FORM_usr_name="$FORM_editname"
    [ -d "/tmp/vca" ] || mkdir /tmp/vca
    echo $FORM_editname > /tmp/vca/editname
else
    FORM_usr_name=$(cat /tmp/vca/editname)
fi

    uci_load ipsec
    config_get FORM_pwd1      $FORM_usr_name pwd
    config_get FORM_pwd2      $FORM_usr_name pwd

display_form <<EOF
start_form|
EOF

cat <<EOF
<tr><td>@TR<<Username>></td>
<td><input name="tunnelname" value="${FORM_usr_name}" disabled="disabled"></td>
</tr>
EOF

display_form <<EOF 
field|@TR<<New Password>>
password|tunnelpwd1|$FORM_pwd1
field|@TR<<Confirm New Password>>
password|tunnelpwd2|$FORM_pwd2
end_form
EOF

footer_vpn "ipsec-s2s-summary.sh" "update_vca" "$FORM_usr_name" ?> 


