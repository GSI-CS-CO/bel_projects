#!/bin/ash

killall saft-fg-ctl
# clear PUR on mil extension
#eb-write dev/wbm0 0x9000/4 0x100
#eb-write dev/wbm0 0x9004/4 0x12ff
# clear PUR on sio 3 
eb-write dev/wbm0 0x420800/2 0x100
eb-write dev/wbm0 0x420802/2 0x12ff
# clear PUR on sio 4
#eb-write dev/wbm0 0x580800/2 0x100
#eb-write dev/wbm0 0x580802/2 0x12ff

saft-fg-ctl -f fg-33-1 < ramp_ifa_scu.txt &
saft-fg-ctl -f fg-33-2 < ramp_ifa_scu.txt &
saft-fg-ctl -f fg-33-3 < ramp_ifa_scu.txt &
saft-fg-ctl -f fg-33-129 < ramp_ifa_scu.txt & 
saft-fg-ctl -f fg-33-131 < ramp_ifa_scu.txt & 
#saft-fg-ctl -f fg-35-141 < ramp_ifa_scu.txt & 
#saft-fg-ctl -f fg-34-1 < ramp_ifa.txt &
#saft-fg-ctl -f fg-34-33 < ramp_ifa.txt &
#saft-fg-ctl -f fg-34-34 < ramp_ifa.txt &
#saft-fg-ctl -f fg-34-35 < ramp_ifa.txt & 

#saft-fg-ctl -f fg-36-1 < ramp_ifa.txt &
#saft-fg-ctl -f fg-36-33 < ramp_ifa.txt &
#saft-fg-ctl -f fg-36-34 < ramp_ifa.txt &
#saft-fg-ctl -f fg-36-35 < ramp_ifa.txt & 

#saft-fg-ctl -f fg-41-1 < ramp4.txt &
#saft-fg-ctl -f fg-41-33 < ramp3.txt &
#saft-fg-ctl -f fg-41-34 < ramp2.txt &
#saft-fg-ctl -f fg-41-35 < ramp1.txt &

#saft-fg-ctl -f fg-44-80 < ramp_ifa_scu.txt &
#saft-fg-ctl -f fg-44-82 < ramp_ifa_scu.txt &
#saft-fg-ctl -f fg-44-83 < ramp_ifa_scu.txt &
#saft-fg-ctl -f fg-44-84 < ramp_ifa_scu.txt &

#saft-fg-ctl -f fg-42-10 < ramp4.txt &
#saft-fg-ctl -f fg-42-12 < ramp2.txt &
#saft-fg-ctl -f fg-44-85 < ramp_ifa_scu.txt &
#saft-fg-ctl -f fg-44-86 < ramp_ifa_scu.txt &

#saft-fg-ctl -f fg-2-0 -e 0xcafebabe < ramp_ifa_scu.txt &
#saft-fg-ctl -f fg-2-1 -e 0xcafebabe < ramp_ifa_scu.txt &
##saft-fg-ctl -f fg-3-0 -e 0xcafebabe < ramp_ifa_scu.txt &
#saft-fg-ctl -f fg-3-1 -e 0xcafebabe < ramp_ifa_scu.txt &
#saft-fg-ctl -f fg-4-0 -e 0xcafebabe < ramp1.txt &
#saft-fg-ctl -f fg-4-1 -e 0xcafebabe < ramp2.txt &

cat ramp1.txt | shuf > temp1.txt
cat ramp2.txt | shuf > temp2.txt
cat ramp3.txt | shuf > temp3.txt
cat ramp4.txt | shuf > temp4.txt

mv temp1.txt ramp1.txt
mv temp2.txt ramp2.txt
mv temp3.txt ramp3.txt
mv temp4.txt ramp4.txt
# give time for setup of fgs before the trigger arrives
sleep 1
# create action for lm32
saft-ecpu-ctl tr0 -x
saft-ecpu-ctl tr0 -c 0xcafebabe 0xffffffffffffffff 0x0 0xdeadbeef -d
# create a 10ms pulse on output B1
# delete old rules; enable output; set output high; set output low after 10000ns
saft-io-ctl tr0 -x
saft-io-ctl tr0 -n B1 -o1
saft-io-ctl tr0 -n B1 -c 0xcafebabe 0xffffffffffffffffffff 0 0x0 0x1 -u
saft-io-ctl tr0 -n B1 -c 0xcafebabe 0xffffffffffffffffffff 10000 0x0 0x0 -u
# inject event
saft-ctl tr0 inject 0xcafebabe 0xffffffffffffffff 10000 -p
