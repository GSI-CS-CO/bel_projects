
#!/bin/bash
#Script for Testing Transmission using IFK Echo Register Response 

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
echo "calculated slotbaseaadr     =" $sba

mil_option_data_adresse="0x$( printf "%X\n" "$((10#$(($sba+0x800)) ))" )"
mil_option_ctrl_adresse="0x$( printf "%X\n" "$((10#$(($sba+0x802)) ))" )"
mil_option_status_adresse="0x$( printf "%X\n" "$((10#$(($sba+0x804)) ))" )"

echo "calculated slotbaseaadr     ="  $sba
echo "mil_option_data_adresse     ="  $mil_option_data_adresse
echo "mil_option_ctrl_adresse     ="  $mil_option_ctrl_adresse
echo "mil_option_status_adresse   ="  $mil_option_status_adresse


[ -z $BASH ] || shopt -s expand_aliases
alias BEGINCOMMENT="if [ ]; then"
alias ENDCOMMENT="fi"

#########################################################################################
# tx_taskram_adr Offset fuer erste  Zelle xC01 = x1802 (SIO)   = x3004 (MilOption)
# tx_taskram_adr Offset fuer 255.   Zelle xCFF = x19FE (SIO)   = x33FC (MilOption)
# Z.B. tx_taskram_adr Zelle 1: SCU MIL x20008000(Basis) + x3004 (Offset)= x2000B004 
# rx_taskram_adr Offset fuer erste  Zelle xD01 = x1A02 (SIO)   = x3404 (MilOption)
# rx_taskram_adr Offset fuer 255.   Zelle xDFF = x1BFE (SIO)   = x37FC (MilOption)
#
# stat_avail_adr Offset fuer erste  Zelle xE00 = x1C00 (SIO)   = x3800 (MilOption) 
# stat_avail_adr Offset fuer 16.    Zelle xE0F = x1C1E (SIO)   = x383C (MilOption)
#
# rx_err_adr     Offset fuer erste  Zelle xE10 = x1C20 (SIO)   = x3840 (MilOption) 
# rx_err_adr     Offset fuer 16.    Zelle xE1F = x1C3E (SIO)   = x387C (MilOption)
#
# tx_req_adr     Offset fuer erste  Zelle xE20 = x1C40 (SIO)   = x3880 (MilOption) 
# tx_req_adr     Offset fuer 16.    Zelle xE2F = x1C5E (SIO)   = x38BC (MilOption)
#########################################################################################

for ((i=1;i<=10;i++)) do
  offset="0x$( printf "%X\n" "$((i*2))" )"
  tx_taskram_adr[i]="0x$( printf "%X\n" "$((10#$(($sba+0x1800+$offset )) ))" )"
  echo "tx_taskram_adr["$i"]=" ${tx_taskram_adr[i]}
  rx_taskram_adr[i]="0x$( printf "%X\n" "$((10#$(($sba+0x1a00+$offset )) ))" )"
  echo "rx_taskram_adr["$i"]=" ${rx_taskram_adr[i]}
done

for ((i=0;i<=15;i++)) do
  offset="0x$( printf "%X\n" "$((i*2))" )"
  rd_stat_avail_adr[i]="0x$( printf "%X\n" "$((10#$(($sba+0x1c00+$offset )) ))" )"
  echo "rd_stat_avail_adr["$i"]=" ${rd_stat_avail_adr[i]}

done

for ((i=0;i<=15;i++)) do
  offset="0x$( printf "%X\n" "$((i*2))" )"
  rd_err_adr[i]="0x$( printf "%X\n" "$((10#$(($sba+0x1c20+$offset )) ))" )"
  echo "rd_err_adr["$i"]=" ${rd_err_adr[i]}

done

for ((i=0;i<=15;i++)) do
  offset="0x$( printf "%X\n" "$((i*2))" )"
  tx_req_adr[i]="0x$( printf "%X\n" "$((10#$(($sba+0x1c40+$offset )) ))" )"
  echo "tx_req_adr["$i"]=" ${tx_req_adr[i]}

done


data1=0x8979  
data2=0x8920
data3=0xcaf3
data4=0xcaf4
data5=0xcaf5
data6=0xcaf6
data7=0xcaf7
data8=0xcaf8
data9=0xcaf9
data10=0xca10


echo "########################Start Devicebustest SIO3 ###########################"
echo "Mit CMD x8979 lese Echoregister aus DevBus Slave (Adr.79)"
echo "Mit CMD x8920 lese Echoregister aus DevBus Slave (Adr.20)"
echo "Diese Register waren vorher mit x5a5a bzw xbabe geladen"
echo "----------------------------------------------------------------------------"

echo "write 2 Commands to tx_taskram1 and tx_taskram 2 register "
eb-write $scuname ${tx_taskram_adr[1]}/2 $data1
eb-write $scuname ${tx_taskram_adr[2]}/2 $data2
echo "----------------------------------------------------------------------------"

echo "tx_req_reg 1             =" $(eb-read  $scuname ${tx_req_adr[0]}/2)
echo "rx_status_avail_reg 1    =" $(eb-read  $scuname ${rd_stat_avail_adr[0]}/2)
echo "rx_err_reg 1             =" $(eb-read  $scuname ${rd_err_adr[0]}/2)
echo "----------------------------------------------------------------------------"
echo "read data from taskram 1 =" $(eb-read $scuname ${rx_taskram_adr[1]}/2) 
echo "----------------------------------------------------------------------------"
echo "tx_req_reg 1             =" $(eb-read  $scuname ${tx_req_adr[0]}/2)
echo "rx_status_avail_reg 1    =" $(eb-read  $scuname ${rd_stat_avail_adr[0]}/2)
echo "rx_err_reg 1             =" $(eb-read  $scuname ${rd_err_adr[0]}/2)
echo "----------------------------------------------------------------------------"
echo "read data from taskram 2 =" $(eb-read $scuname ${rx_taskram_adr[2]}/2)
echo "----------------------------------------------------------------------------"
echo "tx_req_reg 1             =" $(eb-read  $scuname ${tx_req_adr[0]}/2)
echo "rx_status_avail_reg 1    =" $(eb-read  $scuname ${rd_stat_avail_adr[0]}/2)
echo "rx_err_reg 1             =" $(eb-read  $scuname ${rd_err_adr[0]}/2)
echo "########################Ende Devicebustest SIO3 ###########################"
