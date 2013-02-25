#!/bin/sh
# Receive IR commands from Funky v2 and do stuff..

#set terminal speed nice n easy
stty -F /dev/ttyACM0 9600

while [ 1 ]
do
#read latest data from the serial port
read var1 < /dev/ttyACM0

tokens=( $var1 )
#show what we got from port
#echo $var1

if [ ${tokens[2]} == "2602743F" ]; then
   echo "Received shutdown command, shutting down, use 'sudo kill $$' to abort within 30 seconds.."
   sleep 30
   sudo halt -p
fi

# you could send this to pachube..
#pass results to wget to post to pachube
#/usr/bin/wget \
#--header="X-ApiKey: YourAPIkey" \
#--header="X-Http-Method-Override: put" \
#--post-data "$var1" \
#"https://pachube.com/api/YourFeedID.csv"

#remove unwanted file logs as space is tight on router
#rm YourFeedID.csv*

#sleep however long you need not to exceed your quota
#sleep 900
done
