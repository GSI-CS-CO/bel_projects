#!/bin/bash

# Global flags
flag_found_constant=0      # C_xxx
flag_found_wait=0          # WAIT xxx
flag_found_filter_term=0   # PM_xxx, PL_xxx, PF_xxx
flag_found_statistics=0    # PT_xxx, PR_xxx
flag_found_query=0         # xx_xxx ?
flag_found_comment=0       # ; xxx xxx xxx
expected_reply=None        # expected reply

# Clear all global flags
function clear_flags ()
{
  return 0
}

# Set global flags
function inspect_command_and_command ()
{
  INSPECTION=$*
  if [[ ${INSPECTION} == *"C_"* ]]
  then
    echo "Found CONSTANT in ${INSPECTION}!";
  fi
  if [[ ${INSPECTION} == *"WAIT"* ]]
  then
    echo "Found WAIT in ${INSPECTION}!";
  fi
  return 0
}

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

# Simple command -> reply chain
function auto_exchange ()
{
  write_to_file_handle $*
  read_from_file_handle
  return 0
}

# Exchange date with automatic flow control
function auto_exchange_flow_control ()
{
  inspect_command_and_command $*
  write_to_file_handle $*
  read_from_file_handle
  return 0
}

# Connect to Xena Chassis
function telnet_connect ()
{
  return 0 # TBD
}

# Disconnect from Xena Chassis
function telnet_disconnect ()
{
  return 0 # TBD
}

# Log in to Xena Chassis
function telnet_login ()
{
  return 0 # TBD
}
