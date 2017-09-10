#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

###################################################################
#
# System configuration page
#
# Description:
#	Configures access control for the webif
#
# Author(s) [in order of work date]:
#	Travis Kemen <kemen04@gmail.com>
#
# Major revisions:
#
# Configuration files referenced:
#   /etc/config/webif_access_control
#

cachedir=/tmp/.webcache
#admin is also a superuser but maybe removed in the future.
superuser=admin
mkdir /tmp/.webif/
exists /etc/config/webif_access_control || touch /etc/config/webif_access_control

if ! empty "$FORM_passwdchange" ; then
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

if [ "$FORM_user_add" != "" ]; then
	validate <<EOF
string|FORM_password_add|@TR<<Password>>|required min=5|$FORM_password_add
EOF
	equal "$FORM_password_add" "$FORM_password2_add" || {
		[ -n "$ERROR" ] && ERROR="${ERROR}<br />"
		ERROR="${ERROR}@TR<<password_mismatch#The passwords do not match!>><br />"
	}
	if [ "${FORM_user_add}" = "root" -o "${FORM_user_add}" = "admin" -o "${FORM_user_add}" = "upgrade" -o "${FORM_user_add}" = "nobody" ]; then
		[ -n "$ERROR" ] && ERROR="${ERROR}<br />"
		ERROR="${ERROR}@TR<<root, admin, upgrade and nobody are already users>>.<br />"
	fi

	if [ "$REMOTE_USER" != "admin" ]; then
		[ -n "$ERROR" ] && ERROR="${ERROR}<br />"
		ERROR="${ERROR}@TR<< Only admin can add users>>.<br />"
	fi

	[ -e /tmp/.webif/file-httpd.conf ] && HTTPD_CONFIG_FILE=/tmp/.webif/file-httpd.conf
	[ -e /etc/httpd.conf ] && HTTPD_CONFIG_FILE="$HTTPD_CONFIG_FILE /etc/httpd.conf"
	[ -n "HTTPD_CONFIG_FILE" ] && grep -q "$FORM_user_add" $HTTPD_CONFIG_FILE
	if [ "$?" = "0" ]; then
		[ -n "$ERROR" ] && ERROR="${ERROR}<br />"
		ERROR="${ERROR}$FORM_user_add @TR<<already exists.>>.<br />"
	fi
	empty "$ERROR" && {
                deluser $FORM_user_add
                delgroup $FORM_user_add
		RES=$(
		       (
			       echo "$FORM_password_add"
			       sleep 1
			       echo "$FORM_password2_add"
		       ) | adduser -s /bin/false $FORM_user_add 
	       )
	       equal "$?" 0 && {
                  #password=$(uhttpd -m $FORM_password_add)
                  	[ -e /tmp/.webif/file-httpd.conf ] || cp /etc/httpd.conf /tmp/.webif/file-httpd.conf
                  	echo "/cgi-bin/webif/:${FORM_user_add}:\$p\$${FORM_user_add}" >>/tmp/.webif/file-httpd.conf
                  	uci_add "webif_access_control" "accesscontrol" "${FORM_user_add}"
                  	unset FORM_user_add
               }|| ERROR="<pre>$RES</pre>"
	}
	unset FORM_submit
fi
#uci_load "webif_access_control"
header "System" "Access Control" "@TR<<Access Control>>" '' "$SCRIPT_NAME"

display_form <<EOF
start_form|@TR<<Password Change>>
field|<strong>@TR<<User Name >>: $REMOTE_USER </strong>      
field|@TR<<New Password (min 5 characters)>>:
password|pw1
field|@TR<<Confirm Password>>:
password|pw2
submit|passwdchange| @TR<<Change Passwd>> |
end_form
EOF

exists /tmp/.webif/file-httpd.conf && HTTPD_CONFIG_FILE=/tmp/.webif/file-httpd.conf || HTTPD_CONFIG_FILE=/etc/httpd.conf

awk -F: -v "cachedir=$cachedir" -v "superuser=$superuser" '
BEGIN { ucount = 0;
	include("/usr/lib/webif/common.awk");
        config_load("webif_access_control");
	start_form("@TR<<Users>>");

	if (ENVIRON["FORM_submit"] != "") {
		if (ENVIRON["FORM_remove_user_"$2] != "") {
			system("/bin/rm -f " cachedir "/cat_" $2);
			system("/bin/rm -f " cachedir "/subcat_" $2);
			system("/bin/rm -f /tmp/.webif/file-httpd.conf.2 ;touch /tmp/.webif/file-httpd.conf.2");
                }
        } 
}
($1 == "/cgi-bin/webif/" && $2 != superuser && $2 != "root") {
	if (ENVIRON["FORM_remove_user_" $2] == "") {
		field($2);
		print "<span class=\"smalltext\"><a href=\"" ENVIRON["SCRIPT_NAME"] "?remove_user_" $2 "=" $2 "&submit=1" "\">@TR<<system_acl_remove_user#Remove user>> " $2 "</a></span>";
		ucount = ucount + 1;
	}
}
((ENVIRON["FORM_submit"] != "") && ($1 != "")) {
	if (($1 == "/cgi-bin/webif/") && (ENVIRON["FORM_remove_user_"$2] == "") ) {
		print $1":"$2":"$3 >>"/tmp/.webif/file-httpd.conf.2";
	} 
	
        if (ENVIRON["FORM_remove_user_"$2] != ""){
                uci_remove("webif_access_control", $2);
	}
	
	if ($1 != "/cgi-bin/webif/") {
		print $0 >>"/tmp/.webif/file-httpd.conf.2";
	}
	
}
END {
	if (ucount == 0)
		print "<tr><td>No users defined.";

	end_form("","",1);
	start_form("@TR<<Add User:  ( Note: Changes will not take effect until the system is rebooted )>>");
	field("@TR<<Username>>");
	textinput3("user_add", ENVIRON["FORM_user_add"]);
	field("@TR<<Password (min 5 characters)>>");
	password("password_add");
	field("@TR<<Confirm Password>>");
	password("password2_add");
	field("&nbsp;","");
	submit("add_user", "@TR<<Add User>>");
	end_form("","",1);
}' $HTTPD_CONFIG_FILE 2>/dev/null

[ -e /tmp/.webif/file-httpd.conf.2 ] && mv -f /tmp/.webif/file-httpd.conf.2 /tmp/.webif/file-httpd.conf
exists /tmp/.webif/file-httpd.conf && HTTPD_CONFIG_FILE=/tmp/.webif/file-httpd.conf || HTTPD_CONFIG_FILE=/etc/httpd.conf
users=$(awk -F":" '($1 == "/cgi-bin/webif/") { if (($2 != "root") && ($2 != "admin")) print $2 }' $HTTPD_CONFIG_FILE 2>/dev/null)

for user in $users; do
	export user
	#[ -s $cachedir/subcat_${user} ] || init_cache "$user"
	awk -F: '
BEGIN {
	include("/usr/lib/webif/common.awk")
	//config_load("webif_access_control")
	start_form("@TR<<ACL User>>: " ENVIRON["user"])
}
{
	if (ENVIRON["FORM_submit"] == "") {
		if ($1 != "Graphs") {
			var = config_get_bool(ENVIRON["user"], $1"_"$2, "0")
		} else {
			var = config_get_bool(ENVIRON["user"], "Graphs", "0")
		}
	} else {
		if ($1 != "Graphs") {
			varorig = config_get_bool(ENVIRON["user"], $1"_"$2, "0")
			var2 = "FORM_" ENVIRON["user"] "_" $1 "_" $2
			var = ENVIRON[var2]
			if (varorig != var) {
				uci_set("webif_access_control", ENVIRON["user"], $1 "_" $2, var)
			}
		} else {
			varorig = config_get_bool(ENVIRON["user"], "Graphs", "0")
			var2 = "FORM_" ENVIRON["user"] "_Graphs"
			var = ENVIRON[var2]
			if (varorig != var) {
				uci_set("webif_access_control", ENVIRON["user"], "Graphs", var)
			}
		}
	}
	if (($1 != category) && ($1 != GRAPHS)) {
		field("<h2>@TR<<"$1">></h2>")
	}
	category = $1
	if ($1 != GRAPHS) {
		if ($1 == "Graphs") {
			GRAPHS = $1
			field("@TR<<Graphs>>")
			select(ENVIRON["user"]"_Graphs", var)
		} else {
			field("@TR<<"$3">>")
			select(ENVIRON["user"]"_"$1"_"$2, var)
		}
		option("1", "@TR<<Enabled>>")
		option("0", "@TR<<Disabled>>")
		print "</select>"
		select_open = 0
	}
}
	END {
		end_form("","",1)
	}' $cachedir/subcat_$superuser
done

footer

?>

<!--
##WEBIF:name:System:160:Access Control
-->
