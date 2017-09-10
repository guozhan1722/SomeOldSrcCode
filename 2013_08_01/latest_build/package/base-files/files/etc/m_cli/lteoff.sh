setdis=$(uci set lte.connect.disabled=1)
setdis=$(uci commit lte)

exec mpci -gpio set 8 0
