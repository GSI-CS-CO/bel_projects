#!/bin/bash

# Source functions and definitions
source source/generic_definitions.sh
source source/generic_functions.sh

# Define script variables
MINPARAMS=2       # Minimum arguments for this script
TIMEOUT=20        # Default telnet timeout
MACHINE=127.0.0.0 # Default value
PORT=0            # Default value
FILE=none         # Default value
FILE_HANDLE=3     # Default file handle
TERMINATE_APP=0   # End application

# Catch CTRL+C/SIGINT and terminate application
function sigint_handler()
{
  TERMINATE_APP=1
}

# Trap SIGINT
trap sigint_handler SIGINT

# Evaluate arguments
if [ $# -lt "$MINPARAMS" ]
then
  echo $"Usage: $0 {mgnt|ext} {filename}"
  exit 1
fi

# Get the machine/chassis name
case "$1" in
  mgnt)
    MACHINE=$MGMT_MACHINE
    PORT=$MGMT_PORT
    ;;
  ext)
    MACHINE=$EXT_MACHINE
    PORT=$EXT_PORT
    ;; 
  *)
    echo $"Usage: $0 {mgnt|ext} {filename}"
    exit 1
esac

# Get the script file name and check if this file really exists
FILE=$2
file $FILE > /dev/null 2>&1
if (($? > 0)); then
  echo "File $FILE does not exist!"
  exit 1
fi

# Open file handle 3 as TCP connection to chassis:port
echo "Trying to open a new connection..."
echo "Chassis:  $MACHINE"
echo "Port:     $PORT"
echo "Script:   $FILE"
echo "Owner:    $OWNER"
echo "Password: $PASSWORD"

exec 3<> /dev/tcp/${MACHINE}/${PORT} # create file handle 3
if [ $? -eq 0 ]
then
  echo "Telnet accepting connections..."
else
  echo "Telnet connections not possible!"
  exit 1
fi

# Try to login
echo "Trying to log in..."
auto_exchange_flow_control "C_LOGON ${PASSWORD}"

# Try to reserve our resources
auto_exchange_flow_control "C_OWNER ${OWNER}"

# Set the timeout 
auto_exchange_flow_control "C_TIMEOUT ${TIMEOUT}"

# Parse the given script here
while read line
do
  auto_exchange_flow_control $line
done < $FILE

# Wait for further messages or SIGINT
while [ ${TERMINATE_APP} -eq 0 ]
do
  auto_exchange_flow_control
done

# Close file handle 3
echo ""
echo "Closing connection..."
exec 3>&-
if [ $? -eq 0 ]
then
  echo "Telnet accepting close..."
else
  echo "Telnet close not possible!"
  exit 1
fi

# Done
exit 0
