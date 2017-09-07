
#!/bin/bash
#Script for Write some Data using TX_FIFO to IFK

clear

MIL_OPTION_adresse=0x20008000

mil_option_data_adresse=0x20008000
mil_option_ctrl_adresse=0x20008002
mil_option_stat_adresse=0x20008004



scuname="tcp/scuxl0079.acc"

echo "########################Start Devicebustest SIO12 with data x5A5A  ###############"
#echo "set statusreg bit 15 to external HD6408 decoder"

data=0x5a5a
echo "write data "$data" to data register "
eb-write $scuname ${mil_option_data_adresse}/2 $data
echo "write function code 13 and ifk address 79 to command register "
eb-write $scuname ${mil_option_ctrl_adresse}/2 0x1379
sleep 1
data=0xbabe
echo "write data "$data" to data register "
eb-write $scuname ${mil_option_data_adresse}/2 $data
echo "write function code 13 and ifk address 79 to command register "
eb-write $scuname ${mil_option_ctrl_adresse}/2 0x1320
