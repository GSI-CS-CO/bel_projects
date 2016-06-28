                      Instructions of the configuration for the B2B transfer system

===============================================================================================================
For the source B2B SCU, please do the following configuration:
//configure the ECA queue
# ssh root@scuxlNo.acc.gsi.de
# saft-ecpu-ctl baseboard -x
# saft-ecpu-ctl baseboard -c 0xdeadbeef00000000 32 0 0
Action sink configured...

//B2 output set to 0. B2 is an input
# saft-io-ctl baseboard B2 -o 0

//load firmware to the LM32
lm32-ctl
config inst 1
config hw 1/0
load B2B_main_src.elf

or
(Prefer)
make B2B_main_src.bin
/bel_projects/tools$ ./eb-fwload tcp/scuxl0020.acc.gsi.de u0 0 ../modules/lm2B_transfer/B2B_main_src.bin

===============================================================================================================
For the target B2B SCU, please do the following configuration:
//configure the ECA queue
# ssh root@scuxlNo.acc.gsi.de 
# saft-ecpu-ctl baseboard -x
# saft-ecpu-ctl baseboard -c 0xdeadbeef00000000 32 0 0
Action sink configured...

//B2 output set to 0. B2 is an input
# saft-io-ctl baseboard B2 -o 0

//load firmware to the LM32
lm32-ctl
config inst 1
config hw 0/1
load B2B_main_trg.elf

or
(Prefer)
make B2B_main_trg.bin
/bel_projects/tools$ ./eb-fwload tcp/scuxl0020.acc.gsi.de u0 0 ../modules/lm2B_transfer/B2B_main_trg.bin

===============================================================================================================
For the trigger SCU, please do the following configuration:
//kill saftlib on SCU
ssh root@scuxlNo.acc.gsi.de
killall saftd
//start saftlib in PC
/bel_projects/ip_cores/saftlib$ sudo saftd baseboard:tcp/scuxlNo.acc.gsi.de
//B2 output set to 1. B2 is an onput
# saft-io-ctl baseboard B2 -o 1
//check B2 info
# saft-io-ctl baseboard B2
//To monitor the timing events
# saft-io-ctl baseboard B1 -s
/bel_projects/ip_cores/saftlib$./saft-B2B-triggerSCU baseboard -v

===============================================================================================================
Use packeth to produce the timing event EVT_B2B_START
Payload for the EVT_B2B_START:ipv4(0800) 
45000048000040003f11451ec0a80001c0a80002ebd0ebd1003400004e6f14448a0f08007ffffff0deadbeef1111111100013085159d822a0000000000000000000984299b573950
or
# simulation DM to produce the EVT_B2B_BEGIN
~/test/bel_projects_dm/bel_projects/modules/DM_B2B$./DM_configure.sh tcp/scuxl0041.acc.gsi.de

===============================================================================================================




