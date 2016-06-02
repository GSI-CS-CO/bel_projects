a=1
b=105
while [ $a -le $b ]
  do
    echo $a
    saft-ctl baseboard inject 0xdeadbeef11111111 $a 0
    a=`expr $a + 1`
  done
