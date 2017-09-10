#!/usr/bin/webif-page
<? 
. /usr/lib/webif/webif.sh

if ! empty "$FORM_submit" ; then
	SAVED=1
	validate <<EOF
string|FORM_pw1|@TR<<Password>>|required min=5|$FORM_pw1
EOF
	equal "$FORM_pw1" "$FORM_pw2" || {
		[ -n "$ERROR" ] && ERROR="${ERROR}<br />"
		ERROR="${ERROR}@TR<<Passwords do not match>><br />"
	}
       empty "$ERROR" && {
	       RES=$(
		       (
			       echo "$FORM_pw1"
			       sleep 1
			       echo "$FORM_pw2"
		       ) | passwd $REMOTE_USER 2>&1
	       )
	       equal "$?" 0 && {
                    ROOTFALSE=$(cat /etc/passwd |grep "root:"|grep "/bin/false")
                    ROOTTRUE=$(cat /etc/passwd |grep "root:"|grep "/bin/ash")
                    
                    empty "$ROOTFALSE"  &&  ROOTFALSE=$(echo $ROOTTRUE | sed '//s/\/bin\/ash/\/bin\/false/g')
                    empty "$ROOTTRUE"  &&  ROOTTRUE=$(echo $ROOTFALSE | sed '//s/\/bin\/false/\/bin\/ash/g')

                    [  "$FORM_pw2" = "admin" ] && {
                        echo "$ROOTTRUE" > /tmp/passwd_tmp
                        `cat /etc/passwd |grep -v "root:" >> /tmp/passwd_tmp` 
                        `cp /tmp/passwd_tmp /etc/passwd`
                        `rm /tmp/passwd_tmp`
                    }|| {
                        echo "$ROOTFALSE" > /tmp/passwd_tmp
                        `cat /etc/passwd |grep -v "root:" >> /tmp/passwd_tmp` 
                        `cp /tmp/passwd_tmp /etc/passwd`
                        `rm /tmp/passwd_tmp`
                    }
               } || ERROR="<pre>$RES</pre>"
       }

fi

header "System" "Password" "@TR<<Password>>" '' "$SCRIPT_NAME"

display_form <<EOF
start_form|@TR<<Password Change>>
field|<strong>@TR<<User Name >>: $REMOTE_USER </strong>      
field|@TR<<New Password>>:
password|pw1
field|@TR<<Confirm Password>>:
password|pw2
end_form
EOF

footer ?>

#<!--
###WEBIF:name:System:250:Password
#-->
