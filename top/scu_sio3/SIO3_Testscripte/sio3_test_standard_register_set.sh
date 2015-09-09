#!/bin/bash
#Script for Testing SIO3 standard registers, test register, OneWire ID and LEMO Registers
#K. Kaiser Vers.1 2015-08-26

echo ""
echo ""
echo "Script for Testing SIO3 standard registers, test register, OneWire ID and LEMO Registers"
echo ""

scuname="tcp/scuxl0079.acc"

echo "Script needs to be used with SCU  "$scuname "and SIO2 or SIO3 in Slave slot 7"
echo "For other SCUs change variable for scuname in the script"
read -p "Continue with Return, skip with CTRL C"

echo "read reg 0x4e0002 Slave ID (which is 0x2000)"
eb-read  $scuname 0x4e0002/2
echo "--------------------------------------------------"
echo "read reg 0x4e0004 FW Version (which is 0x0005 "
eb-read  $scuname 0x4e0004/2
echo "--------------------------------------------------"
echo "read reg 0x4e0006 FW Release (which is 0x0002)"
eb-read  $scuname 0x4e0006/2
echo "--------------------------------------------------"
echo "read reg 0x4e0008 System CID (which is 0x0037)"
eb-read  $scuname 0x4e0008/2
echo "--------------------------------------------------"
echo "read reg 0x4e000a Group CID (which is 0x001a)"
eb-read  $scuname 0x4e000a/2
echo "--------------------------------------------------"
echo "read reg 0x4e000c  Slave Macro Rev (which is 0x0502)"
eb-read  $scuname 0x4e000c/2
echo "--------------------------------------------------"
echo "read reg 0x4e000e System CID Extension Card (which is undefined)"
eb-read  $scuname 0x4e000e/2
echo "--------------------------------------------------"
echo "read reg 0x4e0010 Group CID Extension Card  (which is undefined)"
eb-read  $scuname 0x4e0010/2
echo "--------------------------------------------------"
echo "read reg 0x4e0012 Clock Frequency  (which is 0x30d4"
eb-read  $scuname 0x4e0012/2
echo "--------------------------------------------------"
echo "read reg 0x4e0020 Echo Register (which should be 0x0000 after reset) "
eb-read  $scuname 0x4e0020/2
echo "--------------------------------------------------"
echo "write reg 0x4e0020 Echo Register with 0xcafe "
eb-write $scuname 0x4e0020/2 0xcafe
echo "--------------------------------------------------"
echo "read reg 0x4e0020 Echo Register  (which should be 0xcafe) "
eb-read  $scuname 0x4e0020/2
echo "--------------------------------------------------"
echo "write reg 0x4e0020 Echo Register with 0xbabe "
eb-write $scuname 0x4e0020/2 0xbabe
echo "--------------------------------------------------"
echo "read reg 0x4e0020 Echo Register(which should be 0xbabe) "
eb-read  $scuname 0x4e0020/2
echo "--------------------------------------------------"
echo "Now testing some Interrupts firstly no Devicebus or Timing cables connected"
read -p "Continue with Return, skip with CTRL C" weiter
echo ""
echo "read reg 0x4e0040 Interrupt states -- Bit meanings see below"

eb-read  $scuname 0x4e0040/2
echo "--------------------------------------------------"
echo "read reg 0x4e0042 Interrupt enables (Bit0=PowerOn Intr=always enabled)"
eb-read  $scuname 0x4e0042/2
echo "--------------------------------------------------"
echo "write reg 0xffff to 0x4e0042 enables Interrupts"


eb-write $scuname 0x4e0042/2 0xffff
echo "--------------------------------------------------"
echo "read reg 0x4e0042 (enabled Interrupts) shows 0xffff "
eb-read  $scuname 0x4e0042/2
echo "--------------------------------------------------"
echo "read reg 0x4e0048 IRQ. Following interrupts may be pending:"
echo "IFK runs on local clock  --> Bit 14 clk_sw "
echo "Devicebus plugged        --> Bit 6  Interlock "
echo "Data Ready               --> Bit 5  Data Ready "
echo "Data Request             --> Bit 4  Data Request "
echo "Delay Timer              --> Bit 3  Delay Timer "
echo "Timing Cable plugged     --> Bit 2  Event Fifo not empty "
echo "Every µs Timer           --> Bit 1  Every µs"
echo ""
 

eb-read  $scuname 0x4e0048/2
echo "--------------------------------------------------"
echo "write 0x0000 to reg 0x4e0048 erases IRQs"
eb-write $scuname 0x4e0048/2 0x0000
echo "--------------------------------------------------"
echo "read reg 0x4e0048  shows  new IRQs 0x0007 pending "
echo "because they were not masked"
eb-read  $scuname 0x4e0048/2
echo "--------------------------------------------------"
echo "write reg 0x0000 to 0x4e0042 disable interrupts"
eb-write $scuname 0x4e0042/2 0x0000
echo "--------------------------------------------------"
echo "read reg 0x4e0048 shows no more IRQs pending due to Maskes"
eb-read  $scuname 0x4e0048/2
echo "--------------------------------------------------"
echo ""
echo ""


echo "Now testing some Interrupts (now Devicebus and Timing cables are connected)"
read -p "Continue with Return, skip with CTRL C" weiter
echo ""


echo "read reg 0x4e0040 Interrupt states -- Bit meanings see below"

eb-read  $scuname 0x4e0040/2
echo "--------------------------------------------------"
echo "read reg 0x4e0042 Interrupt enables (Bit0=PowerOn Intr=always enabled)"
eb-read  $scuname 0x4e0042/2
echo "--------------------------------------------------"
echo "write reg 0xffff to 0x4e0042 enables Interrupts"


eb-write $scuname 0x4e0042/2 0xffff
echo "--------------------------------------------------"
echo "read reg 0x4e0042 (enabled Interrupts) shows 0xffff "
eb-read  $scuname 0x4e0042/2
echo "--------------------------------------------------"
echo "read reg 0x4e0048 IRQ. Following interrupts may be pending:"
echo "IFK runs on local clock  --> Bit 14 clk_sw "
echo "Devicebus plugged        --> Bit 6  Interlock "
echo "Data Ready               --> Bit 5  Data Ready "
echo "Data Request             --> Bit 4  Data Request "
echo "Delay Timer              --> Bit 3  Delay Timer "
echo "Timing Cable plugged     --> Bit 2  Event Fifo not empty "
echo "Every µs Timer           --> Bit 1  Every µs"
echo ""
 

eb-read  $scuname 0x4e0048/2
echo "--------------------------------------------------"
echo "write 0x0000 to reg 0x4e0048 erases IRQs"
eb-write $scuname 0x4e0048/2 0x0000
echo "--------------------------------------------------"
echo "read reg 0x4e0048  shows  new IRQs 0x0007 pending "
echo "because they were not masked"
eb-read  $scuname 0x4e0048/2
echo "--------------------------------------------------"
echo "write reg 0x0000 to 0x4e0042 disable interrupts"
eb-write $scuname 0x4e0042/2 0x0000
echo "--------------------------------------------------"
echo "read reg 0x4e0048 shows no more IRQs pending due to Maskes"
eb-read  $scuname 0x4e0048/2
echo "--------------------------------------------------"
echo ""
echo ""




echo "Now testing both Test User Registers"
read -p "Continue with Return, skip with CTRL C"
echo ""

echo "read Test User Register 0x4e0400 "
eb-read  $scuname 0x4e0400/2
echo "--------------------------------------------------"
echo "write 0x5a5a to Test User Register 0x4e0400"
eb-write $scuname 0x4e0400/2 0x5a5a
echo "--------------------------------------------------"
echo "read  back Test User Register  0x4e0400  shows 0x5a5a"
eb-read  $scuname 0x4e0400/2
echo "--------------------------------------------------"
echo "read Test User Register  0x4e0400 "
eb-read  $scuname 0x4e0400/2
echo "--------------------------------------------------"
echo "write 0xa5a5 to Test User Register  0x4e0400"
eb-write $scuname 0x4e0400/2 0xa5a5
echo "--------------------------------------------------"
echo "read  back Test User Register  0x4e0400  shows 0xa5a5"
eb-read  $scuname 0x4e0400/2
echo "-------------------same for next reg----------------------"

echo "read Test User Register  0x4e0402 "
eb-read  $scuname 0x4e0402/2
echo "--------------------------------------------------"
echo "write 0x5a5a to Test User Register 0x4e0402"
eb-write $scuname 0x4e0402/2 0x5a5a
echo "--------------------------------------------------"
echo "read  back Test User Register 0x4e0402  shows 0x5a5a"
eb-read  $scuname 0x4e0402/2
echo "--------------------------------------------------"
echo "read Test User Register 0x4e0402 "
eb-read  $scuname 0x4e0402/2
echo "--------------------------------------------------"
echo "write 0xa5a5 to Test User Register  0x4e0402"
eb-write $scuname 0x4e0402/2 0xa5a5
echo "--------------------------------------------------"
echo "read  back Test User Register  0x4e0402  shows 0xa5a5"
eb-read  $scuname 0x4e0402/2
echo "--------------------------------------------------"

echo ""
echo ""

echo "Now testing OneWire ID which is forwarded by LM32"
read -p "Continue with Return, skip with CTRL C"
echo ""

echo "read  LM32 OneWire ID for DS28EC20 "
eb-read  $scuname 0x4e0080/2
echo "--------------------------------------------------"

echo "read  LM32 OneWire ID for DS28EC20  "
eb-read  $scuname 0x4e0082/2
echo "--------------------------------------------------"

echo "read  LM32 OneWire ID for DS28EC20  "
eb-read  $scuname 0x4e0084/2
echo "--------------------------------------------------"

echo "read  LM32 OneWire ID for DS28EC20  "
eb-read  $scuname 0x4e0086/2
echo "--------------------------------------------------"


echo ""
echo ""

echo "Now testing LEMOs"
read -p "Continue with Return, skip with CTRL C"
echo ""
read -p "Nun werden Ausgänge disabled    (weiter mit Return) " weiter
echo "write 0x0000 to reg 0x4e0812"
eb-write $scuname 0x4e0812/2 0x0000

echo "read  Lemo conf reg      0x4e0800 "
eb-read  $scuname 0x4e0800/2
echo "--------------------------------------------------"

echo "read  Lemo config status 0x4e0812 "
eb-read  $scuname 0x4e0816/2
echo "--------------------------------------------------"
echo "read  Lemo output reg    0x4e0814 "
eb-read  $scuname 0x4e0816/2
echo "--------------------------------------------------"
echo "read  Lemo pin status    0x4e0816 "
eb-read  $scuname 0x4e0816/2
echo "--------------------------------------------------"
read -p "Erwartet Low und keine ACT Leds (weiter mit Return) " weiter
echo "--------------------------------------------------"
read -p "Nun High an Buchse 1 anlegen    (weiter mit Return) " weiter
echo "read  Lemo pin status    0x4e0816 "
eb-read  $scuname 0x4e0816/2
echo "--------------------------------------------------"
read -p "Nun High an Buchse 2 anlegen    (weiter mit Return) " weiter
echo "read  Lemo pin status    0x4e0816 "
eb-read  $scuname 0x4e0816/2
echo "--------------------------------------------------"
read -p "Nun High an Buchse 3 anlegen    (weiter mit Return) " weiter
echo "read  Lemo pin status    0x4e0816 "
eb-read  $scuname 0x4e0816/2
echo "--------------------------------------------------"
read -p "Nun High an Buchse 4 anlegen    (weiter mit Return) " weiter
echo "read  Lemo pin status    0x4e0816 "
eb-read  $scuname 0x4e0816/2
echo "--------------------------------------------------"
read -p "Nun werden Ausgänge auf high geschaltet    (weiter mit Return) " weiter
echo "write 0x000f to reg 0x4e0814"
eb-write $scuname 0x4e0814/2 0x000f
echo "--------------------------------------------------"
read -p "Noch gibt es keine Pegeländerung an den Ausgängen (weiter mit Return) " weiter
read -p "Nun werden Ausgänge enabled    (weiter mit Return) " weiter
echo "write 0x000f to reg 0x4e0812"
eb-write $scuname 0x4e0812/2 0x000f
echo "--------------------------------------------------"
read -p "Erwarteter Pegel High an allen Ausgängen (weiter mit Return) " weiter
echo "--------------------------------------------------"
read -p "Nun werden Ausgänge auf low geschaltet    (weiter mit Return) " weiter
echo "write 0x000f to reg 0x4e0814"
eb-write $scuname 0x4e0814/2 0x0000
echo "--------------------------------------------------"
read -p "Erwarteter Pegel low, ACT LEDs erlöschen (weiter mit Return) " weiter
echo "-----------------Testende-------------------------"






