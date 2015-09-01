# automatically control the packet generator
dev="dev/ttyUSB0"
date=`date +%Y-%m-%d:%H:%M`
log_file="./log_file$date.log"
basewidth="100000000"
basepay="200"

for i in 1 2 3 4 5 6 7 8 
do
  for j in 1 2 3 4 5 6 7  
  do
    bandwidth=$i*$basewidth
    payload=$j*$basepay
    #echo $((bandwidth))
    sudo ./pkg_gen_conf $dev $((bandwidth)) $((payload))
    sudo eb-write $dev 0x10000/4 0x1
    # sleep h(hour) m(minute) s(second) d(day) 
    sleep 1s
    sudo eb-write $dev 0x10000/4 0x0
    echo "The counter for the bandwidth of $((bandwidth)) and payload of
    $((payload))" >> $log_file
    sudo eb-read $dev 0x1001c/4 >>$log_file
  done
done
