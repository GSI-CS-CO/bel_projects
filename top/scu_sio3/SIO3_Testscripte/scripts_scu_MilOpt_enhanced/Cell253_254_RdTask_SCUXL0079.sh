
#!/bin/bash
#Script for Testing Transmission using IFK Echo Register Response 

clear
scunummer=0079
scuname="tcp/scuxl0079.acc"


echo "Per eb-find errechnete Wishbone-Adressen:"

gsi_mil_scu_adr=$(eb-find $scuname 0x651 0x35aa6b96)

sba="0x$(printf "%X\n" $gsi_mil_scu_adr )"
mil_option_data_adresse="0x$(   printf "%X\n" "$((10#$(($sba+0x1000)) ))" )"
mil_option_ctrl_adresse="0x$(   printf "%X\n" "$((10#$(($sba+0x1004)) ))" )"
mil_option_status_adresse="0x$( printf "%X\n" "$((10#$(($sba+0x1008)) ))" )"

echo "calculated slotbaseaadr     ="  $sba
echo "mil_option_data_adresse     ="  $mil_option_data_adresse
echo "mil_option_ctrl_adresse     ="  $mil_option_ctrl_adresse
echo "mil_option_status_adresse   ="  $mil_option_status_adresse

# For using Aliases BEGINCOMMENT ENDCOMMENT
[ -z $BASH ] || shopt -s expand_aliases
alias BEGINCOMMENT="if [ ]; then"
alias ENDCOMMENT="fi"

#########################################################################################
# tx_taskram_adr vorher 501==A02,  jetzt c01==1802
# rx_taskram_adr vorher 601==C02,  jetzt d01==1a02

# Calculation TX_TASKRAM ADRESSES
for ((i=1;i<=254;i++)) do
  offset="0x$( printf "%X\n" "$((i*4))" )"
  tx_taskram_adr[i]="0x$( printf "%X\n" "$((10#$(($sba+0x3000+$offset )) ))" )"
  echo "tx_taskram_adr["$i"]=" ${tx_taskram_adr[i]}
done

# Calculation RX_TASKRAM ADRESSES
for ((i=1;i<=254;i++)) do
  offset="0x$( printf "%X\n" "$((i*4))" )"
  rx_taskram_adr[i]="0x$( printf "%X\n" "$((10#$(($sba+0x3400+$offset )) ))" )"
  echo "rx_taskram_adr["$i"]=" ${rx_taskram_adr[i]}
done

# Calculation rd_stat_avail, rd_err, tx_req adresses

for ((i=0;i<=15;i++)) do
  offset="0x$( printf "%X\n" "$((i*4))" )"
  rd_stat_avail_adr[i]="0x$( printf "%X\n" "$((10#$(($sba+0x3800+$offset )) ))" )"
  echo "rd_stat_avail_adr["$i"]=" ${rd_stat_avail_adr[i]}

done

for ((i=0;i<=15;i++)) do
  offset="0x$( printf "%X\n" "$((i*4))" )"
  rd_err_adr[i]="0x$( printf "%X\n" "$((10#$(($sba+0x3840+$offset )) ))" )"
  echo "rd_err_adr["$i"]=" ${rd_err_adr[i]}

done

for ((i=0;i<=15;i++)) do
  offset="0x$( printf "%X\n" "$((i*4))" )"
  tx_req_adr[i]="0x$( printf "%X\n" "$((10#$(($sba+0x3880+$offset )) ))" )"
  echo "tx_req_adr["$i"]=" ${tx_req_adr[i]}

done
#######################################################################################
#fc 89 : Echoregister rücklesen von Adresse 79
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



echo "########################Start Devicebustest SIO3 with data x5A5A  ###############"

echo "--------------------------------------------------"

echo "write 2 Echo Functioncodes to tx_taskram253 and tx_taskram254"
echo "for readback (Task253:x5a5a from addr79, Task254:xbabe from addr20)"

eb-write $scuname ${tx_taskram_adr[230]}/4 $data1
eb-write $scuname ${tx_taskram_adr[231]}/4 $data1
eb-write $scuname ${tx_taskram_adr[232]}/4 $data1
eb-write $scuname ${tx_taskram_adr[233]}/4 $data1
eb-write $scuname ${tx_taskram_adr[234]}/4 $data1
eb-write $scuname ${tx_taskram_adr[235]}/4 $data1
eb-write $scuname ${tx_taskram_adr[236]}/4 $data1
eb-write $scuname ${tx_taskram_adr[237]}/4 $data1
eb-write $scuname ${tx_taskram_adr[238]}/4 $data1
eb-write $scuname ${tx_taskram_adr[239]}/4 $data1


eb-write $scuname ${tx_taskram_adr[241]}/4 $data1
eb-write $scuname ${tx_taskram_adr[242]}/4 $data1
eb-write $scuname ${tx_taskram_adr[243]}/4 $data1
eb-write $scuname ${tx_taskram_adr[244]}/4 $data1
eb-write $scuname ${tx_taskram_adr[245]}/4 $data1
eb-write $scuname ${tx_taskram_adr[246]}/4 $data2
eb-write $scuname ${tx_taskram_adr[247]}/4 $data1
eb-write $scuname ${tx_taskram_adr[248]}/4 $data2

eb-write $scuname ${tx_taskram_adr[249]}/4 $data1
eb-write $scuname ${tx_taskram_adr[250]}/4 $data2

eb-write $scuname ${tx_taskram_adr[251]}/4 $data1
eb-write $scuname ${tx_taskram_adr[252]}/4 $data2

eb-write $scuname ${tx_taskram_adr[253]}/4 $data1
eb-write $scuname ${tx_taskram_adr[254]}/4 $data2




eb-write $scuname ${tx_taskram_adr[253]}/4 $data1
eb-write $scuname ${tx_taskram_adr[254]}/4 $data2

#echo "Now check for request/error/avail bits"

echo "tx_req_reg 15            =" $(eb-read  $scuname ${tx_req_adr[15]}/4)

BEGINCOMMENT
echo "rx_status_avail_reg 15   =" $(eb-read  $scuname ${rd_stat_avail_adr[15]}/4)
echo "rx_err_reg 15            =" $(eb-read  $scuname ${rd_err_adr[15]}/4)

echo "Now read first data back"
echo "read data from taskram253=" $(eb-read $scuname ${rx_taskram_adr[253]}/4) 



echo "Now check for request/error/avail bits"
echo "tx_req_reg 15            =" $(eb-read  $scuname ${tx_req_adr[15]}/4)
echo "rx_status_avail_reg 15   =" $(eb-read  $scuname ${rd_stat_avail_adr[15]}/4)
echo "rx_err_reg 15            =" $(eb-read  $scuname ${rd_err_adr[15]}/4)
echo "Now read second data back"
echo "read data from taskram254=" $(eb-read  $scuname ${rx_taskram_adr[254]}/4)

echo "Now check for request/error/avail bits"
echo "tx_req_reg 15            =" $(eb-read  $scuname ${tx_req_adr[15]}/4)
echo "rx_status_avail_reg 15   =" $(eb-read  $scuname ${rd_stat_avail_adr[15]}/4)
echo "rx_err_reg 15            =" $(eb-read  $scuname ${rd_err_adr[15]}/4)

echo "ende"




echo "read data from taskram253=" $(eb-read  $scuname ${rx_taskram_adr[1]}/4) 
echo "read data from taskram254=" $(eb-read  $scuname ${rx_taskram_adr[2]}/4)


echo "ende"

ENDCOMMENT
