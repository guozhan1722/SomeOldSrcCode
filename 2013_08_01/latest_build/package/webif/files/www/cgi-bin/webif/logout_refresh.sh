#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

router_ip="$FORM_localip"


header_inject_head=$(cat <<EOF
<meta http-equiv="refresh" content="1;http://$router_ip\" />
EOF
)

header "System" "Logout" "@TR<<logout_window#Are you sure you want to log out ...>>" '' ''
cat <<EOF
<script type='text/javascript' src='/js/logout.js'></script>
<script type='text/javascript'>
<!--  
        document.write('<a href=\"null\">Logout Now</a>');
-->            
</script>

EOF
footer_nosubmit
?>

