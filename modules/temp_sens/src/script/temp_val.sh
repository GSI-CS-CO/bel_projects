#!/bin/bash
#This script is to convert hexadecimal value to corresponding temperature value

	value=`sudo eb-read dev/ttyUSB0 0x400/4`

#Suppress 0 from the value

	hex_val=`echo $value|sed 's/0*//'`

#Convert hexadecimal value to decimal value

	dec_val=$((16#$hex_val))

	declare -i const_val
	const_val=128

#Based on the data sheet provided by Altera temperature sensor, decimal value must be
#subtracted by a constant value 128 to get the corresponding temperature value

	temp_val=`expr $dec_val - $const_val`

	if [ $hex_val != "deadc0de" ]; then

		echo "$temp_val degC"

	else

		echo $hex_val

	fi

exit 0



