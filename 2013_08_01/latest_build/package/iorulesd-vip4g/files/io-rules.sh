#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

config_cb() {
	local cfg_type="$1"
	local cfg_name="$2"
        #logger "function config_cb(): cfg_type=$1 cfg_name=$2 rule_cfgs=$rule_cfgs"
	case "$cfg_type" in
		rule)
			append rule_cfgs "$cfg_name" "$N"                        
		;;
      	interface)
                        append netmode "$cfg_name" "$N"
                ;;
	esac
}

unique_error=0
name_count=0
un_names=$(cat /var/run/ioport_rule_name)
for chname in $un_names; do
    name_count=$(($name_count+1))
done


#remove rule
if ! empty "$FORM_remove_vcfg"; then
	uci_remove "iorules" "$FORM_remove_vcfg"
fi

if ! empty "$FORM_add_rule"; then
    #[ -z "$FORM_name_rule" ] && append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: Rule Name not unique</h3></td></tr>" "$N"

    un_names=$(cat /var/run/ioport_rule_name)
    for chname in $un_names; do
        if [ "$chname" = "CUSTOM_$FORM_name_rule" ]; then
            append added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">Error in Name: Rule Name not unique</h3></td></tr>" "$N"
            unique_error=1
            break
        fi
    done
validate <<EOF
string|FORM_name_rule|@TR<<Name>>|required|$FORM_name_rule
EOF
	equal "$?" 0 && [ "$unique_error" != "1" ] && {
                
		uci_add iorules rule "CUSTOM_$FORM_name_rule"; add_rule_cfg="$CONFIG_SECTION"
                uci_set iorules "$add_rule_cfg" rule_mode "$FORM_rule_mode"
                
                if [ "$FORM_rule_mode" = "INPUT_ONLY" ]; then
                    uci_set iorules "$add_rule_cfg" timercount "n/a"
                else
                    uci_set iorules "$add_rule_cfg" timercount "$FORM_timercount"
                fi
                if [ "$FORM_rule_mode" = "TIMER_ONLY" ]; then
                    uci_set iorules "$add_rule_cfg" input1 "n/a"
                    uci_set iorules "$add_rule_cfg" input2 "n/a"
                    uci_set iorules "$add_rule_cfg" input3 "n/a"
                    uci_set iorules "$add_rule_cfg" input4 "n/a"
                else                    
                    uci_set iorules "$add_rule_cfg" input1 "$FORM_input1"
                    uci_set iorules "$add_rule_cfg" input2 "$FORM_input2"
                    uci_set iorules "$add_rule_cfg" input3 "$FORM_input3"
                    uci_set iorules "$add_rule_cfg" input4 "$FORM_input4"
                fi                
                
                if [ "$FORM_output1" = "" ]; then value_output1="n/a"; else value_output1=$FORM_output1; fi
                if [ "$FORM_output2" = "" ]; then value_output2="n/a"; else value_output2=$FORM_output2; fi
                if [ "$FORM_output3" = "" ]; then value_output3="n/a"; else value_output3=$FORM_output3; fi
                if [ "$FORM_output4" = "" ]; then value_output4="n/a"; else value_output4=$FORM_output4; fi
                uci_set iorules "$add_rule_cfg" output1 "$value_output1"
                uci_set iorules "$add_rule_cfg" output2 "$value_output2"
                uci_set iorules "$add_rule_cfg" output3 "$value_output3"
                uci_set iorules "$add_rule_cfg" output4 "$value_output4"
                #logger "Add Rule: $FORM_rule_mode $FORM_timercount $FORM_input1 $FORM_input2 $FORM_input3 $FORM_input4 $FORM_output1 $FORM_output2 $FORM_output3 $FORM_output4"                
		unset FORM_name_rule FORM_rule_mode FORM_timercount FORM_input1 FORM_input2 FORM_input3 FORM_input4 FORM_output1 FORM_output2 FORM_output3 FORM_output4
                name_count=$(($name_count+1)) 
	} || {
                append  added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">$ERROR</h3></td></tr>" "$N"
                #logger ">>>>>>>>>>>Add Rule: not unique name"                

        }
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


uci_load iorules
if [ "$FORM_submit" = "" ]; then
    config_get FORM_rule_define "rule_define" rule_define

else
     eval FORM_rule_define="\$FORM_rule_define"
     equal "$?" 0 && {     
        uci_add iorules rule_define rule_define; 
        #uci_set iorules rule_define rule_define A
        uci_set iorules rule_define rule_define "$FORM_rule_define"
     }

fi



#init the adding field
#uci_load iorules
#FORM_rule_define=""
FORM_name_rule="rule$name_count"
FORM_rule_mode="TIMER_ONLY"
FORM_timercount="1"
FORM_input1="0"
FORM_input2="0"
FORM_input3="0"
FORM_input4="0"
FORM_output1="n/a"
FORM_output2="n/a"
FORM_output3="n/a"
FORM_output4="n/a"


#field|@TR<<  >>|IOPORT_default_rule_description_field|hidden|1   
#helptext|Helptext Wireless Distance#This is the distance of your longest link.
#radio|input1|$FORM_input1|n/a|@TR<<n/a>>            
#radio|input2|$FORM_input2|n/a|@TR<<n/a>>
#radio|input3|$FORM_input3|n/a|@TR<<n/a>> 
#radio|input4|$FORM_input4|n/a|@TR<<n/a>>

form="      start_form|I/O Rules Configuration
 
            field|@TR<< <strong>I/O Port Rule Define<strong> >>|IOPORT_rule_define_field|1
            select|rule_define|$FORM_rule_define
            option|A|@TR<<Disable>>
            option|B|@TR<<Default Rules>>
            option|C|@TR<<User Custom Rules>> 

 
            field|@TR<<RULE NAME>>|IOPORT_name_rule_field|hidden
	    text|name_rule|$FORM_name_rule
            
            string|<hr align=\"left\" width=\"20%\" />
            ############################################
            field|@TR<<I/O RULE MODE:>>|IOPORT_rule_mode_field|hidden|1           
                select|rule_mode|$FORM_rule_mode
                option|TIMER_ONLY|@TR<<Use Timer Only>>
                option|INPUT_ONLY|@TR<<Use Input States Only>>                
                option|INPUT_TIMER|@TR<<Use Input States With Timer>>
                
            field|@TR<<>>|IOPORT_timercount_field|hidden|1   
                text|timercount|$FORM_timercount|Seconds        
                               
            ############################################
            field|@TR<<INPUT EVENT:>>|IOPORT_input_event_field|hidden|1   
                
            field|&nbsp;&nbsp;&nbsp;&nbsp;@TR<<INPUT 1>>|IOPORT_input_1_field|hidden|1 

            radio|input1|$FORM_input1|0|@TR<<Open>>
            radio|input1|$FORM_input1|1|@TR<<Close>>
            
            field|&nbsp;&nbsp;&nbsp;&nbsp;@TR<<INPUT 2>>|IOPORT_input_2_field|hidden|1 
            radio|input2|$FORM_input2|0|@TR<<Open>>
            radio|input2|$FORM_input2|1|@TR<<Close>>
            
            field|&nbsp;&nbsp;&nbsp;&nbsp;@TR<<INPUT 3>>|IOPORT_input_3_field|hidden|1 
            radio|input3|$FORM_input3|0|@TR<<Open>>
            radio|input3|$FORM_input3|1|@TR<<Close>>
            
            field|&nbsp;&nbsp;&nbsp;&nbsp;@TR<<INPUT 4>>|IOPORT_input_4_field|hidden|1
            radio|input4|$FORM_input4|0|@TR<<Open>>
            radio|input4|$FORM_input4|1|@TR<<Close>>

            string|<hr align=\"left\" width=\"20%\" />
            #############################################

            field|@TR<<ACTION TO OUTPUT:>>|IOPORT_action_to_output_field|hidden|1 

            field|&nbsp;&nbsp;&nbsp;&nbsp;@TR<<OUTPUT 1>>|IOPORT_output_1_field|hidden|1 
            radio|output1|$FORM_output1|n/a|@TR<<n/a>>
            radio|output1|$FORM_output1|0|@TR<<Open>>
            radio|output1|$FORM_output1|1|@TR<<Close>>
            
            field|&nbsp;&nbsp;&nbsp;&nbsp;@TR<<OUTPUT 2>>|IOPORT_output_2_field|hidden|1 
            radio|output2|$FORM_output2|n/a|@TR<<n/a>>
            radio|output2|$FORM_output2|0|@TR<<Open>>
            radio|output2|$FORM_output2|1|@TR<<Close>>
            
            field|&nbsp;&nbsp;&nbsp;&nbsp;@TR<<OUTPUT 3>>|IOPORT_output_3_field|hidden|1 
            radio|output3|$FORM_output3|n/a|@TR<<n/a>>
            radio|output3|$FORM_output3|0|@TR<<Open>>
            radio|output3|$FORM_output3|1|@TR<<Close>>
            
            field|&nbsp;&nbsp;&nbsp;&nbsp;@TR<<OUTPUT 4>>|IOPORT_output_4_field|hidden|1 
            radio|output4|$FORM_output4|n/a|@TR<<n/a>>
            radio|output4|$FORM_output4|0|@TR<<Open>>
            radio|output4|$FORM_output4|1|@TR<<Close>>

            #############################################

            field|@TR<<>>|IOPORT_add_rule_field|hidden|1 
            submit|add_rule|@TR<<  Add To I/O RULE LIST  >>
            end_form"
        
append forms "$form" "$N"



form="  string|</br>
        string|<div class=\"settings\" id=\"IOPORT_rule_list_field\" style=\"display: none\">
	string|<h3><strong>@TR<<I/O RULE LIST>></strong></h3>
	string|<table  style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.9em;\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" summary=\"@TR<<io Rules Summary>>\">
        string|<tr>
	string|<td>@TR<<Name>></td>
        string|<td>@TR<<Rule Mode>></td>
        string|<td>@TR<<Expiration Time>></td>
	string|<td>@TR<<Input1>></td>
        string|<td>@TR<<Input2>></td>
        string|<td>@TR<<Input3>></td>
        string|<td>@TR<<Input4>></td>
        string|<td>@TR<<>></td>
	string|<td>@TR<<Output1>></td>
        string|<td>@TR<<Output2>></td>
        string|<td>@TR<<Output3>></td>
        string|<td>@TR<<Output4>></td>
        string|</tr>"
append forms "$form" "$N"


#uci_load iorules
#config_get_bool disabled_firewall normal disabled_firewall 0

vcfgcnt=0
uniq_names=""
rm -rf /var/run/ioport_rule_name

for rule in $rule_cfgs; do
        logger "The iorule is $rule"

	#echo "$rule" |grep -q "cfg*****" || echo "$rule" | grep -q "MAC_" || echo "$rule" | grep -q "IPLIST_" || echo "$rule" | grep -q "DEFRule_" || {

        echo "$rule" | grep -q "CUSTOM_" && {
            #logger "FORM_submit=$FORM_submit add_rule_cfg=$add_rule_cfg"
        
	    if [ "$FORM_submit" = "" -o "$add_rule_cfg" = "$rule" ]; then
                    #config_get FORM_rule_define "rule_define" rule_define
                    config_get FORM_rule_mode "$rule" rule_mode
                    config_get FORM_timercount "$rule" timercount
	    	    config_get FORM_input1 "$rule" input1
                    config_get FORM_input2 "$rule" input2
                    config_get FORM_input3 "$rule" input3
                    config_get FORM_input4 "$rule" input4
                    config_get FORM_output1 "$rule" output1
                    config_get FORM_output2 "$rule" output2
                    config_get FORM_output3 "$rule" output3
                    config_get FORM_output4 "$rule" output4
                    #logger "hello1 INPUT:$FORM_rule_mode $FORM_timercount $FORM_input1 $FORM_input2 $FORM_input3 $FORM_input4 OUTPUT:$FORM_output1 $FORM_output2 $FORM_output3 $FORM_output4"
	    else
	    	   
                    eval FORM_rule_mode="\$FORM_rule_mode_$vcfgcnt"
                    eval FORM_timercount="\$FORM_timercount_$vcfgcnt"
                    eval FORM_input1="\$FORM_input1_$vcfgcnt"
                    eval FORM_input2="\$FORM_input2_$vcfgcnt"
                    eval FORM_input3="\$FORM_input3_$vcfgcnt"
                    eval FORM_input4="\$FORM_input4_$vcfgcnt"
                    eval FORM_output1="\$FORM_output1_$vcfgcnt"
                    eval FORM_output2="\$FORM_output2_$vcfgcnt"
                    eval FORM_output3="\$FORM_output3_$vcfgcnt"
                    eval FORM_output4="\$FORM_output4_$vcfgcnt"
                    #logger "hello2 INPUT:$FORM_rule_mode $FORM_timercount $FORM_input1 $FORM_input2 $FORM_input3 $FORM_input4 OUTPUT:$FORM_output1 $FORM_output2 $FORM_output3 $FORM_output4"
            
	    	    equal "$?" 0 && {
                            #uci_set iorules "$rule" rule_define "$FORM_rule_define"
                            uci_set iorules "$rule" rule_mode "$FORM_rule_mode"
                            uci_set iorules "$rule" timercount "$FORM_timercount"
                            uci_set iorules "$rule" input1 "$FORM_input1"
	    	            uci_set iorules "$rule" input2 "$FORM_input2"
                            uci_set iorules "$rule" input3 "$FORM_input3"
                            uci_set iorules "$rule" input4 "$FORM_input4"
                            uci_set iorules "$rule" output1 "$FORM_output1"
                            uci_set iorules "$rule" output2 "$FORM_output2"
                            uci_set iorules "$rule" output3 "$FORM_output3"
                            uci_set iorules "$rule" output4 "$FORM_output4"
                    } || {
                        append  added_error "string|<tr><td colspan=\"2\"><h3 class=\"warning\">$ERROR</h3></td></tr>" "$N"
                    }  
            
	    fi
            
            
#string|<h3 ><strong>@TR<<I/O RULES LIST>></strong></h3>
#string|<table style=\"width: 90%; margin-left: 2.5em; text-align: left; font-size: 0.8em;\" border=\"1\" cellpadding=\"0\" cellspacing=\"0\" summary=\"@TR<<io Rules Summary>>\">            
            
            name=$(echo "$rule" |cut -c 8-)
            append uniq_names "$rule" "$N"
            echo "$uniq_names" > /var/run/ioport_rule_name 
            
            get_tr
	    form="$tr  
                    string|<td>
                    string|<b>$name&nbsp;&nbsp;&nbsp;&nbsp;</b>
                    string|</td>            
                    
                    string|<td>          
                    select|rule_mode_$vcfgcnt|$FORM_rule_mode
                    option|INPUT_ONLY|@TR<<Use Input States Only>>                
                    option|INPUT_TIMER|@TR<<Use Input States With Timer>>
                    option|TIMER_ONLY|@TR<<Use Timer Only>>
                    string|</td>

                    string|<td>
                    string|<input id=\"timercount_$vcfgcnt\" type=\"text\" name=\"timercount_$vcfgcnt\" value=\"$FORM_timercount\" size=\"8\"> 
                    string|</td>            
            
            	    string|<td>
                    string|<input id=\"input1_$vcfgcnt\" type=\"text\" name=\"input1_$vcfgcnt\" value=\"$FORM_input1\"   size=\"6\">           	    
            	    string|</td>

                    string|<td>
            	    string|<input id=\"input2_$vcfgcnt\" type=\"text\" name=\"input2_$vcfgcnt\" value=\"$FORM_input2\"   size=\"6\"> 
            	    string|</td>

                    string|<td>
            	    string|<input id=\"input3_$vcfgcnt\" type=\"text\" name=\"input3_$vcfgcnt\" value=\"$FORM_input3\"   size=\"6\"> 
            	    string|</td>

                    string|<td>
            	    string|<input id=\"input4_$vcfgcnt\" type=\"text\" name=\"input4_$vcfgcnt\" value=\"$FORM_input4\"    size=\"6\"> 
            	    string|</td>

                    string|<td>
                    string|<b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</b>
                    string|</td>

                    string|<td>
            	    string|<input id=\"output1_$vcfgcnt\" type=\"text\" name=\"output1_$vcfgcnt\" value=\"$FORM_output1\"    size=\"6\"> 
            	    string|</td>

                    string|<td>
            	    string|<input id=\"output2_$vcfgcnt\" type=\"text\" name=\"output2_$vcfgcnt\" value=\"$FORM_output2\"    size=\"6\"> 
            	    string|</td>

                    string|<td>
            	    string|<input id=\"output3_$vcfgcnt\" type=\"text\" name=\"output3_$vcfgcnt\" value=\"$FORM_output3\"   size=\"6\"> 
            	    string|</td>

                    string|<td>
            	    string|<input id=\"output4_$vcfgcnt\" type=\"text\" name=\"output4_$vcfgcnt\" value=\"$FORM_output4\"   size=\"6\"> 
            	    string|</td>
            
	    	    string|<td>
	    	    string|&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"$SCRIPT_NAME?remove_vcfg=$rule\">@TR<<Remove>></a>
	    	    string|</td>

	    	    string|</tr>"
	    append forms "$form" "$N"
            let "vcfgcnt+=1"
    }
#    }
done

form="  string|</tr>
        string|</table></div>"

append forms "$form" "$N"




IOPORT_rule_mode_field






######################################################################################
# set JavaScript
javascript_forms="

    v01 = isset('rule_define', 'A') || isset('rule_define', 'B');
    set_visible('IOPORT_name_rule_field', !v01);
    set_visible('IOPORT_rule_mode_field', !v01);
    set_visible('IOPORT_timercount_field', !v01);
    set_visible('IOPORT_input_event_field', !v01);
    set_visible('IOPORT_input_1_field', !v01);
    set_visible('IOPORT_input_2_field', !v01);
    set_visible('IOPORT_input_3_field', !v01);
    set_visible('IOPORT_input_4_field', !v01);
    set_visible('IOPORT_action_to_output_field', !v01);
    set_visible('IOPORT_output_1_field', !v01);
    set_visible('IOPORT_output_2_field', !v01);
    set_visible('IOPORT_output_3_field', !v01);
    set_visible('IOPORT_output_4_field', !v01);
    set_visible('IOPORT_add_rule_field', !v01);
    set_visible('IOPORT_rule_list_field', !v01);
    "
append js "$javascript_forms" "$N"


######################################################################################
header "I\/O" "I\/O Rules" "@TR<<I/O Rules>>" ' onload="modechange()" ' "$SCRIPT_NAME"
######################################################################################

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
$added_error
$forms
EOF


footer ?>
<!--
##WEBIF:name:I/O:300:I/O Rules
-->
