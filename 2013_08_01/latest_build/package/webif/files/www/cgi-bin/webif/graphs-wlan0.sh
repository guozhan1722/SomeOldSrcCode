#!/usr/bin/webif-page
<?
#
#credit goes to arantius and GasFed
#
. /usr/lib/webif/webif.sh
. /www/cgi-bin/webif/graphs-subcategories.sh

header "Graphs" "wlan0" "@TR<<Interface wlan0>>"
# This construction supports all recent browsers, degrades correctly, 
# see http://joliclic.free.fr/html/object-tag/en/object-svg.html
?>
<center>
	<object type="image/svg+xml" data="/cgi-bin/webif/graph_if_svg.sh?if=<? echo -n wlan0 ?>"
		width="500" height="250">
		<param name="src" value="/cgi-bin/webif/graph_if_svg.sh?if=<? echo -n wlan0 ?>" />
		<a href="http://www.adobe.com/svg/viewer/install/main.html">@TR<<graphs_svg_download#If the graph is not fuctioning download the viewer here.>></a>
	</object>
</center>
<? footer ?>
<!--
##WEBIF:name:Graphs:2:wlan0
-->
