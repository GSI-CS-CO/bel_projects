DEV=$1
TR=$2
DOT=$3

dm-cmd tcp/$DEV halt
sleep 0.3
dm-sched tcp/$DEV clear
dm-sched tcp/$DEV add $DOT
dm-sched tcp/$DEV -s dump
ADR=`dm-sched tcp/$DEV | awk '/50Hz_INBOX/ {print $NF}'`
echo $ADR
eb-write tcp/$DEV $ADR/4 0x1300000
eb-read tcp/$DEV $ADR/4
ssh root@$DEV "echo 0x14c0fc0000000000 0x1312d00 0 > unilac_20.txt; saft-wbm-ctl $TR -x; saft-wbm-ctl $TR -r 3 $ADR 0 0x5f; saft-wbm-ctl $TR -c 0x14c0fc0000000000 0xffffff0000000000 0 3 -d; sleep 1; saft-dm $TR -p -v unilac_20.txt"
#ssh root@$DEV "echo \"0x14c0fc0000000000 0x1312d00 0\" > unilac_20.txt; sleep 0.1; saft-dm $TR -p -v unilac_20.txt"
sleep 0.3
eb-read tcp/$DEV $ADR/4
echo "Starting unilac master pattern (main loop)"
dm-cmd tcp/$DEV startpattern unilacMaster_V9