#!/bin/bash

# Write/Send commands
function write_to_file_handle ()
{
  COMMAND=$*
  if [ ! -z "${COMMAND}" ] # Check if line/command is not empty
  then
    echo "Command: $*"
    echo -en "$*\r\n" >&3 # Add return and endline
  fi
  return 0
}

# Read/Get reply
function read_from_file_handle ()
{
  read -t 1 <&3 # Will only read one line of answer
  if [ ! -z "${REPLY}" ] # Check if we got a reply
  then
    echo -n "Reply: "
    echo $REPLY # Read command will save the answer as $REPLY
  fi
  return 0
}

# Combine write and read functions
function auto_transmit()
{
  write_to_file_handle $*
  read_from_file_handle
  return 0
}
