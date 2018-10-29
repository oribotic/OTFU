#!/bin/bash
BIN=/sbin/ori

datainserted=false


#sleep 20

while true 
do
	numofproces=`ps ax | grep -v "grep" | grep -v "safe_ori" | grep -c /sbin/ori`

	if [ "$numofprocess" = 0 ]; then
		echo "starting /sbin/ori"
		/sbin/ori &
	fi

	if [ "$datainserted" = false ] ; then
		mysqlon=`ps ax | grep -v "grep"  | grep -c "mysql"`
		echo $mysqlon
		if [ "$mysqlon" -eq 1 ]; then
			echo "do mysql insert now"
			sudo mysql -u root -D oribotics < /home/pi/Oribot/bot.sql
			datainserted=true
		fi
	fi

	sleep 2
done


