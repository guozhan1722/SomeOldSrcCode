#!/bin/sh
. /usr/lib/webif/webif.sh

uci_load firewall
config_load firewall

echo "Firewall Configuration Committing... "
uci_commit firewall
/etc/init.d/firewall restart
echo "done"


