dev="dev/ttyUSB0"
# Packet generator configuration bandwidth (bps) + payload_length (Bytes) 
./src/pkg_gen_conf $dev 100000000 800
# unicast or broadcast. 0x8 des address hb; 0x1c des address lb
#eb-write $dev 0x10008/4 0x00260800
#eb-write $dev 0x1000c/4 0x7b00020a
# Choose packet genrator mode. 0x0 continuous mode;
#eb-write $dev 0x10018/4 0x0
# udp protocol
#eb-write $dev 0x10020/4 0x11

# Enable packet generator
eb-write $dev 0x10000/4 0x1

