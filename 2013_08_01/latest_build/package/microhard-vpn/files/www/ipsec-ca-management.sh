#!/usr/bin/webif-page "-U /tmp -u 16384"
<?
. /usr/lib/webif/webif.sh

cur_color="even"
get_tr() {
	if equal "$cur_color" "odd"; then
		cur_color="even"
		tr="string|<tr>"
	else
		cur_color="odd"
		tr="string|<tr class=\"odd\">"
	fi
}
header "VPN" "Certificate Management" "@TR<<Certificate Management>>" '' "$SCRIPT_NAME"
#remove rootca
if ! empty "$FORM_remove_root_ca"; then
        /bin/rm -f /etc/ipsec.d/cacerts/${FORM_remove_root_ca} >/dev/null 2>&1
fi

#remove clientca
if ! empty "$FORM_remove_client_ca"; then
        /bin/rm -f /etc/ipsec.d/certs/${FORM_remove_client_ca} >/dev/null 2>&1
fi

#remove privatekey
if ! empty "$FORM_remove_private_key"; then
        /bin/rm -f /etc/ipsec.d/private/${FORM_remove_private_key} >/dev/null 2>&1
fi

#remove crls
if ! empty "$FORM_remove_crls"; then
        /bin/rm -f /etc/ipsec.d/crls/${FORM_remove_crls} >/dev/null 2>&1
fi

#import rootca
if ! equal $FORM_import_root_ca "" ; then
        if empty "$FORM_importrootca_name"; then
            echo "<br /><p><font color=\"red\">No file was imported !!! </p><br />"
        else
            mv $FORM_importrootca /etc/ipsec.d/cacerts/${FORM_importrootca_name}
        fi
fi

#import clientca
if ! equal $FORM_import_client_ca "" ; then
        if empty "$FORM_importclientca_name"; then
            echo "<br /><p><font color=\"red\">No file was imported !!! </p><br />"
        else
            mv $FORM_importclientca /etc/ipsec.d/certs/${FORM_importclientca_name}
        fi
fi
    
#import private key
if ! equal $FORM_import_private_key "" ; then
        if empty "$FORM_importprivatekey_name"; then
            echo "<br /><p><font color=\"red\">No file was imported !!! </p><br />"
        else
            mv $FORM_importprivatekey /etc/ipsec.d/private/${FORM_importprivatekey_name}
        fi
fi    

#import crls
if ! equal $FORM_import_crls "" ; then
        if empty "$FORM_importcrls_name"; then
            echo "<br /><p><font color=\"red\">No file was imported !!! </p><br />"
        else
            mv $FORM_importcrls /etc/ipsec.d/crls/${FORM_importcrls_name}
        fi
fi

#summary of rootcas
form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<X509 Root Certificates>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<X509 Root Certificates>>\">
	string|<tr><th>@TR<<No.>></th>
	string|<th>@TR<<Name>></th>
	string|<th>@TR<<Config.>></th>
	string|</tr>"
append forms "$form" "$N"

cfgcnt="0"

rootca_cfgs=$(ls /etc/ipsec.d/cacerts)
for rootca in $rootca_cfgs; do
    let "cfgcnt+=1"
    
    get_tr
    form="$tr  
            string|<td>
            string|$cfgcnt
            string|</td>

            string|<td>
            string|$rootca
            string|</td>

            string|<td>
            string|<a href=\"$SCRIPT_NAME?remove_root_ca=$rootca\">@TR<<Remove>></a>
            string|</td>

            string|</tr>"

    append forms "$form" "$N"
done

get_tr
if [ "$cfgcnt" -lt "16" ]; then
form="$tr
        string|<td>
        string|@TR<<Import Certificate:>>
        string|</td><td>
        upload|importrootca|$FORM_importrootca_name
        string|</td><td><input type=\"submit\" class=\"buttons\" name=\"import_root_ca\" value=\"@TR<<Import>>\">
        string|</td></tr>
        string|</table></div>
        string|<br/>"
else
form="  string|</table></div>
        string|<br/>"
fi

append forms "$form" "$N"

#summary of clientcas
form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<X509 Certificates>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<X509 Root Certificates>>\">
	string|<tr><th>@TR<<No.>></th>
	string|<th>@TR<<Name>></th>
	string|<th>@TR<<Config.>></th>
	string|</tr>"
append forms "$form" "$N"

cfgcnt="0"

clientca_cfgs=$(ls /etc/ipsec.d/certs)
for clientca in $clientca_cfgs; do
    let "cfgcnt+=1"
    
    get_tr
    form="$tr  
            string|<td>
            string|$cfgcnt
            string|</td>

            string|<td>
            string|$clientca
            string|</td>

            string|<td>
            string|<a href=\"$SCRIPT_NAME?remove_client_ca=$clientca\">@TR<<Remove>></a>
            string|</td>

            string|</tr>"

    append forms "$form" "$N"
done

get_tr
if [ "$cfgcnt" -lt "16" ]; then
form="$tr
        string|<td>
        string|@TR<<Import Certificate:>>
        string|</td><td>
        upload|importclientca|$FORM_importclientca_name
        string|</td><td><input type=\"submit\" class=\"buttons\" name=\"import_client_ca\" value=\"@TR<<Import>>\">
        string|</td></tr>
        string|</table></div>
        string|<br/>"
else
form="  string|</table></div>
        string|<br/>"
fi

append forms "$form" "$N"

#summary of privatekey
form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<X509 Private Keys>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<X509 Root Certificates>>\">
	string|<tr><th>@TR<<No.>></th>
	string|<th>@TR<<Name>></th>
	string|<th>@TR<<Config.>></th>
	string|</tr>"
append forms "$form" "$N"

cfgcnt="0"

privatekey_cfgs=$(ls /etc/ipsec.d/private)
for pkey in $privatekey_cfgs; do
    let "cfgcnt+=1"
    
    get_tr
    form="$tr  
            string|<td>
            string|$cfgcnt
            string|</td>

            string|<td>
            string|$pkey
            string|</td>

            string|<td>
            string|<a href=\"$SCRIPT_NAME?remove_private_key=$pkey\">@TR<<Remove>></a>
            string|</td>

            string|</tr>"

    append forms "$form" "$N"
done

get_tr
if [ "$cfgcnt" -lt "16" ]; then
form="$tr
        string|<td>
        string|@TR<<Import Private key:>>
        string|</td><td>
        upload|importprivatekey|$FORM_importprivatekey_name
        string|</td><td><input type=\"submit\" class=\"buttons\" name=\"import_private_key\" value=\"@TR<<Import>>\">
        string|</td></tr>
        string|</table></div>
        string|<br/>"
else
form="  string|</table></div>
        string|<br/>"
fi
append forms "$form" "$N"

#summary of crls
form="  string|<div class=\"settings\">
	string|<h3><strong>@TR<<X509 Certificates Revocation Lists>></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<X509 Root Certificates>>\">
	string|<tr><th>@TR<<No.>></th>
	string|<th>@TR<<Name>></th>
	string|<th>@TR<<Config.>></th>
	string|</tr>"
append forms "$form" "$N"

cfgcnt="0"

crls_cfgs=$(ls /etc/ipsec.d/crls)
for crls in $crls_cfgs; do
    let "cfgcnt+=1"
    
    get_tr
    form="$tr  
            string|<td>
            string|$cfgcnt
            string|</td>

            string|<td>
            string|$crls
            string|</td>

            string|<td>
            string|<a href=\"$SCRIPT_NAME?remove_crls=$crls\">@TR<<Remove>></a>
            string|</td>

            string|</tr>"

    append forms "$form" "$N"
done

get_tr
if [ "$cfgcnt" -lt "16" ]; then
form="$tr
        string|<td>
        string|@TR<<Import Certificate:>>
        string|</td><td>
        upload|importcrls|$FORM_importcrls_name
        string|</td><td><input type=\"submit\" class=\"buttons\" name=\"import_crls\" value=\"@TR<<Import>>\">
        string|</td></tr>
        string|</table></div>
        string|<br/>"
else
form="  string|</table></div>
        string|<br/>"
fi

append forms "$form" "$N"

display_form <<EOF
$forms
EOF

footer_nosubmit ?> 
<!--
##WEBIF:name:VPN:500:Certificate Management
-->
