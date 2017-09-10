#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"
###################################################################
# HOTSPOT configuration page
#
# Description:
#	Use coova-chilli (based on chillispot) as default
#
# Configuration files referenced:
#   /etc/config/coova-chilli
#

uci_load coova-chilli
hotspot="coova_chilli"

if empty "$FORM_submit"; then
       config_get FORM_coova_status "hs_config" enabled
       config_get FORM_coova_nasid $hotspot NASID
       config_get FORM_coova_network $hotspot NETWORK
       config_get FORM_coova_network_mask $hotspot NETMASK
       config_get FORM_coova_dns_domain $hotspot DNS_DOMAIN
       config_get FORM_coova_radius_server $hotspot RADIUS
       config_get FORM_coova_radius_server2 $hotspot RADIUS2
       config_get FORM_coova_uam_redirect $hotspot UAMFORMAT
       config_get FORM_coova_uam_secret $hotspot UAMSECRET
       config_get FORM_coova_radius_secret $hotspot RADSECRET
else
       eval FORM_coova_status="\$FORM_${hotspot}_coova_status"
       eval FORM_coova_nasid="\$FORM_${hotspot}_coova_nasid"
       eval FORM_coova_network="\$FORM_${hotspot}_coova_network"
       eval FORM_coova_network_mask="\$FORM_${hotspot}_coova_network_mask"
       eval FORM_coova_dns_domain="\$FORM_${hotspot}_coova_dns_domain"
       eval FORM_coova_radius_server="\$FORM_${hotspot}_coova_radius_server"
       eval FORM_coova_radius_server2="\$FORM_${hotspot}_coova_radius_server2"
       eval FORM_coova_uam_redirect="\$FORM_${hotspot}_coova_uam_redirect"
       eval FORM_coova_uam_secret="\$FORM_${hotspot}_coova_uam_secret"
       eval FORM_coova_radius_secret="\$FORM_${hotspot}_coova_radius_secret"

       validate <<EOF
ip|FORM_${hotspot}_coova_network|$hotspot @TR<<IP Address>>||$FORM_coova_network
netmask|FORM_${hotspot}_coova_network_mask|$hotspot @TR<<Netmask>>||$FORM_coova_network_mask
EOF

       [ "$?" = "0" ] && {
         uci_set "coova-chilli" "hs_config" "enabled" "$FORM_coova_status"
         uci_set "coova-chilli" "$hotspot" "NASID" "$FORM_coova_nasid"
         uci_set "coova-chilli" "$hotspot" "NETWORK" "$FORM_coova_network"
         uci_set "coova-chilli" "$hotspot" "NETMASK" "$FORM_coova_network_mask"
         uci_set "coova-chilli" "$hotspot" "DNS_DOMAIN" "$FORM_coova_dns_domain"
         uci_set "coova-chilli" "$hotspot" "RADIUS" "$FORM_coova_radius_server"
         uci_set "coova-chilli" "$hotspot" "RADIUS2" "$FORM_coova_radius_server2"
         uci_set "coova-chilli" "$hotspot" "UAMFORMAT" "$FORM_coova_uam_redirect"
         uci_set "coova-chilli" "$hotspot" "UAMSECRET" "$FORM_coova_uam_secret"
         uci_set "coova-chilli" "$hotspot" "RADSECRET" "$FORM_coova_radius_secret"

     }

fi

hotspot_options="start_form|Hotspot @TR<<Configuration>>

field|@TR<<Hotspot status>>
select|${hotspot}_coova_status|$FORM_coova_status
option|1|@TR<<Enable>>
option|0|@TR<<Disable>>

field|@TR<<Radius NAS ID>>|${hotspot}_coova_nasid|hidden
text|${hotspot}_coova_nasid|$FORM_coova_nasid

field|@TR<<Hotspot Network>>|${hotspot}_coova_network|hidden
text|${hotspot}_coova_network|$FORM_coova_network

field|@TR<<Hotspot Netmask>>|${hotspot}_coova_network_mask|hidden
text|${hotspot}_coova_network_mask|$FORM_coova_network_mask

field|@TR<<DNS Domain>>|${hotspot}_coova_dns_domain|hidden
text|${hotspot}_coova_dns_domain|$FORM_coova_dns_domain

field|@TR<<Radius Server 1>>|${hotspot}_coova_radius_server
text|${hotspot}_coova_radius_server|$FORM_coova_radius_server|||style=\"width:360px\"

field|@TR<<Radius Server 2>>|${hotspot}_coova_radius_server2
text|${hotspot}_coova_radius_server2|$FORM_coova_radius_server2|||style=\"width:360px\"

field|@TR<<Redirect URL>>|${hotspot}_coova_uam_redirect
text|${hotspot}_coova_uam_redirect|$FORM_coova_uam_redirect|||style=\"width:360px\"

field|@TR<<UAM Secret>>|${hotspot}_coova_uam_secret
text|${hotspot}_coova_uam_secret|$FORM_coova_uam_secret

field|@TR<<Radius Secret>>|${hotspot}_coova_radius_secret
text|${hotspot}_coova_radius_secret|$FORM_coova_radius_secret

end_form
"

append forms "$hotspot_options" "$N"

###################################################################
# set JavaScript
javascript_forms="
    if (isset('${hotspot}_coova_status','0')) 
    {
        set_visible('${hotspot}_coova_nasid', 0);
        set_visible('${hotspot}_coova_network', 0);
        set_visible('${hotspot}_coova_network_mask', 0);
        set_visible('${hotspot}_coova_dns_domain', 0);
        set_visible('${hotspot}_coova_radius_server', 0);
        set_visible('${hotspot}_coova_radius_server2', 0);
        set_visible('${hotspot}_coova_uam_redirect', 0);
        set_visible('${hotspot}_coova_uam_secret', 0);
        set_visible('${hotspot}_coova_radius_secret', 0);
    }
    else
    {
        set_visible('${hotspot}_coova_nasid', 1);
        set_visible('${hotspot}_coova_network', 1);
        set_visible('${hotspot}_coova_network_mask', 1);
        set_visible('${hotspot}_coova_dns_domain', 1);
        set_visible('${hotspot}_coova_radius_server', 1);
        set_visible('${hotspot}_coova_radius_server2', 1);
        set_visible('${hotspot}_coova_uam_redirect', 1);
        set_visible('${hotspot}_coova_uam_secret', 1);
        set_visible('${hotspot}_coova_radius_secret', 1);
    }"

append js "$javascript_forms" "$N"

header "Wireless" "HotSpot" "@TR<<Hotspot Configuration>>" ' onload="modechange()" ' "$SCRIPT_NAME"

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
##WEBIF:name:Wireless:500:HotSpot
-->
