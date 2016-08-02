#!/bin/bash

# load some environment variables
if [ -a /etc/profile.d/tissuestack_env.sh ]; then
	source /etc/profile.d/tissuestack_env.sh
else
	if [ "$TISSUE_STACK_ENV" != "" ];then
		source $TISSUE_STACK_ENV
	else
		echo "ERROR: You are missing environment variables!!"
		exit -1
	fi
fi

# actual script
SHUTDOWN_TRIES=10
EVERYTHING_IS_DOWN=false
PROCESSES=(TissueStackServer)
PROCESS_COUNT=${#PROCESSES[*]}
FIRST_ARGUMENT=none
SECOND_ARGUMENT=none

if [ $# -gt 0 ] && [[ $1 -eq "start"  ||  $1 -eq "stop"  ||  $1 -eq "restart"  ]]; then
	FIRST_ARGUMENT=$1
else
	echo -e "No/invalid command line argument given, falling back onto default: 'restart'. Other options are 'start' and 'stop'"
	FIRST_ARGUMENT="restart"
fi

if [ $# -gt 1 ] && [ $2 == "force" ]; then
	SECOND_ARGUMENT=$2
fi

while [ $SHUTDOWN_TRIES -gt 0 ] && [ $EVERYTHING_IS_DOWN == false ]; do
	echo -e "Checking for running TissueStack components...\n"
	for i in ${PROCESSES[*]}; do
		PID_OF_PROCESS=`pgrep -f $i`
		if  [ "$PID_OF_PROCESS" != "" ]; then
			echo "$PID_OF_PROCESS ($i) is running. Shutting it down now... Number of global retries left: $SHUTDOWN_TRIES"
			if [ $SECOND_ARGUMENT == "force" ]; then
				kill -9 $PID_OF_PROCESS
			else
				kill $PID_OF_PROCESS
			fi
			(( SHUTDOWN_TRIES -= 1 ))
			sleep 5
		else
			(( PROCESS_COUNT -= 1 ))
		fi
	done
	if [ $PROCESS_COUNT -eq 0 ] || [ $PROCESS_COUNT -lt 0 ] ; then
		EVERYTHING_IS_DOWN=true
	fi
done

echo -e "\nEverything's down!"

if [ $FIRST_ARGUMENT == "stop" ]; then
	exit 0
fi

echo -e "\nNow start everything again ... (Check following output for errors during during Tissue Stack startup!!) \n"

$IMAGE_SERVER_EXE
