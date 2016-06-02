start_regs=0x200000c0
a=1
b=2000
dev_name=tcp/scuxl0020.acc.gsi.de
while [ $a -le $b ]
  do
eb-write $dev_name $(($start_regs+0x04))/4 0x1
#echo -n "ECA_QUEUE_QUEUE_ID_GET   : "
#eb-read $dev_name $(($start_regs+0x00))/4
#echo -n "ECA_QUEUE_POP_OWR        : "
#eb-read $dev_name $(($start_regs+0x04))/4
#echo -n "ECA_QUEUE_FLAGS_GET      : "
#eb-read $dev_name $(($start_regs+0x08))/4
#echo -n "ECA_QUEUE_NUM_GET        : "
#eb-read $dev_name $(($start_regs+0x0c))/4
#echo -n "ECA_QUEUE_EVENT_ID_HI_GET: "
#eb-read $dev_name $(($start_regs+0x10))/4
#echo -n "ECA_QUEUE_EVENT_ID_LO_GET: "
#eb-read $dev_name $(($start_regs+0x14))/4
#echo -n "ECA_QUEUE_PARAM_HI_GET   : "
#eb-read $dev_name $(($start_regs+0x18))/4
echo -n "ECA_QUEUE_PARAM_LO_GET   : "
eb-read $dev_name $(($start_regs+0x1c))/4
#echo -n "ECA_QUEUE_TAG_GET        : "
#eb-read $dev_name $(($start_regs+0x20))/4
#echo -n "ECA_QUEUE_TEF_GET        : "
#eb-read $dev_name $(($start_regs+0x24))/4
#echo -n "ECA_QUEUE_DEADLINE_HI_GET: "
#eb-read $dev_name $(($start_regs+0x28))/4
#echo -n "ECA_QUEUE_DEADLINE_LO_GET: "
#eb-read $dev_name $(($start_regs+0x2c))/4
#echo -n "ECA_QUEUE_EXECUTED_HI_GET: "
#eb-read $dev_name $(($start_regs+0x30))/4
#echo -n "ECA_QUEUE_EXECUTED_LO_GET: "
#eb-read $dev_name $(($start_regs+0x34))/4
a=`expr $a + 1`
  done
