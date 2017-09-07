
#!/bin/bash
#Script for Write some Data using TX_FIFO to IFK

clear

scunummer=0079
scuname="tcp/scuxl0079.acc"

echo "Per eb-find errechnete Wishbone-Adressen:"

gsi_mil_scu_adr=$(eb-find $scuname 0x651 0x35aa6b96)

sba="0x$(printf "%X\n" $gsi_mil_scu_adr )"
mil_option_data_adresse="0x$(   printf "%X\n" "$((10#$(($sba+0x1000)) ))" )"
mil_option_ctrl_adresse="0x$(   printf "%X\n" "$((10#$(($sba+0x1004)) ))" )"
mil_option_status_adresse="0x$( printf "%X\n" "$((10#$(($sba+0x1008)) ))" )"

echo "_calculated slotbaseaadr     ="  $sba
echo "_mil_option_data_adresse     ="  $mil_option_data_adresse
echo "_mil_option_ctrl_adresse     ="  $mil_option_ctrl_adresse
echo "_mil_option_status_adresse   ="  $mil_option_status_adresse


scuname="tcp/scuxl0079.acc"

echo "########################Start Devicebustest SCU MIL Macro with data x5A5A  ###############"


status=$(eb-read $scuname ${mil_option_status_adresse}/4)
echo "SIO3 MIL statusregister="$status

data=0x5a5a
echo "write data "$data" to data register "
eb-write $scuname ${mil_option_data_adresse}/4 $data
echo "write function code 13 and ifk address 79 to command register "
eb-write $scuname ${mil_option_ctrl_adresse}/4 0x1379
sleep 1
data=0xbabe
echo "write data "$data" to data register "
eb-write $scuname ${mil_option_data_adresse}/4 $data
echo "write function code 13 and ifk address 79 to command register "
eb-write $scuname ${mil_option_ctrl_adresse}/4 0x1320
