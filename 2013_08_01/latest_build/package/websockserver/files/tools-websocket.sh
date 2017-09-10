#!/usr/bin/webif-page
<?
. "/usr/lib/webif/webif.sh"

conf="wsr_conf"
submit_ok="0"

if empty "$FORM_submit"; then

    conf="wsr_conf"

else
 	FORM_WebSockSvr_Enable="$FORM_WebSockSvr_Enable_ws"
 	FORM_WebSockSvr_timeout="$FORM_WebSockSvr_timeout_ws"
 	FORM_WebSockSvr_GPS="$FORM_WebSockSvr_GPS_ws"
 	FORM_WebSockSvr_GPSNMEA="$FORM_WebSockSvr_GPSNMEA_ws"
 	FORM_WebSockSvr_COM="$FORM_WebSockSvr_COM_ws"
 	FORM_WebSockSvr_RADIO="$FORM_WebSockSvr_RADIO_ws"
 	FORM_WebSockSvr_port="$FORM_WebSockSvr_port_ws"
 	FORM_WebSockSvr_password="$FORM_WebSockSvr_password_ws"
 	FORM_WebSockSvr_freshinter="$FORM_WebSockSvr_freshinter_ws"

 	FORM_WebSockSvr_GPSNMEA_RMC="$FORM_WebSockSvr_GPSNMEA_RMC_ws"
 	FORM_WebSockSvr_GPSNMEA_GGA="$FORM_WebSockSvr_GPSNMEA_GGA_ws"
 	FORM_WebSockSvr_GPSNMEA_GSA="$FORM_WebSockSvr_GPSNMEA_GSA_ws"
 	FORM_WebSockSvr_GPSNMEA_GSV="$FORM_WebSockSvr_GPSNMEA_GSV_ws"
 	FORM_WebSockSvr_GPSNMEA_VTG="$FORM_WebSockSvr_GPSNMEA_VTG_ws"
 	FORM_WebSockSvr_COM_MODE="$FORM_WebSockSvr_COM_MODE_ws"


validate <<EOF
int|$FORMM_WebSockSvr_timeout_ws|@TR<<Server timeout(minutes)>>||$FORM_WebSockSvr_timeout
int|$FORMM_WebSockSvr_port_ws|@TR<<WebSocket Port>>||$FORM_WebSockSvr_port
int|$FORMM_WebSockSvr_freshinter_ws|@TR<<Data Fresh Interval(seconds)>>||$FORM_WebSockSvr_freshinter
EOF

       [ "$?" = "0" ] && {

            if [ "$FORM_WebSockSvr_GPS$FORM_WebSockSvr_GPSNMEA$FORM_WebSockSvr_COM$FORM_WebSockSvr_RADIO" = "AAAA" ]; then
                FORM_WebSockSvr_Enable="A"
            fi            

            uci_set "websockserver" "$conf" "WebSockSvr_Enable" "$FORM_WebSockSvr_Enable"
            uci_set "websockserver" "$conf" "WebSockSvr_timeout" "$FORM_WebSockSvr_timeout"
            uci_set "websockserver" "$conf" "WebSockSvr_port" "$FORM_WebSockSvr_port"
            uci_set "websockserver" "$conf" "WebSockSvr_GPS" "$FORM_WebSockSvr_GPS"
            uci_set "websockserver" "$conf" "WebSockSvr_GPSNMEA" "$FORM_WebSockSvr_GPSNMEA"
            uci_set "websockserver" "$conf" "WebSockSvr_COM" "$FORM_WebSockSvr_COM"
            uci_set "websockserver" "$conf" "WebSockSvr_RADIO" "$FORM_WebSockSvr_RADIO"
            uci_set "websockserver" "$conf" "WebSockSvr_password" "$FORM_WebSockSvr_password"
            uci_set "websockserver" "$conf" "WebSockSvr_freshinter" "$FORM_WebSockSvr_freshinter"

            uci_set "websockserver" "$conf" "WebSockSvr_GPSNMEA_RMC" "$FORM_WebSockSvr_GPSNMEA_RMC"
            uci_set "websockserver" "$conf" "WebSockSvr_GPSNMEA_GGA" "$FORM_WebSockSvr_GPSNMEA_GGA"
            uci_set "websockserver" "$conf" "WebSockSvr_GPSNMEA_GSA" "$FORM_WebSockSvr_GPSNMEA_GSA"
            uci_set "websockserver" "$conf" "WebSockSvr_GPSNMEA_GSV" "$FORM_WebSockSvr_GPSNMEA_GSV"
            uci_set "websockserver" "$conf" "WebSockSvr_GPSNMEA_VTG" "$FORM_WebSockSvr_GPSNMEA_VTG"
            uci_set "websockserver" "$conf" "WebSockSvr_COM_MODE" "$FORM_WebSockSvr_COM_MODE"

            submit_ok="1"

         }
fi

uci_load websockserver
conf="wsr_conf"

       config_get FORM_WebSockSvr_Enable $conf WebSockSvr_Enable "A"
       config_get FORM_WebSockSvr_timeout $conf WebSockSvr_timeout "60"
       config_get FORM_WebSockSvr_GPS $conf WebSockSvr_GPS "A"
       config_get FORM_WebSockSvr_GPSNMEA $conf WebSockSvr_GPSNMEA "A"
       config_get FORM_WebSockSvr_COM $conf WebSockSvr_COM "A"
       config_get FORM_WebSockSvr_RADIO $conf WebSockSvr_RADIO "A"
       config_get FORM_WebSockSvr_port $conf WebSockSvr_port "7681"
       config_get FORM_WebSockSvr_password $conf WebSockSvr_password ""
       config_get FORM_WebSockSvr_freshinter $conf WebSockSvr_freshinter "10"
        
       config_get FORM_WebSockSvr_GPSNMEA_RMC $conf WebSockSvr_GPSNMEA_RMC "A"
       config_get FORM_WebSockSvr_GPSNMEA_GGA $conf WebSockSvr_GPSNMEA_GGA "A"
       config_get FORM_WebSockSvr_GPSNMEA_GSA $conf WebSockSvr_GPSNMEA_GSA "A"
       config_get FORM_WebSockSvr_GPSNMEA_GSV $conf WebSockSvr_GPSNMEA_GSV "A"
       config_get FORM_WebSockSvr_GPSNMEA_VTG $conf WebSockSvr_GPSNMEA_VTG "A"
       config_get FORM_WebSockSvr_COM_MODE $conf WebSockSvr_COM_MODE "A"

    uci_load "comport2"
     config_get FORM_COM2_Port_Status "com2" "COM2_Port_Status"
     config_get FORM_COM2_IP_Protocol "com2" "COM2_IP_Protocol"

     Com_TCPServer_Status="0"
     if [ "$FORM_COM2_Port_Status" = "B" ]; then
        if [ "$FORM_COM2_IP_Protocol" = "B" ]; then
            Com_TCPServer_Status="1"
        fi
        if [ "$FORM_COM2_IP_Protocol" = "C" ]; then
            Com_TCPServer_Status="1"
        fi
     fi
    if [ "$Com_TCPServer_Status" = "0" ]; then
        FORM_WebSockSvr_COM="A"
    fi


config_options="
start_form| @TR<<Setting>>
field|@TR<< <strong>Status</strong> >>|WebSockSvr_Enable_ws_field|1
select|WebSockSvr_Enable_ws|$FORM_WebSockSvr_Enable
option|A|@TR<<Disable>>
option|B|@TR<<Enable Web Socket Service>>

field|@TR<<&nbsp;&nbsp;Web Socket Port(default:7681)>>|WebSockSvr_port_ws_field|hidden
text|WebSockSvr_port_ws|$FORM_WebSockSvr_port|@TR<<[100-65535]>>


field|@TR<<&nbsp;&nbsp;Data Fresh Interval(seconds)>>|WebSockSvr_freshinter_ws_field|hidden
text|WebSockSvr_freshinter_ws|$FORM_WebSockSvr_freshinter|@TR<<[2-65535]>>

field|@TR<<&nbsp;&nbsp;Connect Password>>|WebSockSvr_password_ws_field|hidden
password|WebSockSvr_password_ws|$FORM_WebSockSvr_password|@TR<<(Blank for Disable)>>

field|@TR<<&nbsp;&nbsp;Max Keep Time(minutes)>>|WebSockSvr_timeout_ws_field|hidden
text|WebSockSvr_timeout_ws|$FORM_WebSockSvr_timeout|@TR<<(0:keep alive)>>

field|@TR<<&nbsp;&nbsp;GPS Coordinate>>|WebSockSvr_GPS_ws_field|hidden
radio|WebSockSvr_GPS_ws|$FORM_WebSockSvr_GPS|A|@TR<<Disable>>
radio|WebSockSvr_GPS_ws|$FORM_WebSockSvr_GPS|B|@TR<<Enable>>

field|@TR<<&nbsp;&nbsp;GPS NMEA Data>>|WebSockSvr_GPSNMEA_ws_field|hidden
radio|WebSockSvr_GPSNMEA_ws|$FORM_WebSockSvr_GPSNMEA|A|@TR<<Disable>>
radio|WebSockSvr_GPSNMEA_ws|$FORM_WebSockSvr_GPSNMEA|B|@TR<<Enable>>

field|@TR<<&nbsp;&nbsp;>>|WebSockSvr_GPSNMEA_OPT_ws_field|hidden
string|Option:
checkbox|WebSockSvr_GPSNMEA_RMC_ws|$FORM_WebSockSvr_GPSNMEA_RMC|B|@TR<<RMC>>
checkbox|WebSockSvr_GPSNMEA_GGA_ws|$FORM_WebSockSvr_GPSNMEA_GGA|B|@TR<<GGA>>
checkbox|WebSockSvr_GPSNMEA_GSA_ws|$FORM_WebSockSvr_GPSNMEA_GSA|B|@TR<<GSA>>
checkbox|WebSockSvr_GPSNMEA_GSV_ws|$FORM_WebSockSvr_GPSNMEA_GSV|B|@TR<<GSV>>
checkbox|WebSockSvr_GPSNMEA_VTG_ws|$FORM_WebSockSvr_GPSNMEA_VTG|B|@TR<<VTG>>

field|@TR<<&nbsp;&nbsp;Carrier Information>>|WebSockSvr_RADIO_ws_field|hidden
radio|WebSockSvr_RADIO_ws|$FORM_WebSockSvr_RADIO|A|@TR<<Disable>>
radio|WebSockSvr_RADIO_ws|$FORM_WebSockSvr_RADIO|B|@TR<<Enable>>

field|@TR<<&nbsp;&nbsp;Comport Data>>|WebSockSvr_COM_ws_field|hidden
"
if [ "$Com_TCPServer_Status" = "1" ]; then
config_options="$config_options
radio|WebSockSvr_COM_ws|$FORM_WebSockSvr_COM|A|@TR<<Disable>>
radio|WebSockSvr_COM_ws|$FORM_WebSockSvr_COM|B|@TR<<Enable>>
"
else
config_options="$config_options
radio|WebSockSvr_COM_ws|$FORM_WebSockSvr_COM|A|@TR<<Disabled>>
string|(Please enable comport tcp server.)
"
fi

config_options="$config_options

field|@TR<<&nbsp;&nbsp;>>|WebSockSvr_COM_MODE_ws_field|hidden
string|Trans Mode:
radio|WebSockSvr_COM_MODE_ws|$FORM_WebSockSvr_COM_MODE|A|@TR<<Raw/Text>>
radio|WebSockSvr_COM_MODE_ws|$FORM_WebSockSvr_COM_MODE|B|@TR<<Hex/ASCII>>

end_form
"

append forms "$config_options" "$N"


######################################################################################
javascript_forms="
    v_en = isset('WebSockSvr_Enable_ws','B');

    set_visible('WebSockSvr_timeout_ws_field', v_en);
    set_visible('WebSockSvr_port_ws_field', v_en);
    set_visible('WebSockSvr_freshinter_ws_field', v_en);
    set_visible('WebSockSvr_password_ws_field', v_en);
    set_visible('WebSockSvr_GPS_ws_field', v_en);
    set_visible('WebSockSvr_GPSNMEA_ws_field', v_en);
    set_visible('WebSockSvr_RADIO_ws_field', v_en);
    set_visible('WebSockSvr_COM_ws_field', v_en);

    v_nmea=0;
    if(v_en)
    {
        if(document.getElementsByName('WebSockSvr_GPSNMEA_ws')[1].checked)v_nmea=1;
    }
    set_visible('WebSockSvr_GPSNMEA_OPT_ws_field', v_nmea);

    v_com=0;
    if(v_en)
    {
        if(document.getElementsByName('WebSockSvr_COM_ws')[1].checked)v_com=1;
    }
    set_visible('WebSockSvr_COM_MODE_ws_field', v_com);


"

append js "$javascript_forms" "$N"



#####################################################################
header "Tools" "Websocket" "@TR<<Web Socket Service>>" ' onload="modechange()" ' "$SCRIPT_NAME"
#####################################################################

if [ "$submit_ok" = "1" ]; then
cat <<EOF
    <meta http-equiv='refresh' content='5'>
EOF
fi

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

<div class="settings">
<h3><strong>Online Connected Data</strong></h3>
<div class="settings-content">
<table width="900" summary="Settings">
<tr><td width="100" align="left">Browser Type:</td>
<td align="left" colspan=2 width="800"><div id="brow">Unknown</div></td></tr>
EOF

if [ "$FORM_WebSockSvr_Enable" = "B" ]; then

if [ "$FORM_WebSockSvr_GPS" = "B" ]; then
cat <<EOF
<tr><td width="100" align="left">GPS Data:</td>
<td id="gps_statustd" align="left" width="80" style="color: rgb(10, 10, 250); "><div id="gps_status">WAITING</div></td>
<td align="left" width="720"><div id="gpsdata">N/A</div></td></tr>
EOF
fi

if [ "$FORM_WebSockSvr_GPSNMEA" = "B" ]; then
cat <<EOF
<tr><td width="100" align="left" valign="top">GPS NMEA:</td>
<td id="gpsnmea_statustd" align="left" valign="top" width="60" style="color: rgb(10, 10, 250); "><div id="gpsnmea_status">WAITING</div></td>
<td align="left" valign="top" width="740"><div id="gpsnmeadata">N/A</div></td></tr>
EOF
fi

if [ "$FORM_WebSockSvr_RADIO" = "B" ]; then
cat <<EOF
<tr><td width="100" align="left" valign="top">Carrier Info:</td>
<td id="radioinfo_statustd" align="left" valign="top" width="60" style="color: rgb(10, 10, 250); "><div id="radioinfo_status">WAITING</div></td>
<td align="left" valign="top" width="740"><div id="radioinfodata">N/A</div></td></tr>
EOF
fi

if [ "$FORM_WebSockSvr_COM" = "B" ]; then
cat <<EOF
<tr><td width="100" align="left" valign="top">Comport Data:</td>
<td id="comport_statustd" align="left" valign="top" width="60" style="color: rgb(10, 10, 250); "><div id="comport_status">WAITING</div></td>
<td align="left" valign="top" width="740"><div id="comportdata">N/A</div></td></tr>
EOF
fi

fi

cat <<EOF
</table>
</div>
<script>

/* BrowserDetect came from http://www.quirksmode.org/js/detect.html */

var BrowserDetect = {
	init: function () {
		this.browser = this.searchString(this.dataBrowser) || "An unknown browser";
		this.version = this.searchVersion(navigator.userAgent)
			|| this.searchVersion(navigator.appVersion)
			|| "an unknown version";
		this.OS = this.searchString(this.dataOS) || "an unknown OS";
	},
	searchString: function (data) {
		for (var i=0;i<data.length;i++)	{
			var dataString = data[i].string;
			var dataProp = data[i].prop;
			this.versionSearchString = data[i].versionSearch || data[i].identity;
			if (dataString) {
				if (dataString.indexOf(data[i].subString) != -1)
					return data[i].identity;
			}
			else if (dataProp)
				return data[i].identity;
		}
	},
	searchVersion: function (dataString) {
		var index = dataString.indexOf(this.versionSearchString);
		if (index == -1) return;
		return parseFloat(dataString.substring(index+this.versionSearchString.length+1));
	},
	dataBrowser: [
		{
			string: navigator.userAgent,
			subString: "Chrome",
			identity: "Chrome"
		},
		{ 	string: navigator.userAgent,
			subString: "OmniWeb",
			versionSearch: "OmniWeb/",
			identity: "OmniWeb"
		},
		{
			string: navigator.vendor,
			subString: "Apple",
			identity: "Safari",
			versionSearch: "Version"
		},
		{
			prop: window.opera,
			identity: "Opera",
			versionSearch: "Version"
		},
		{
			string: navigator.vendor,
			subString: "iCab",
			identity: "iCab"
		},
		{
			string: navigator.vendor,
			subString: "KDE",
			identity: "Konqueror"
		},
		{
			string: navigator.userAgent,
			subString: "Firefox",
			identity: "Firefox"
		},
		{
			string: navigator.vendor,
			subString: "Camino",
			identity: "Camino"
		},
		{		// for newer Netscapes (6+)
			string: navigator.userAgent,
			subString: "Netscape",
			identity: "Netscape"
		},
		{
			string: navigator.userAgent,
			subString: "MSIE",
			identity: "Explorer",
			versionSearch: "MSIE"
		},
		{
			string: navigator.userAgent,
			subString: "Gecko",
			identity: "Mozilla",
			versionSearch: "rv"
		},
		{ 		// for older Netscapes (4-)
			string: navigator.userAgent,
			subString: "Mozilla",
			identity: "Netscape",
			versionSearch: "Mozilla"
		}
	],
	dataOS : [
		{
			string: navigator.platform,
			subString: "Win",
			identity: "Windows"
		},
		{
			string: navigator.platform,
			subString: "Mac",
			identity: "Mac"
		},
		{
			   string: navigator.userAgent,
			   subString: "iPhone",
			   identity: "iPhone/iPod"
	    },
		{
			string: navigator.platform,
			subString: "Linux",
			identity: "Linux"
		}
	]

};
BrowserDetect.init();

document.getElementById("brow").textContent = " " + BrowserDetect.browser + " "
	+ BrowserDetect.version +" " + BrowserDetect.OS +" ";

	var pos = 0;

function get_appropriate_ws_url()
{
	var pcol;
	var u = document.URL;

	/*
	 * We open the websocket encrypted if this page came on an
	 * https:// url itself, otherwise unencrypted
	 */

	if (u.substring(0, 5) == "https") {
		pcol = "wss://";
		u = u.substr(8);
	} else {
		pcol = "ws://";
		if (u.substring(0, 4) == "http")
			u = u.substr(7);
	}

	u = u.split('/');

	return pcol + u[0] + ":" + "$FORM_WebSockSvr_port" ;
}
EOF

if [ "$FORM_WebSockSvr_Enable" = "B" ]; then

if [ "$FORM_WebSockSvr_GPS" = "B" ]; then
cat <<EOF
        document.getElementById("gpsdata").textContent = get_appropriate_ws_url();

	var socket_gps;
	if (BrowserDetect.browser == "Firefox" && BrowserDetect.version < 12) {
		socket_gps = new MozWebSocket(get_appropriate_ws_url(),
				   "mh-gps-protocol");
	} else {
		socket_gps = new WebSocket(get_appropriate_ws_url(),
				   "mh-gps-protocol");
	}
	try {
		socket_gps.onopen = function() {
			document.getElementById("gps_statustd").style.color = "#1010ff";
			document.getElementById("gps_status").textContent = "OPENED";
			socket_gps.send("$FORM_WebSockSvr_password");
		} 
		socket_gps.onmessage =function got_packet(msg) {
			document.getElementById("gpsdata").textContent = msg.data + "\n";
		} 
		socket_gps.onclose = function(){
			document.getElementById("gps_statustd").style.color = "#ff1010";
			document.getElementById("gps_status").textContent = "CLOSED";
		}
	} catch(exception) {
		alert('<p>Error' + exception);  
	}
EOF
fi

if [ "$FORM_WebSockSvr_GPSNMEA" = "B" ]; then
cat <<EOF

        document.getElementById("gpsnmeadata").textContent = get_appropriate_ws_url();
	var socket_gpsnmea;
	if (BrowserDetect.browser == "Firefox" && BrowserDetect.version < 12) {
		socket_gpsnmea = new MozWebSocket(get_appropriate_ws_url(),
				   "mh-gpsnmea-protocol");
	} else {
		socket_gpsnmea = new WebSocket(get_appropriate_ws_url(),
				   "mh-gpsnmea-protocol");
	}
	try {
		socket_gpsnmea.onopen = function() {
			document.getElementById("gpsnmea_statustd").style.color = "#1010ff";
			document.getElementById("gpsnmea_status").textContent = "OPENED";
			socket_gpsnmea.send("$FORM_WebSockSvr_password");
		} 
		socket_gpsnmea.onmessage =function got_packet(msg) {
			document.getElementById("gpsnmeadata").textContent = msg.data + "\n";
		} 
		socket_gpsnmea.onclose = function(){
			document.getElementById("gpsnmea_statustd").style.color = "#ff1010";
			document.getElementById("gpsnmea_status").textContent = "CLOSED";
		}
	} catch(exception) {
		alert('<p>Error' + exception);  
	}
EOF
fi

if [ "$FORM_WebSockSvr_RADIO" = "B" ]; then
cat <<EOF

        document.getElementById("radioinfodata").textContent = get_appropriate_ws_url();
	var socket_radioinfo;
	if (BrowserDetect.browser == "Firefox" && BrowserDetect.version < 12) {
		socket_radioinfo = new MozWebSocket(get_appropriate_ws_url(),
				   "mh-radioinfo-protocol");
	} else {
		socket_radioinfo = new WebSocket(get_appropriate_ws_url(),
				   "mh-radioinfo-protocol");
	}
	try {
		socket_radioinfo.onopen = function() {
			document.getElementById("radioinfo_statustd").style.color = "#1010ff";
			document.getElementById("radioinfo_status").textContent = "OPENED";
			socket_radioinfo.send("$FORM_WebSockSvr_password");
		} 
		socket_radioinfo.onmessage =function got_packet(msg) {
			document.getElementById("radioinfodata").textContent = msg.data + "\n";
		} 
		socket_radioinfo.onclose = function(){
			document.getElementById("radioinfo_statustd").style.color = "#ff1010";
			document.getElementById("radioinfo_status").textContent = "CLOSED";
		}
	} catch(exception) {
		alert('<p>Error' + exception);  
	}
EOF
fi

if [ "$FORM_WebSockSvr_COM" = "B" ]; then
cat <<EOF
        document.getElementById("comportdata").textContent = get_appropriate_ws_url();
	var socket_comport;
        var com_msgdata = new String(" "); 
        var n=0;
	if (BrowserDetect.browser == "Firefox" && BrowserDetect.version < 12) {
		socket_comport = new MozWebSocket(get_appropriate_ws_url(),
				   "mh-comport-protocol");
	} else {
		socket_comport = new WebSocket(get_appropriate_ws_url(),
				   "mh-comport-protocol");
	}
	try {
		socket_comport.onopen = function() {
			document.getElementById("comport_statustd").style.color = "#1010ff";
			document.getElementById("comport_status").textContent = "OPENED";
			socket_comport.send("$FORM_WebSockSvr_password");
		} 
		socket_comport.onmessage =function got_packet(msg) {
                        com_msgdata=com_msgdata + msg.data;
			document.getElementById("comportdata").textContent = com_msgdata + "\n";
                        if(com_msgdata.length-n>90)
                        {
                          com_msgdata=com_msgdata + "\n";
                          n=com_msgdata.length;
                        }
                        if(com_msgdata.length>1024)
                        {
                            com_msgdata="";
                            n=0;
                        }
		} 
		socket_comport.onclose = function(){
			document.getElementById("comport_statustd").style.color = "#ff1010";
			document.getElementById("comport_status").textContent = "CLOSED";
		}
	} catch(exception) {
		alert('<p>Error' + exception);  
	}
EOF
fi

fi

cat <<EOF
</script>
<div class="clearfix"></div></div>
EOF


display_form <<EOF
onchange|modechange
$validate_error
$forms
EOF



footer ?>
<!--
##WEBIF:name:Tools:800:Websocket 
-->
