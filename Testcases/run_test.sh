#!/bin/sh
if [ ! -d $1 ]; then
	echo $0: missing testcase directory
	exit 1
fi
# check for bt script
if [ -r $1/script ]; then
	cd $1
	if [ "$2" = "create_output_masters" ]; then
		../../bt <script >output_master 2>&1
	else
		../../bt <script >output_test 2>&1
		diff output_master output_test >/dev/null 2>&1
		if [ $? -eq 0 ]; then
			echo -e "$1: \tpassed"
		else
			echo -e "$1: \tFAILED <--------------------------"
		fi
	fi
# check for shell script
elif [ -r $1/script.sh ]; then
	cd $1
	if [ "$2" = "create_output_masters" ]; then
		sh ./script.sh >output_master 2>&1
	else
		sh ./script.sh >output_test 2>&1
		diff output_master output_test >/dev/null 2>&1
		if [ $? -eq 0 ]; then
			echo -e "$1: \tpassed"
		else
			echo -e "$1: \tFAILED <--------------------------"
		fi    
    fi
else
	echo -e "$1: \tno script file"
fi
