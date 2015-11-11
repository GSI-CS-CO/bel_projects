#!/bin/bash
#Script for Testing Transmission using IFK Echo Register Response 

echo "Script for Testing Transmission using IFK Echo Register Response"
echo "Sie können das Script jederzeit mit CTRL C abbrechen"
sleep 2

clear


eingabefehler=j
while eingabefehler=j 
 do
  read -p "Geben Sie die SCU Nummer ein (z.B. 0022 für scuxl0022):" scunummer
  scuname="tcp/scuxl$scunummer.acc"
  echo ""
  echo "Script benötigt "$scuname "und SIO im Slave slot 7"
  echo ""
  read -p  "Ist der Test Set-Up ok ( j / n ):" eingabefehler 
   if [ "$eingabefehler" == "j" ] 
    then
     break
    else
     echo -e "\e[7mSie können den Namen nun aendern\e[0m"
     sleep 1
   fi
done

read -p "Start Devicebus Test  (Abbruch mit ^C, weiter mit CR)"
echo "Achten sie auf auch auf die TRM und RCV LEDs"
clear

echo "Start Devicebustest"

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


echo "read of echo-ed word at command register 0x4e0802"
eb-read  $scuname 0x4e0802/2


echo "read status and control register 0x4e0804"
eb-read  $scuname 0x4e0804/2

echo "--------------------------------------------------"
echo "--------------------------------------------------"
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
echo ""

echo "Ende Device Bus Test"
echo "--------------------------------------------------"

