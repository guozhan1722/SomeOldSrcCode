#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"

if  [ "$FORM_view" = "dmap" ];  then
/bin/modbusds dmap
#####################################################################
header "Tools" "Modbus" "@TR<<Modbus Data Map>>" "$SCRIPT_NAME"
#####################################################################
#<table style="width: 65%; margin-left: 0em; text-align: left; font-size: 1em;" border="0" cellpadding="3" cellspacing="2" summary="Settings"><tr><td width=20%>Bits Address</td><td width=30%>Hex Format</td><td>Definition</td></tr>
#</table>
cat <<EOF
<div class="settings"> 
EOF
cat /var/run/modbusdmap <<EOF
EOF
cat <<EOF
</div>
EOF
else

if empty "$FORM_submit"; then
    conf="modbus_conf"
else
    if empty "$FORM_Modbus_S_Enable_md"; then
        conf="modbus_conf"
    else

 	FORM_Modbus_S_Enable="$FORM_Modbus_S_Enable_md"
 	FORM_Modbus_S_TCP_en="$FORM_Modbus_S_TCP_en_md"
 	FORM_Modbus_S_TCP_port="$FORM_Modbus_S_TCP_port_md"
 	FORM_Modbus_S_TCP_timeout="$FORM_Modbus_S_TCP_timeout_md"
 	FORM_Modbus_S_TCP_id="$FORM_Modbus_S_TCP_id_md"
 	FORM_Modbus_S_TCP_C="$FORM_Modbus_S_TCP_C_md"
 	FORM_Modbus_S_TCP_I="$FORM_Modbus_S_TCP_I_md"
 	FORM_Modbus_S_TCP_R="$FORM_Modbus_S_TCP_R_md"
 	FORM_Modbus_S_TCP_IPF_EN="$FORM_Modbus_S_TCP_IPF_EN_md"
 	FORM_Modbus_S_TCP_IPF1="$FORM_Modbus_S_TCP_IPF1_md"
 	FORM_Modbus_S_TCP_IPF2="$FORM_Modbus_S_TCP_IPF2_md"
 	FORM_Modbus_S_TCP_IPF3="$FORM_Modbus_S_TCP_IPF3_md"
 	FORM_Modbus_S_TCP_IPF4="$FORM_Modbus_S_TCP_IPF4_md"
 	FORM_Modbus_S_COM_en="$FORM_Modbus_S_COM_en_md"
 	FORM_Modbus_S_COM_rate="$FORM_Modbus_S_COM_rate_md"
 	FORM_Modbus_S_COM_format="$FORM_Modbus_S_COM_format_md"
 	FORM_Modbus_S_COM_chmode="$FORM_Modbus_S_COM_chmode_md"
 	FORM_Modbus_S_COM_timeout="$FORM_Modbus_S_COM_timeout_md"
 	FORM_Modbus_S_COM_id="$FORM_Modbus_S_COM_id_md"
 	FORM_Modbus_S_COM_C="$FORM_Modbus_S_COM_C_md"
 	FORM_Modbus_S_COM_I="$FORM_Modbus_S_COM_I_md"
 	FORM_Modbus_S_COM_R="$FORM_Modbus_S_COM_R_md"

validate <<EOF
int|FORM_Modbus_S_TCP_C_md|@TR<<TCP Coil Offset>>||$FORM_Modbus_S_TCP_C
int|FORM_Modbus_S_TCP_I_md|@TR<<TCP Input Offset>>||$FORM_Modbus_S_TCP_I
int|FORM_Modbus_S_TCP_R_md|@TR<<TCP Register Offset>>||$FORM_Modbus_S_TCP_R
int|FORM_Modbus_S_TCP_id_md|@TR<<TCP Slave ID>>||$FORM_Modbus_S_TCP_id
int|FORM_Modbus_S_TCP_port_md|@TR<<TCP Port>>||$FORM_Modbus_S_TCP_port
int|FORM_Modbus_S_TCP_timeout_md|@TR<<Modbus TCP timeout>>||$FORM_Modbus_S_TCP_timeout
int|FORM_Modbus_S_COM_C_md|@TR<<COM Coil Offset>>||$FORM_Modbus_S_TCOM_C
int|FORM_Modbus_S_COM_I_md|@TR<<COM Input Offset>>||$FORM_Modbus_S_COM_I
int|FORM_Modbus_S_COM_R_md|@TR<<COM Register Offset>>||$FORM_Modbus_S_COM_R
int|FORM_Modbus_S_COM_id_md|@TR<<COM Slave ID>>||$FORM_Modbus_S_COM_id
int|FORM_Modbus_S_COM_port_md|@TR<<COM Port>>||$FORM_Modbus_S_COM_port
int|FORM_Modbus_S_COM_timeout_md|@TR<<Modbus COM timeout>>||$FORM_Modbus_S_COM_timeout
ip|FORM_Modbus_S_TCP_IPF1_md|@TR<<TCP Accept IP1>>||$FORM_Modbus_S_TCP_IPF1
ip|FORM_Modbus_S_TCP_IPF2_md|@TR<<TCP Accept IP2>>||$FORM_Modbus_S_TCP_IPF2
ip|FORM_Modbus_S_TCP_IPF3_md|@TR<<TCP Accept IP3>>||$FORM_Modbus_S_TCP_IPF3
ip|FORM_Modbus_S_TCP_IPF4_md|@TR<<TCP Accept IP4>>||$FORM_Modbus_S_TCP_IPF4
EOF

       [ "$?" = "0" ] && {
            uci_set "modbusd" "modbus_conf" "Modbus_S_Enable" "$FORM_Modbus_S_Enable"
            uci_set "modbusd" "modbus_conf" "Modbus_S_TCP_en" "$FORM_Modbus_S_TCP_en"
            uci_set "modbusd" "modbus_conf" "Modbus_S_TCP_port" "$FORM_Modbus_S_TCP_port"
            uci_set "modbusd" "modbus_conf" "Modbus_S_TCP_timeout" "$FORM_Modbus_S_TCP_timeout"
            uci_set "modbusd" "modbus_conf" "Modbus_S_TCP_id" "$FORM_Modbus_S_TCP_id"
            uci_set "modbusd" "modbus_conf" "Modbus_S_TCP_C" "$FORM_Modbus_S_TCP_C"
            uci_set "modbusd" "modbus_conf" "Modbus_S_TCP_I" "$FORM_Modbus_S_TCP_I"
            uci_set "modbusd" "modbus_conf" "Modbus_S_TCP_R" "$FORM_Modbus_S_TCP_R"
            uci_set "modbusd" "modbus_conf" "Modbus_S_TCP_IPF_EN" "$FORM_Modbus_S_TCP_IPF_EN"
            uci_set "modbusd" "modbus_conf" "Modbus_S_TCP_IPF1" "$FORM_Modbus_S_TCP_IPF1"
            uci_set "modbusd" "modbus_conf" "Modbus_S_TCP_IPF2" "$FORM_Modbus_S_TCP_IPF2"
            uci_set "modbusd" "modbus_conf" "Modbus_S_TCP_IPF3" "$FORM_Modbus_S_TCP_IPF3"
            uci_set "modbusd" "modbus_conf" "Modbus_S_TCP_IPF4" "$FORM_Modbus_S_TCP_IPF4"
            uci_set "modbusd" "modbus_conf" "Modbus_S_COM_en" "$FORM_Modbus_S_COM_en"
            uci_set "modbusd" "modbus_conf" "Modbus_S_COM_rate" "$FORM_Modbus_S_COM_rate"
            uci_set "modbusd" "modbus_conf" "Modbus_S_COM_format" "$FORM_Modbus_S_COM_format"
            uci_set "modbusd" "modbus_conf" "Modbus_S_COM_chmode" "$FORM_Modbus_S_COM_chmode"
            uci_set "modbusd" "modbus_conf" "Modbus_S_COM_timeout" "$FORM_Modbus_S_COM_timeout"
            uci_set "modbusd" "modbus_conf" "Modbus_S_COM_id" "$FORM_Modbus_S_COM_id"
            uci_set "modbusd" "modbus_conf" "Modbus_S_COM_C" "$FORM_Modbus_S_COM_C"
            uci_set "modbusd" "modbus_conf" "Modbus_S_COM_I" "$FORM_Modbus_S_COM_I"
            uci_set "modbusd" "modbus_conf" "Modbus_S_COM_R" "$FORM_Modbus_S_COM_R"
         }
   fi
fi


uci_load "modbusd"
       config_get FORM_Modbus_S_Enable "modbus_conf" Modbus_S_Enable
       config_get FORM_Modbus_S_TCP_en "modbus_conf" Modbus_S_TCP_en
       config_get FORM_Modbus_S_TCP_port "modbus_conf" Modbus_S_TCP_port
       config_get FORM_Modbus_S_TCP_timeout "modbus_conf" Modbus_S_TCP_timeout
       config_get FORM_Modbus_S_TCP_id "modbus_conf" Modbus_S_TCP_id
       config_get FORM_Modbus_S_TCP_C "modbus_conf" Modbus_S_TCP_C
       config_get FORM_Modbus_S_TCP_I "modbus_conf" Modbus_S_TCP_I
       config_get FORM_Modbus_S_TCP_R "modbus_conf" Modbus_S_TCP_R
       config_get FORM_Modbus_S_TCP_IPF_EN "modbus_conf" Modbus_S_TCP_IPF_EN
       config_get FORM_Modbus_S_TCP_IPF1 "modbus_conf" Modbus_S_TCP_IPF1
       config_get FORM_Modbus_S_TCP_IPF2 "modbus_conf" Modbus_S_TCP_IPF2
       config_get FORM_Modbus_S_TCP_IPF3 "modbus_conf" Modbus_S_TCP_IPF3
       config_get FORM_Modbus_S_TCP_IPF4 "modbus_conf" Modbus_S_TCP_IPF4
       config_get FORM_Modbus_S_COM_en "modbus_conf" Modbus_S_COM_en
       config_get FORM_Modbus_S_COM_rate "modbus_conf" Modbus_S_COM_rate
       config_get FORM_Modbus_S_COM_format "modbus_conf" Modbus_S_COM_format
       config_get FORM_Modbus_S_COM_chmode "modbus_conf" Modbus_S_COM_chmode
       config_get FORM_Modbus_S_COM_timeout "modbus_conf" Modbus_S_COM_timeout
       config_get FORM_Modbus_S_COM_id "modbus_conf" Modbus_S_COM_id
       config_get FORM_Modbus_S_COM_C "modbus_conf" Modbus_S_COM_C
       config_get FORM_Modbus_S_COM_I "modbus_conf" Modbus_S_COM_I
       config_get FORM_Modbus_S_COM_R "modbus_conf" Modbus_S_COM_R

uci_load "comport2"
     config_get FORM_COM2_Port_Status "com2" COM2_Port_Status
     str_configcom_md=" "
    if [ "$FORM_COM2_Port_Status" = "B" ]; then
            str_configcom_md=":Com used"
    fi

config_options="
start_form| @TR<<Modbus Slave Device Config:>>
field|@TR<< <strong>Status</strong> >>|Modbus_S_Enable_md_field|1
select|Modbus_S_Enable_md|$FORM_Modbus_S_Enable
option|A|@TR<<Disable Service>>
option|B|@TR<<Enable Service>>

field|@TR<< <strong>TCP Mode Status</strong> >>|Modbus_S_TCP_en_md_field|hidden
select|Modbus_S_TCP_en_md|$FORM_Modbus_S_TCP_en
option|A|@TR<<Disable TCP Connection Service>>
option|B|@TR<<Enable TCP Connection Service>>

field|@TR<<&nbsp;&nbsp;Port>>|Modbus_S_TCP_port_md_field|hidden
text|Modbus_S_TCP_port_md|$FORM_Modbus_S_TCP_port|@TR<<[1 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Active Timeout(s)>>|Modbus_S_TCP_timeout_md_field|hidden
text|Modbus_S_TCP_timeout_md|$FORM_Modbus_S_TCP_timeout|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Slave ID>>|Modbus_S_TCP_id_md_field|hidden
text|Modbus_S_TCP_id_md|$FORM_Modbus_S_TCP_id|@TR<<[1 ~ 255]>>

field|@TR<<&nbsp;&nbsp;Coils Address Offset>>|Modbus_S_TCP_C_md_field|hidden
text|Modbus_S_TCP_C_md|$FORM_Modbus_S_TCP_C|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Input Address Offset>>|Modbus_S_TCP_I_md_field|hidden
text|Modbus_S_TCP_I_md|$FORM_Modbus_S_TCP_I|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Register Address Offset>>|Modbus_S_TCP_R_md_field|hidden
text|Modbus_S_TCP_R_md|$FORM_Modbus_S_TCP_R|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Master IP Filter Set>>|Modbus_S_TCP_IPF_EN_md_field|hidden
select|Modbus_S_TCP_IPF_EN_md|$FORM_Modbus_S_TCP_IPF_EN
option|A|@TR<<Disable IP Filter>>
option|B|@TR<<Enable IP Filter>>

field|@TR<<&nbsp;&nbsp;Accept Master IP1>>|Modbus_S_TCP_IPF1_md_field|hidden
text|Modbus_S_TCP_IPF1_md|$FORM_Modbus_S_TCP_IPF1|@TR<<[0.0.0.0]>>

field|@TR<<&nbsp;&nbsp;Accept Master IP1>>|Modbus_S_TCP_IPF2_md_field|hidden
text|Modbus_S_TCP_IPF2_md|$FORM_Modbus_S_TCP_IPF2|@TR<<[0.0.0.0]>>

field|@TR<<&nbsp;&nbsp;Accept Master IP1>>|Modbus_S_TCP_IPF3_md_field|hidden
text|Modbus_S_TCP_IPF3_md|$FORM_Modbus_S_TCP_IPF3|@TR<<[0.0.0.0]>>

field|@TR<<&nbsp;&nbsp;Accept Master IP1>>|Modbus_S_TCP_IPF4_md_field|hidden
text|Modbus_S_TCP_IPF4_md|$FORM_Modbus_S_TCP_IPF4|@TR<<[0.0.0.0]>>

field|@TR<< <strong>COM Mode Status</strong> >>|Modbus_S_COM_en_md_field|hidden
select|Modbus_S_COM_en_md|$FORM_Modbus_S_COM_en
option|A|@TR<<Disable COM Connection Service$str_configcom_md>>
"

if [ "$FORM_COM2_Port_Status" = "A" ]; then
config_options=$config_options"
option|B|@TR<<Enable COM ASCII Mode>>
option|C|@TR<<Enable COM RTU Mode>>
"
fi

config_options=$config_options"
field|@TR<<&nbsp;&nbsp;Data Mode>>|Modbus_S_COM_chmode_md_field|hidden
select|Modbus_S_COM_chmode_md|$FORM_Modbus_S_COM_chmode
option|A|@TR<<RS232>>
option|B|@TR<<RS485>>
option|C|@TR<<RS422>>

field|@TR<<&nbsp;&nbsp;Baud Rate>>|Modbus_S_COM_rate_md_field|hidden
select|Modbus_S_COM_rate_md|$FORM_Modbus_S_COM_rate
option|A|@TR<<300>>
option|B|@TR<<600>>
option|C|@TR<<1200>>
option|D|@TR<<2400>>
option|E|@TR<<3600>>
option|F|@TR<<4800>>
option|G|@TR<<7200>>
option|H|@TR<<9600>>
option|I|@TR<<14400>>
option|J|@TR<<19200>>
option|K|@TR<<28800>>
option|L|@TR<<38400>>
option|M|@TR<<57600>>
option|N|@TR<<115200>>
option|O|@TR<<230400>>
option|P|@TR<<460800>>
option|Q|@TR<<921600>>

field|@TR<<&nbsp;&nbsp;Data Format>>|Modbus_S_COM_format_md_field|hidden
select|Modbus_S_COM_format_md|$FORM_Modbus_S_COM_format
option|A|@TR<<8N1>>
option|B|@TR<<8N2>>
option|C|@TR<<8E1>>
option|D|@TR<<8O1>>
option|E|@TR<<7N1>>
option|F|@TR<<7N2>>
option|G|@TR<<7E1>>
option|H|@TR<<7O1>>
option|I|@TR<<7E2>>
option|J|@TR<<7O2>>

field|@TR<<&nbsp;&nbsp;Character Timeout(s)>>|Modbus_S_COM_timeout_md_field|hidden
text|Modbus_S_COM_timeout_md|$FORM_Modbus_S_COM_timeout|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Slave ID>>|Modbus_S_COM_id_md_field|hidden
text|Modbus_S_COM_id_md|$FORM_Modbus_S_COM_id|@TR<<[1 ~ 255]>>

field|@TR<<&nbsp;&nbsp;Coils Address Offset>>|Modbus_S_COM_C_md_field|hidden
text|Modbus_S_COM_C_md|$FORM_Modbus_S_COM_C|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Input Address Offset>>|Modbus_S_COM_I_md_field|hidden
text|Modbus_S_COM_I_md|$FORM_Modbus_S_COM_I|@TR<<[0 ~ 65535]>>

field|@TR<<&nbsp;&nbsp;Register Address Offset>>|Modbus_S_COM_R_md_field|hidden
text|Modbus_S_COM_R_md|$FORM_Modbus_S_COM_R|@TR<<[0 ~ 65535]>>

end_form

start_form
field|@TR<< >>
string| <a href=tools-modbus.sh?view=dmap>View Data Map</a>
end_form

"

append forms "$config_options" "$N"


######################################################################################
# set JavaScript
javascript_forms="
    v_en = isset('Modbus_S_Enable_md','B');
    v_tcp = isset('Modbus_S_TCP_en_md','B');
    v_tcp_ip = isset('Modbus_S_TCP_IPF_EN_md','B');
    v_com = isset('Modbus_S_COM_en_md','B');
    if(v_com==0)v_com = isset('Modbus_S_COM_en_md','C');

    set_visible('Modbus_S_TCP_en_md_field', v_en);
    set_visible('Modbus_S_COM_en_md_field', v_en);
    if((v_tcp)&&(v_en))v_tcp=1;
    else v_tcp=0;
    set_visible('Modbus_S_TCP_port_md_field', v_tcp);
    set_visible('Modbus_S_TCP_timeout_md_field', v_tcp);
    set_visible('Modbus_S_TCP_id_md_field', v_tcp);
    set_visible('Modbus_S_TCP_C_md_field', v_tcp);
    set_visible('Modbus_S_TCP_I_md_field', v_tcp);
    set_visible('Modbus_S_TCP_R_md_field', v_tcp);
    set_visible('Modbus_S_TCP_IPF_EN_md_field', v_tcp);
    if((v_tcp)&&(v_tcp_ip))v_tcp_ip=1;
    else v_tcp_ip=0;
    set_visible('Modbus_S_TCP_IPF1_md_field', v_tcp_ip);
    set_visible('Modbus_S_TCP_IPF2_md_field', v_tcp_ip);
    set_visible('Modbus_S_TCP_IPF3_md_field', v_tcp_ip);
    set_visible('Modbus_S_TCP_IPF4_md_field', v_tcp_ip);
    if((v_com)&&(v_en))v_com=1;
    else v_com=0;
    set_visible('Modbus_S_COM_rate_md_field', v_com);
    set_visible('Modbus_S_COM_format_md_field', v_com);
    set_visible('Modbus_S_COM_chmode_md_field', v_com);
    set_visible('Modbus_S_COM_timeout_md_field', v_com);
    set_visible('Modbus_S_COM_id_md_field', v_com);
    set_visible('Modbus_S_COM_C_md_field', v_com);
    set_visible('Modbus_S_COM_I_md_field', v_com);
    set_visible('Modbus_S_COM_R_md_field', v_com);

"

append js "$javascript_forms" "$N"


#####################################################################
header "Tools" "Modbus" "@TR<<Modbus>>"  ' onload="modechange()" ' "$SCRIPT_NAME"
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


fi

footer ?>
<!--
##WEBIF:name:Tools:400:Modbus
-->
