#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
###################################################################
# About page
#
# Description:
#        Shows the many contributors.
#
# Author(s) [in order of work date]:
#       Original webif authors.
#        Jeremy Collake <jeremy.collake@gmail.com>
#        Dmytro Dykhman <dmytro@iroot.ca.
#
# Major revisions:
#
# Configuration files referenced:
#   none
#
header "Info" "About" "<img src=\"/images/abt.jpg\" alt=\"@TR<<About>>\" />@TR<<About>>"

this_revision=$(cat "/www/.version")

?>
<script src="/js/scrollbox.js" type="text/javascript"></script>
<h2>@TR<<VIP 2.0>></h2>

<div id="outerscrollBox"  onMouseOver="copyspeed=pausespeed" onMouseOut="copyspeed=marqueespeed">

<div id="scrollBox">

<h1>@TR<<Vip 2.0 Features>></h1>
<h2>@TR<<Microhard Systems Inc.>></h2>
<ul>
	<li>Up tp 54Mbps data rate</li>
	<li>Adaptive modulation</li>
	<li>Wan and Lan dial ports</li>
	<li>AP may be configurate as router</li>
	<li>WDS station bridge</li>
	<li>User-configurable firewall functions</li>
	<li>Comprehensive encryption support</li>
	<li>Authenticator and supplicant</li>
	<li>Quality of Service (QoS)</li>
	<li>Wireless firmware upgrade capable</li>
</ul>


<p>@TR<<This device is running vip 2.0 >>@TR<<>>.</p>

<p>@TR<<Microhard Systems Inc. specializes in the design and manufacture of long range robust wireless data equipment. Our continuous innovation and unparalleled product performance has earned us a trusted name in the wireless industry.>></p>
<p>@TR<<Microhard provides OEM level products, packaged products, and ready to install systems for wireless Data and Voice for industrial, and military, and government clients. We serve a variety of customers ranging from radio integrators to multibillion-dollar defense contractors.>></p>

</div></div> <!-- End scrollBox -->

<? footer ?>
#<!--
###WEBIF:name:Info:950:About
#-->
