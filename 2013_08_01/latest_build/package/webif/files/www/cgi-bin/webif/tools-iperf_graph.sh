#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
tmpfile="/var/run/iperf_run"
max_result=0
[ -f "/var/run/iperf_refresh" ] && {
. /var/run/iperf_refresh
} || {
page_refresh=1
}

cat <<EOF
Content-Type: text/html; charset=UTF-8
Pragma: no-cache##

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html><head>
<meta http-equiv="Content-Type" content="text/html; charset=windows-1252" url="tools-iperf_data.sh">
</head><body>
<p>
EOF
?>
        <object type="image/svg+xml" data="/cgi-bin/webif/graph_iperf_svg.sh?if=iperf"
		width="95%" height="270">
		<param name="src" value="/cgi-bin/webif/graph_iperf_svg.sh?if=<? echo -n iperf ?>" />
		<a href="http://www.adobe.com/svg/viewer/install/main.html">If the graph is not fuctioning download the viewer here.</a>
	</object>

<?
cat <<EOF
</p>
</body>
</html>
EOF
?>

