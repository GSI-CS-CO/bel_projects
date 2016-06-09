
SCU: Stop saftd
$ ps aux | grep saft
$ killall saftd

PC: Start saftd
$ sudo saftd baseboard:tcp/scuxl0041.acc.gsi.de
//check the start of saftd
$ saft-pps-gen baseboard -s
Output:
ECA configuration done for 2 IO(s)!
Next pulse at: 0x8e66fe0be2000 -> 1970-01-29 23:52:48.000000000
Next pulse at: 0x8e6701c58ea00 -> 1970-01-29 23:52:49.000000000
$ d-feet


$ sudo make saft-B2B-triggerSCU => saft-B2B-triggerSCU binary file
// saft-B2B-triggerSCU is used to config ECA table to produce TTL output from
// Lemo 2 IOs
/bel_projects/ip_cores/saftlib$ ./saft-B2B-triggerSCU baseboard -e -v
ECA configuration done for 2 IO(s)!
Waiting for timing events...
Got event at: 0x97a1acc61cea8 -> 1970-01-31 20:58:50.307096232

/bel_projects/ip_cores/saftlib$ sudo saft-ctl baseboard inject
0xdeadbeef22222222 0x0 +1 
or 
load elf file to LM32

Monitor the telegrams which go into the Timing network.
$ sudo saft-ctl baseboard snoop 0xdeadbeef22222222 0xffffffffffffffff


//How to config the boot file for OS on SCU 

$ ssh jbai@asl730.acc.gsi.de
$ PW
//Here lists all link info for the timing notes
[jbai@asl730 /]$ cd /common/export/nfsinit/scuxl0053
//Check the link info
>>ls
//Add saftlib and etherbone to SCU
>>ln -s ../global/saftlib-dev 40_saftlib-dev
>>ln -s ../global/saftlib-dev 30_etherbone-dev
