#!/bin/sh

double_click_button_file=/var/run/double_click_button

. /etc/functions.sh
. /etc/version

if [ -f $double_click_button_file ]; then
. $double_click_button_file
	while [ "$dcount" -le "4" ]; do
		let dcount=$dcount+1
		echo "dcount=$dcount" > $double_click_button_file
		sleep 1
	done	
rm $double_click_button_file
fi

