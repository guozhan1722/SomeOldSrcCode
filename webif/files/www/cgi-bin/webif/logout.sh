#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
###################################################################
# Logout
#
# Description:
#       Logs user out.
#
# Author(s) [in order of work date]:
#	Jeremy Collake <jeremy.collake@gmail.com>
#
# Major revisions:
#
# NVRAM variables referenced:
#
# Configuration files referenced:
#   none
#
header "Logout" "Logout" "@TR<<logout_window#Are you sure you want to log out ...>>" '' ''

cat <<EOF
<script type='text/javascript' src='/js/logout.js'></script>
<script type='text/javascript'>
<!--  

    var agt=navigator.userAgent.toLowerCase();
    if (agt.indexOf("chrome") != -1) {
        var loc=location.hostname;
        var httploc=location.href;
        document.write('<a href=\"http://admin@')
        document.write(loc);
        document.write('/cgi-bin/webif/logout_refresh.sh?localip=');
        document.write(loc);
        document.write('\">Logout Now</a>');
    }else {
        document.write('<a href="system-info.sh" onclick="clearAuthenticationCache();">Logout Now</a>')
    }

-->            
</script>

EOF

#logout_user
footer ?>
<!--
##WEBIF:name:Logout:1:Logout
-->
