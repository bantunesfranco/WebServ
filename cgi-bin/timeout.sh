#!/usr/bin/env sh

# Get the start time
start_time=`date +%s`
duration=60
loops=0

# Run the loop until the specified duration is reached
while true; do
	loops=$((loops+1))
	>&2 echo "loops: $loops"
	# Get the current time
	current_time=`date +%s`
	elapsed_time=$((current_time - start_time))

	# Check if the duration has been reached
	if [ $elapsed_time -ge $duration ]
	then
		break
	fi

	done
