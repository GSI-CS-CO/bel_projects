#!/bin/bash
#Script for Testing SIO3 Event Filter read and write
echo ""
scuname="tcp/scuxl0079.acc"
echo "Script for Testing SIO3 Event Filter read and write"

echo ""

echo "This script must be used with "$scuname " and SIO2 or SIO3 at slave slot 7"
echo "In case of wrong scu change variable scuname in this script"

read -p "Continue with Return or finish with CTRL C" weiter 

echo ""

 echo " Readout of 4096x6 Filter RAM. After Powerup the data is 0x0000"



  for i in {0..256..2}
  do
    var=$((0x4e2000+i))
    echo "ibase=10;obase=16;$((var))"|bc
    eb-read $scuname $((var))/2
  done


read -p " Continue with Return " weiter


echo " write of 4096x6 Filter RAM with data 0x002c"
read -p " Continue with Return " weiter


  for i in {0..256..2}
  do
     var=$((0x4e2000+i))
     echo "ibase=10;obase=16;$((var))"|bc
   eb-write  $scuname $((var))/2 0x002c
   done


echo " Readout of 4096x6 Filter RAM"
read -p " Continue with Return " weiter

   for i in {0..256..2}
   do
     var=$((0x4e2000+i))
     echo "ibase=10;obase=16;$((var))"|bc
     lesewert=$(eb-read  $scuname $((var))/2)

     if [ 0x$lesewert == 0x"002c" ] 
      then
       :
      else
       echo $lesewert
      fi
   done









