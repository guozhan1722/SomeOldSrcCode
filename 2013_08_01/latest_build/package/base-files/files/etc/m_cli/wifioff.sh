setdis=$(uci set wireless.radio0.disabled=1)
setdis=$(uci commit wireless)

wifi down
