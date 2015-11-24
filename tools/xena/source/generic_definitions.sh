#!/bin/bash 

# Define chassis and port
# MGMT Port
MGMT_MACHINE=192.168.1.170
MGMT_PORT=22611
# EXT port
EXT_MACHINE=172.16.255.200
EXT_PORT=22611

# Get system/user information
HOSTNAME=$(hostname)
USERNAME=$(whoami)

# Define password and owner
PASSWORD='"xena"'  # Must be in double quotes
OWNER='"BASHTEST"' # Must be in double quotes
