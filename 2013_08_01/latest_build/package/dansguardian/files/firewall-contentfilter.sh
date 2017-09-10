#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"

	case "$cfg_type" in
		cf_general)
			append cf_cfgs "$cfg_name" "$N"
		;;
		site)
	                append cf_sites "$cfg_name" "$N"
		;;
	esac
}

#remove site
if ! empty "$FORM_remove_cf_site"; then
	uci_remove "dansguardian" "$FORM_remove_cf_site"
fi

if empty "$FORM_submit"; then
    #refresh display
    #config_get FORM_mode dg_config mode
    FORM_mode=$(uci get dansguardian.dg_config.mode)
else
    #save mode
    eval FORM_mode="\$FORM_cf_mode"
    uci_set dansguardian dg_config mode $FORM_mode
    
#add site
if ! empty "$FORM_add_cf_site"; then
    added_error=""
    un_names=$(cat /var/run/dansguardian_cf_sites)
    for chname in $un_names; do
        if [ "$chname" = "$FORM_name_cf_site" ]; then
            append added_error "Error in Name: Route Name not unique" "$N"
            break
        fi
    done
validate <<EOF
string|FORM_name_cf_site|@TR<<Name>>|required|$FORM_name_cf_site
EOF
	[ "$?" = "0" -a "$added_error" = "" ] && {
		uci_add dansguardian site
                add_site_cfg="$CONFIG_SECTION"
		uci_set dansguardian "$add_site_cfg" site_name "$FORM_name_cf_site"
                uci_set dansguardian "$add_site_cfg" url ""
                uci_set dansguardian "$add_site_cfg" ip_addr ""
                logger "Added a site to site list: $FORM_name_cf_site. Mode=$FORM_mode"
		unset FORM_name_cf_site FORM_add_cf_site
	}
        append  added_error  "$ERROR" "$N"
fi
fi

cur_color="odd"
get_tr() {
	if equal "$cur_color" "odd"; then
		cur_color="even"
		tr="string|<tr>"
	else
		cur_color="odd"
		tr="string|<tr class=\"odd\">"
	fi
}

uci_load dansguardian

#FIXME: uci_load bug
#uci_load will pass the same config twice when there is a section to be added by using uci_add before a uci_commit happens
#we will use uniq so we don't try to parse the same config section twice.
cf_sites=$(echo "$cf_sites" |uniq)

#init the adding field
FORM_name_cf_site="name.com"

#Web page form1
form="start_form| Configuration
field|@TR<<Working Mode>>
select|cf_mode|$FORM_mode
option|black_list|@TR<<Black List>>
option|white_list|@TR<<White List>>
end_form"

append forms "$form" "$N"

#Web page form2
form="start_form|Site Name
        field|@TR<<Site Name>>
        text|name_cf_site|$FORM_name_cf_site
	submit|add_cf_site|@TR<<Add>>
        end_form"
append forms "$form" "$N"

#Web page form3
form="string|<div class=\"settings\">
	string|<h3><strong>@TR<< Site List >></strong></h3>
	string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"2\" summary=\"@TR<<>>\">
	string|<th width="50%">@TR<<Name>></th>
	string|<th width="50%">@TR<<Action>></th>
	string|</tr>"
append forms "$form" "$N"

sitecnt=0
uniq_names=""
rm -f /var/run/dansguardian_cf_sites

for site in $cf_sites; do
      		config_get FORM_cf_site_name "$site" site_name

                append uniq_names  "$FORM_cf_site_name" "$N"
                echo "$uniq_names" > /var/run/dansguardian_cf_sites 
                
                # name=$(echo "$site")
                name=$FORM_cf_site_name
                get_tr
        	form="$tr  
                        string|<td>
                        string|$name
                        string|</td>
        		string|<td>
        		string|<a href=\"$SCRIPT_NAME?remove_cf_site=$site\">@TR<<Remove >></a>
        		string|</td>
        		string|</tr>"
        	append forms "$form" "$N"
                let "sitecnt+=1"
done

form="	string|</tr>
	string|</table></div>"
append forms "$form" "$N"

ERROR=$added_error
header "Firewall" "Content Filtering" "@TR<<Content Filtering>>" 'onload="modechange()"' "$SCRIPT_NAME"

enabled_contentfilter=$(uci get tinyproxy.@tinyproxy[0].enabled)

if [ "$enabled_contentfilter" != "1" ]; then
display_form <<EOF
string|<tr><td colspan=\"2\"><h3 class=\"warning\">Content Filtering is Disabled </h3></td></tr>
EOF
else
display_form <<EOF
$added_error
$forms
EOF
fi
footer ?>
<!--
##WEBIF:name:Firewall:600:Content Filtering
-->
