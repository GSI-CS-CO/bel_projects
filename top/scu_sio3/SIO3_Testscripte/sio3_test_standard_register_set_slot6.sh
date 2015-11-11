#!/bin/bash
#Script for Testing SIO standard registers, test register, OneWire ID and LEMO Registers
#K. Kaiser Vers.1 2015-08-26

echo "Test script für SIO3 standard register, test register, OneWire ID und LEMO Register"
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

echo "Auslesen der Standardregister"
echo ""
echo "read reg 0x4c0002 Slave ID (which is 0x2000)"
eb-read  $scuname 0x4c0002/2
echo "--------------------------------------------------"
echo "read reg 0x4c0004 FW Version (which is 0x0005 "
eb-read  $scuname 0x4c0004/2
echo "--------------------------------------------------"
echo "read reg 0x4c0006 FW Release (which is 0x0002)"
eb-read  $scuname 0x4c0006/2
echo "--------------------------------------------------"
echo "read reg 0x4c0008 System CID (which is 0x0037)"
eb-read  $scuname 0x4c0008/2
echo "--------------------------------------------------"
echo "read reg 0x4c000a Group CID (which is 0x001a)"
eb-read  $scuname 0x4c000a/2
echo "--------------------------------------------------"
echo "read reg 0x4c000c  Slave Macro Rev (which is 0x0502)"
eb-read  $scuname 0x4c000c/2
echo "--------------------------------------------------"
echo "read reg 0x4c000e System CID Extension Card (which is undefined)"
eb-read  $scuname 0x4c000e/2
echo "--------------------------------------------------"
echo "read reg 0x4c0010 Group CID Extension Card  (which is undefined)"
eb-read  $scuname 0x4c0010/2
echo "--------------------------------------------------"
echo "read reg 0x4c0012 Clock Frequency  (which is 0x30d4"
eb-read  $scuname 0x4c0012/2
echo "--------------------------------------------------"
echo "read reg 0x4c0020 Echo Register (which should be 0x0000 after reset) "
eb-read  $scuname 0x4c0020/2
echo "--------------------------------------------------"
echo "write reg 0x4c0020 Echo Register with 0xcafe "
eb-write $scuname 0x4c0020/2 0xcafe
echo "--------------------------------------------------"
echo "read reg 0x4c0020 Echo Register  (which should be 0xcafe) "
eb-read  $scuname 0x4c0020/2
echo "--------------------------------------------------"
echo "write reg 0x4c0020 Echo Register with 0xbabe "
eb-write $scuname 0x4c0020/2 0xbabe
echo "--------------------------------------------------"
echo "read reg 0x4c0020 Echo Register(which should be 0xbabe) "
eb-read  $scuname 0x4c0020/2
echo "--------------------------------------------------"
echo "read reg 0x4c0030 ClockSwitch Register "
eb-read  $scuname 0x4c0060/2
echo "--------------------------------------------------"
echo "Ende Standardregister Test "
echo ""
echo ""



echo "Nun folgt: Interrupt Test, Devicebus oder TwinAx Timing sind hierbei nicht verbunden"
read -p "Weiter mit Return, Abbruch mit CTRL C" weiter

clear
echo "Interrupt Test, Devicebus oder TwinAx Timing nicht verbunden"
echo "read reg 0x4c0040 Interrupt states -- Bit meanings see below"

eb-read  $scuname 0x4c0040/2
echo "--------------------------------------------------"
echo "read reg 0x4c0042 Interrupt enables (Bit0=PowerOn Intr=always enabled)"
eb-read  $scuname 0x4c0042/2
echo "--------------------------------------------------"
echo "write reg 0xffff to 0x4c0042 enables Interrupts"


eb-write $scuname 0x4c0042/2 0xffff
echo "--------------------------------------------------"
echo "read reg 0x4c0042 (enabled Interrupts) shows 0xffff "
eb-read  $scuname 0x4c0042/2
echo "--------------------------------------------------"
echo "read reg 0x4c0048 IRQ. Following interrupts may be pending:"
echo ""
echo "IFK runs on local clock  --> Bit 14 clk_sw "
echo "Devicebus plugged        --> Bit 6  Interlock "
echo "Data Ready               --> Bit 5  Data Ready "
echo "Data Request             --> Bit 4  Data Request "
echo "Delay Timer              --> Bit 3  Delay Timer "
echo "Timing Cable plugged     --> Bit 2  Event Fifo not empty "
echo "Every µs Timer           --> Bit 1  Every µs"
echo "PowerOn Interrupt        --> Bit 0  PowerOn (always enabled)"
echo "e.g x47 = Interlock, Everyµs and Power Interrupt"
 

eb-read  $scuname 0x4c0048/2
echo "--------------------------------------------------"
echo "write 0x0000 to reg 0x4c0048 erases IRQs"
eb-write $scuname 0x4c0048/2 0x0000
echo "--------------------------------------------------"
echo "read reg 0x4c0048  shows  new IRQs 0x0007 pending "
echo "because they were not masked"
eb-read  $scuname 0x4c0048/2
echo "--------------------------------------------------"
echo "write reg 0x0000 to 0x4c0042 disable interrupts"
eb-write $scuname 0x4c0042/2 0x0000
echo "--------------------------------------------------"
echo "read reg 0x4c0048 shows no more IRQs pending due to masking interrupts"
eb-read  $scuname 0x4c0048/2
echo "--------------------------------------------------"
echo ""
echo ""


echo " Nun folgt: Interrupttest (Devicebus und TwinAx Timing sind angeschlossen)"
read -p "Continue with Return, skip with CTRL C" weiter
clear

echo " Nun folgt: Interrupttest (Devicebus und TwinAx Timing sind angeschlossen)"
echo "read reg 0x4c0040 Interrupt states -- Bit meanings see below"

eb-read  $scuname 0x4c0040/2
echo "--------------------------------------------------"
echo "read reg 0x4c0042 Interrupt enables (Bit0=PowerOn Intr=always enabled)"
eb-read  $scuname 0x4c0042/2
echo "--------------------------------------------------"
echo "write reg 0xffff to 0x4c0042 enables Interrupts"


eb-write $scuname 0x4c0042/2 0xffff
echo "--------------------------------------------------"
echo "read reg 0x4c0042 (enabled Interrupts) shows 0xffff "
eb-read  $scuname 0x4c0042/2
echo "--------------------------------------------------"
echo "read reg 0x4c0048 IRQ. Following interrupts may be pending:"
echo "IFK runs on local clock  --> Bit 14 clk_sw "
echo "Devicebus plugged        --> Bit 6  Interlock "
echo "Data Ready               --> Bit 5  Data Ready "
echo "Data Request             --> Bit 4  Data Request "
echo "Delay Timer              --> Bit 3  Delay Timer "
echo "Timing Cable plugged     --> Bit 2  Event Fifo not empty "
echo "Every µs Timer           --> Bit 1  Every µs"
echo ""
 

eb-read  $scuname 0x4c0048/2
echo "--------------------------------------------------"
echo "write 0x0000 to reg 0x4c0048 erases IRQs"
eb-write $scuname 0x4c0048/2 0x0000
echo "--------------------------------------------------"
echo "read reg 0x4c0048  shows  new IRQs 0x0007 pending "
echo "because they were not masked"
eb-read  $scuname 0x4c0048/2
echo "--------------------------------------------------"
echo "write reg 0x0000 to 0x4c0042 disable interrupts"
eb-write $scuname 0x4c0042/2 0x0000
echo "--------------------------------------------------"
echo "read reg 0x4c0048 shows no more IRQs pending due to Maskes"
eb-read  $scuname 0x4c0048/2
echo "--------------------------------------------------"



echo "Nun folgt:  OneWire ID auslesen (ueber LM32)"
read -p "Weiter mit Return, Abbruch mit CTRL C"
echo ""

clear
echo " Start OneWire Test"
echo ""

echo "read  LM32 OneWire ID for DS28EC20 "
eb-read  $scuname 0x4c0080/2
echo "--------------------------------------------------"

echo "read  LM32 OneWire ID for DS28EC20  "
eb-read  $scuname 0x4c0082/2
echo "--------------------------------------------------"

echo "read  LM32 OneWire ID for DS28EC20  "
eb-read  $scuname 0x4c0084/2
echo "--------------------------------------------------"

echo "read  LM32 OneWire ID for DS28EC20  "
eb-read  $scuname 0x4c0086/2
echo "--------------------------------------------------"

echo ""
echo ""

echo "Nun folgt: LEMOs als Eingänge testen"
read -p "Weiter mit Return, Abbruch mit CTRL C"

clear
echo " Start of LEMO EingangsTest"
echo ""
read -p "Nun werden Ausgänge disabled    (weiter mit Return) " weiter
echo "write 0x0000 to reg 0x4c0812"
eb-write $scuname 0x4c0812/2 0x0000

echo "Auslesen aller LEMO Register"

echo "read  Lemo conf reg      0x4c0800 "
eb-read  $scuname 0x4c0800/2
echo "--------------------------------------------------"

echo "read  Lemo config status 0x4c0812 "
eb-read  $scuname 0x4c0816/2
echo "--------------------------------------------------"
echo "read  Lemo output reg    0x4c0814 "
eb-read  $scuname 0x4c0816/2
echo "--------------------------------------------------"
echo "read  Lemo pin status    0x4c0816 "
eb-read  $scuname 0x4c0816/2
echo "--------------------------------------------------"
read -p "Weiter mit Return, Abbruch mit CTRL C"
clear

echo " Teste LEMOs als Eingang"
read -p "Erwartet Low und keine ACT Leds (weiter mit Return) " weiter
echo "--------------------------------------------------"
read -p "Nun High an Buchse 1 anlegen    (weiter mit Return) " weiter
echo "read  Lemo pin status    0x4c0816 "
eb-read  $scuname 0x4c0816/2
echo "--------------------------------------------------"
read -p "Nun High an Buchse 2 anlegen    (weiter mit Return) " weiter
echo "read  Lemo pin status    0x4c0816 "
eb-read  $scuname 0x4c0816/2
echo "--------------------------------------------------"
read -p "Nun High an Buchse 3 anlegen    (weiter mit Return) " weiter
echo "read  Lemo pin status    0x4c0816 "
eb-read  $scuname 0x4c0816/2
echo "--------------------------------------------------"
read -p "Nun High an Buchse 4 anlegen    (weiter mit Return) " weiter
echo "read  Lemo pin status    0x4c0816 "
eb-read  $scuname 0x4c0816/2

read -p "Weiter mit Return, Abbruch mit CTRL C"
clear 
echo " LEMO Ausgangstest: Schalten der Ausgänge"
echo "--------------------------------------------------"
read -p "Nun werden Ausgänge auf high geschaltet    (weiter mit Return) " weiter
echo "write 0x000f to reg 0x4c0814"
eb-write $scuname 0x4c0814/2 0x000f
echo "--------------------------------------------------"
read -p "Noch gibt es keine Pegeländerung an den Ausgängen (weiter mit Return) " weiter
read -p "Nun werden Ausgänge enabled    (weiter mit Return) " weiter
echo "write 0x000f to reg 0x4c0812"
eb-write $scuname 0x4c0812/2 0x000f
echo "--------------------------------------------------"
read -p "Erwarteter Pegel High an allen Ausgängen (weiter mit Return) " weiter
echo "read  Lemo pin status    0x4c0816 "
eb-read  $scuname 0x4c0816/2
echo "--------------------------------------------------"
read -p "Nun werden Ausgänge auf low geschaltet    (weiter mit Return) " weiter
echo "write 0x000f to reg 0x4c0814"
eb-write $scuname 0x4c0814/2 0x0000
echo "--------------------------------------------------"
read -p "Erwarteter Pegel low, ACT LEDs erlöschen (weiter mit Return) " weiter
echo "read  Lemo pin status    0x4c0816 "
eb-read  $scuname 0x4c0816/2
echo "--------------------------------------------------"
read -p "Nun werden Ausgänge auf 0101 geschaltet    (weiter mit Return) " weiter
echo "write 0x0005 to reg 0x4c0814"
eb-write $scuname 0x4c0814/2 0x0005
echo "--------------------------------------------------"
read -p "Erwarteter Pegel 0101, ACT LEDs erlöschen tlws (weiter mit Return) " weiter
echo "read  Lemo pin status    0x4c0816 "
eb-read  $scuname 0x4c0816/2
echo "--------------------------------------------------"
read -p "Nun werden Ausgänge auf 1010 geschaltet    (weiter mit Return) " weiter
echo "write 0x000A to reg 0x4c0814"
eb-write $scuname 0x4c0814/2 0x000A
echo "--------------------------------------------------"
read -p "Erwarteter Pegel 1010, ACT LEDs erlöschen tlws (weiter mit Return) " weiter
echo "read  Lemo pin status    0x4c0816 "
eb-read  $scuname 0x4c0816/2

echo "-----------------Testende-------------------------"






