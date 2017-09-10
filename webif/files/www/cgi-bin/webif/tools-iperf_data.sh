#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
tmpfile="/var/run/iperf_run"

iperf_id=`ps|grep "iperf -c" | grep -v "grep"| awk '{print $1}'`
empty "$iperf_id" &&    echo "page_refresh=30" > /var/run/iperf_refresh

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
<meta http-equiv="Content-Type" content="text/html; charset=windows-1252">
<meta http-equiv="refresh" content="$page_refresh" url="tools-iperf_data.sh">  
</head><body>
<p>
EOF
#awk '{printf("%s <br/>",$0);}' $tmpfile
cat $tmpfile
cat <<EOF
</p>
</body>
</html>
EOF
?>

