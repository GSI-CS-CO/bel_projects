# xwb_dpram attributes

Addressing: Inferring from ip_cores/general-cores/testbench/wishbone/lm32_testsys/sw/target/lm32/ram.ld and ip_cores/general-cores/testbench/wishbone/lm32_testsys/lm32_test_system.vhd g_size is the memory size in words. One word is 32 bit or 4 byte.

The module needs to be instantiated as byte addressable to be able to use all of the memory. Otherwise only the words on sums of 4 can be read because the etherbone tool is written as 2 byte addressable.

The two ports don’t seem to behave identically.