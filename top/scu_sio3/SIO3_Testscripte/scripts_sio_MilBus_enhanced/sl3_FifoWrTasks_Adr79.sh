
#!/bin/bash
#Script for Write some Data using TX_FIFO to IFK

clear



clear
scunummer=0079
scuname="tcp/scuxl0079.acc"

echo "Per eb-find errechnete Wishbone-Adressen:"
scuslave_baseadr=$(eb-find tcp/scuxl$scunummer.acc 0x651 0x9602eb6f)
echo "scuslave_baseadr            =" ${scuslave_baseadr}

for ((i=3;i<=3;i++)) do
#  calculatedslotaddress="$((10#$(($scuslave_baseadr+0x20000*i+0x20)) ))"
  slotbaseaadr="$((10#$(($scuslave_baseadr+0x20000*i)) ))"
  mil_option_data_adresse="$((10#$(($slotbaseaadr+0x20000*i+0x800)) ))"
  slavebase[i]="0x$( printf "%X\n" $slotbaseaadr )"
  sub="0x$(printf "%X\n" $slotbaseaadr )"

  slavebase[i]="0x$( printf "%X\n" $slotbaseaadr )"
done

sba="0x$(printf "%X\n" $slotbaseaadr )"
mil_option_data_adresse="0x$( printf "%X\n" "$((10#$(($sba+0x800)) ))" )"
mil_option_ctrl_adresse="0x$( printf "%X\n" "$((10#$(($sba+0x802)) ))" )"
mil_option_status_adresse="0x$( printf "%X\n" "$((10#$(($sba+0x804)) ))" )"

echo "calculated slotbaseaadr     ="  $sba
echo "mil_option_data_adresse     ="  $mil_option_data_adresse
echo "mil_option_ctrl_adresse     ="  $mil_option_ctrl_adresse
echo "mil_option_status_adresse   ="  $mil_option_status_adresse


scuname="tcp/scuxl0079.acc"

echo "########################Start Devicebustest SIO 3 with data x5A5A  ###############"


status=$(eb-read $scuname ${mil_option_status_adresse}/2)
echo "SIO3 MIL statusregister="$status

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
