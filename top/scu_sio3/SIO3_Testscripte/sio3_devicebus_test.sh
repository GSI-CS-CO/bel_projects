#!/bin/bash
#Script for Testing Transmission using IFK Echo Register Response 

echo ""

scuname="tcp/scuxl0079.acc"

echo "This script must be used with "$scuname " and SIO2 or SIO3 at slave slot 7"
echo "In case of wrong scu change variable scuname in this script"

read -p " Continue with Return or finish with CTRL C" weiter 
echo ""

echo "set statusreg bit 15 to external HD6408 decoder"
eb-write $scuname 0x4e0804/2 0x1000
echo "read status and control register 0x4e0804"
eb-read  $scuname 0x4e0804/2
echo "--------------------------------------------------"

echo ""
data=0x5a5a
echo "write data "$data" to data register 0x4e0800"
eb-write $scuname 0x4e0800/2 $data

echo ""
echo "write function code 13 and ifk address 79 to command register 0x4e0802"
eb-write $scuname 0x4e0802/2 0x1379


echo "write function code 89 and ifk address 79 to command register 0x4e0802"
eb-write $scuname 0x4e0802/2 0x8979

echo "--------------------------------------------------"
echo ""


echo "read of echo-ed word at command register 0x4e0802"
eb-read  $scuname 0x4e0802/2

echo "read status and control register 0x4e0804"
eb-read  $scuname 0x4e0804/2

echo "--------------------------------------------------"

echo ""
data=0xa5a5
echo "write data "$data" to data register 0x4e0800"
eb-write $scuname 0x4e0800/2 $data


echo "write function code 13 and ifk address 79 to command register 0x4e0802"
eb-write $scuname 0x4e0802/2 0x1379


echo "write function code 89 and ifk address 79 to command register 0x4e0802"
eb-write $scuname 0x4e0802/2 0x8979

echo "--------------------------------------------------"



echo "read of echo-ed word at command register 0x4e0802"
eb-read  $scuname 0x4e0802/2

echo "read status and control register 0x4e0804"
eb-read  $scuname 0x4e0804/2

echo "--------------------------------------------------"

