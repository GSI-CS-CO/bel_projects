#!/bin/bash
#Script for Testing SIO3 Event Filter read and write
echo ""
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

clear

read -p " Starte Auslesen des 4096x6 Filter RAM Inhalts  (Abbruch mit ^C, weiter mit CR)"

clear
echo "Auslesen des 4096x6 Filter RAM. Nach Powerup sind die Zellen 0x0000"




  for i in {0..8194..2}
  do
    var=$((0x4e2000+i))
    echo "ibase=10;obase=16;$((var))"|bc
    eb-read $scuname $((var))/2
  done
echo "Es weren 2 Zellen mehr ausgelesen, um Adressende zu checken. Der Read Error am Ende ist beabsichtigt"
echo ""
echo ""

read -p  " Starte Beschreiben des 4096x6 Filter RAM Inhalts  (Abbruch mit ^C, weiter mit CR)"
clear

echo " Beschreiben des 4096x6 Filter RAM mit 0x002C"

  for i in {0..8192..2}
  do
     var=$((0x4e2000+i))
     echo "ibase=10;obase=16;$((var))"|bc
   eb-write  $scuname $((var))/2 0x002c
   done
echo "Es werden 2 Zellen mehr ausgelesen, um Adressende zu checken. Der Read Error am Ende ist beabsichtigt"
echo ""
echo ""
   
   
read -p  " Auslesen des 4096x6 Filter RAM Inhalts 0x002C (Abbruch mit ^C, weiter mit CR)"
clear

echo " Readout of 4096x6 Filter RAM contents 0x002C is self-checking"
read -p " Continue with Return " weiter

   for i in {0..8192..2}
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
   
echo "Es werden 2 Zellen mehr ausgelesen, um Adressende zu checken. Der Read Error am Ende ist beabsichtigt"
echo ""
echo ""









