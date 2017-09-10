#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
tmpfile="/var/run/trace_result_tmp"

trace_pid=`ps|grep "traceroute" | grep -v "grep"| awk '{print $1}'`
empty "$trace_pid" &&  {
    page_refresh=30
} || {
    [ -f "/var/run/trace_refresh" ] && {
        . /var/run/trace_refresh
    } || {
        page_refresh=1
    }
}

cat <<EOF
Content-Type: text/html; charset=UTF-8
Pragma: no-cache##

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html><head>
<meta http-equiv="Content-Type" content="text/html; charset=windows-1252">
<meta http-equiv="refresh" content="$page_refresh" url="tools-trace_data.sh">  
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

