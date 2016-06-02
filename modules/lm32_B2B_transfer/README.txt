ssh root@scuxl0041.acc.gsi.de
Pw: geheim

branch: network_monitor_asterisk_test_pps_out
bel_projects/modules/syncmon/scripts/configure-data-master.sh

jbai@belpc136:~/test/bel_projects_B2B/bel_projects/modules/lm32_B2B_transfer$
ssh root@scuxl0020.acc.gsi.de
root@scuxl0020.acc.gsi.de's password: geheim

config inst 1
config hw 0

# Start saftlib 
/bel_projects/ip_cores/saftlib$ sudo saftd baseboard:tcp/scuxl0041.acc.gsi.de

# simulation DM to produce the EVT_B2B_BEGIN
~/test/bel_projects_dm/bel_projects/modules/DM_B2B$ 

eb-ls tcp/scuxl0041.acc.gsi.de

SCU: three lm32  
lm32 inst 0(ftm_lm32:\G1:0) => FG; LM32-IRQ-EP 20800 20900 20a00 (not aviable)
lm32 inst 1(ftm_lm32:\G1:1) => user(B2B); LM32-IRQ-EP 20b00 (irq_slave_i[0])
20b00(irq_slave_i[1]) 20c00(irq_slave_i[2] not aviable) 
lm32 inst 2 => PTP core

B2B_main.c.int => LM32 intrupt is OK
Need to be done:(ECA updates...)
ECA produces intrupt for LM32 and LM32 reads data from ECA Queue

/* GSI_TM_LATCH_V2 */
/* TLU */ there is one FIFO for channel 0 (B1) and other FIFO for channel 1 (B2)
TLU_CLEAR 0x3 //clear channel 0 and channel 1, they start to timestamp at same
time 
TLU_CH_SELECT 0x0 // select channel 0
TLU_CH_TIME1 
TLU_CH_TIME0 // read timestamp 
TLU_POP //channel 0 pop the timestamp out, the number of data in FIFO 0 minus 1

TLU_CH_SELECT 0x1 // select channel 1
TLU_CH_TIME1 
TLU_CH_TIME0 // read timestamp 
TLU_POP //channel 1 pop the timestamp out, the number of data in FIFO 1 minus 1

For SCU B2 input => output: modify scu_control.vhd gpio_i(1) lemo_io 

ECA

# Activate ECA Queue for events ID 0x881122 or Event tag 5
[root@scuxl0041 ~]# saft-ecpu-ctl baseboard -x
[root@scuxl0041 ~]# saft-ecpu-ctl baseboard -c 0xdeadbeef11111111 64 0 5
Action sink configured...

# Send event
[root@scuxl0041 ~]# saft-ctl baseboard inject 0xdeadbeef11111111
0xaaaaaaaabbbbbbbb 0
#Pop the event out
eb-write tcp/scuxl0041.acc.gsi.de 0xc4/4 0x1
# Read event content
jbai@belpc136:~/test/bel_projects_B2B/bel_projects/modules/B2B_queue_test$ ./ECA_QUEUE_READ.sh

Timing events assumption:
EVT_B2B_BEGIN 0xdeadbeef11111111

SCU IO config by saftlib
//B2 output set to 0. B2 is an input
[root@scuxl0041 jbai]# saft-io-ctl baseboard B2 -o 0 
//check B2 info
[root@scuxl0041 jbai]# saft-io-ctl baseboard B2
[root@scuxl0041 jbai]# saft-io-ctl baseboard B1 -s
