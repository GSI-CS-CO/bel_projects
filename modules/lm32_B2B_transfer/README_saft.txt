
SCU: Stop saftd
$ ps aux | grep saft
$ kill 

PC: Start saftd
$ sudo saftd baseboard:tcp/scuxl0041.acc.gsi.de
$ saft-pps-gen baseboard -s
Output:
ECA configuration done for 2 IO(s)!
Next pulse at: 0x8e66fe0be2000 -> 1970-01-29 23:52:48.000000000
Next pulse at: 0x8e6701c58ea00 -> 1970-01-29 23:52:49.000000000
$ d-feet

$ sudo make saft-B2B-triggerSCU => saft-B2B-triggerSCU binary file
// saft-B2B-triggerSCU is used to config ECA table to produce TTL output from
// Lemo 2 IOs
$ sudo ./saft-B2B-triggerSCU baseboard -s -e

Monitor the telegrams which go into the Timing network.
$ sudo saft-ctl baseboard snoop 0xdeadbeef22222222 0xffffffffffffffff

