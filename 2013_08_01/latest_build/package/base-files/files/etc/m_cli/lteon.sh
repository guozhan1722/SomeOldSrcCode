setdis=$(uci set lte.connect.disabled=0)
setdis=$(uci commit lte)
exec mpci -gpio set 8 1
