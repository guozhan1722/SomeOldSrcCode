setdis=$(uci set wireless.radio0.disabled=0)
setdis=$(uci commit wireless)

wifi up >/dev/null 2>&1 
