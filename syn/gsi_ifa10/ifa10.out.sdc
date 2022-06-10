## Generated SDC file "ifa10.out.sdc"

## Copyright (C) 2018  Intel Corporation. All rights reserved.
## Your use of Intel Corporation's design tools, logic functions 
## and other software and tools, and its AMPP partner logic 
## functions, and any output files from any of the foregoing 
## (including device programming or simulation files), and any 
## associated documentation or information are expressly subject 
## to the terms and conditions of the Intel Program License 
## Subscription Agreement, the Intel Quartus Prime License Agreement,
## the Intel FPGA IP License Agreement, or other applicable license
## agreement, including, without limitation, that your use is for
## the sole purpose of programming logic devices manufactured by
## Intel and sold by Intel or its authorized distributors.  Please
## refer to the applicable agreement for further details.


## VENDOR  "Altera"
## PROGRAM "Quartus Prime"
## VERSION "Version 18.1.0 Build 625 09/12/2018 SJ Lite Edition"

## DATE    "Mon May 23 13:48:47 2022"

##
## DEVICE  "10M50DAF672C7G"
##


#**************************************************************
# Time Information
#**************************************************************

set_time_format -unit ns -decimal_places 3



#**************************************************************
# Create Clock
#**************************************************************

create_clock -name {altera_reserved_tck} -period 100.000 -waveform { 0.000 50.000 } [get_ports {altera_reserved_tck}]
create_clock -name {A_CLK_50MHz} -period 20.000 -waveform { 0.000 10.000 } [get_ports {A_CLK_50MHz}]
create_clock -name {A_EXT_CLK} -period 100.000 -waveform { 0.000 50.000 } [get_ports { A_EXT_CLK }]
create_clock -name {A_CLK_24MHz} -period 41.666 -waveform { 0.000 20.833 } [get_ports {A_CLK_24MHz}]


#**************************************************************
# Create Generated Clock
#**************************************************************

create_generated_clock -name {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]} -source [get_pins {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|inclk[0]}] -duty_cycle 50/1 -multiply_by 1 -divide_by 2 -master_clock {A_CLK_24MHz} [get_pins {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}] 
create_generated_clock -name {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]} -source [get_pins {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|inclk[0]}] -duty_cycle 50/1 -multiply_by 5 -master_clock {A_CLK_24MHz} [get_pins {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]}] 
create_generated_clock -name {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]} -source [get_pins {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|inclk[0]}] -duty_cycle 50/1 -multiply_by 1 -master_clock {A_CLK_24MHz} [get_pins {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}] 


#**************************************************************
# Set Clock Latency
#**************************************************************



#**************************************************************
# Set Clock Uncertainty
#**************************************************************

set_clock_uncertainty -rise_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}] -rise_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}] -fall_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}] -rise_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}] -fall_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}] -rise_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}] -fall_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}] -rise_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]}]  0.020  
set_clock_uncertainty -fall_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}] -fall_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]}]  0.020  
set_clock_uncertainty -rise_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}] -rise_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]}]  0.010  
set_clock_uncertainty -rise_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}] -fall_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]}]  0.010  
set_clock_uncertainty -rise_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}] -rise_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000  
set_clock_uncertainty -rise_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}] -fall_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000  
set_clock_uncertainty -fall_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}] -rise_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]}]  0.010  
set_clock_uncertainty -fall_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}] -fall_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]}]  0.010  
set_clock_uncertainty -fall_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}] -rise_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000  
set_clock_uncertainty -fall_from [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}] -fall_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000  
set_clock_uncertainty -rise_from [get_clocks {A_EXT_CLK}] -rise_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}] -setup 0.000  
set_clock_uncertainty -rise_from [get_clocks {A_EXT_CLK}] -fall_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}] -setup 0.000  
set_clock_uncertainty -rise_from [get_clocks {A_EXT_CLK}] -rise_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]}] -setup 0.000  
set_clock_uncertainty -rise_from [get_clocks {A_EXT_CLK}] -fall_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]}] -setup 0.000  
set_clock_uncertainty -rise_from [get_clocks {A_EXT_CLK}] -rise_to [get_clocks {A_CLK_24MHz}]  0.000  
set_clock_uncertainty -rise_from [get_clocks {A_EXT_CLK}] -fall_to [get_clocks {A_CLK_24MHz}]  0.000  
set_clock_uncertainty -rise_from [get_clocks {A_EXT_CLK}] -rise_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}] -setup 0.000  
set_clock_uncertainty -rise_from [get_clocks {A_EXT_CLK}] -fall_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}] -setup 0.000  
set_clock_uncertainty -rise_from [get_clocks {A_EXT_CLK}] -rise_to [get_clocks {A_EXT_CLK}] -setup 0.000  
set_clock_uncertainty -rise_from [get_clocks {A_EXT_CLK}] -rise_to [get_clocks {A_EXT_CLK}] -hold 0.020  
set_clock_uncertainty -rise_from [get_clocks {A_EXT_CLK}] -fall_to [get_clocks {A_EXT_CLK}] -setup 0.000  
set_clock_uncertainty -rise_from [get_clocks {A_EXT_CLK}] -fall_to [get_clocks {A_EXT_CLK}] -hold 0.020  
set_clock_uncertainty -rise_from [get_clocks {A_EXT_CLK}] -rise_to [get_clocks {A_CLK_50MHz}] -setup 0.000  
set_clock_uncertainty -rise_from [get_clocks {A_EXT_CLK}] -fall_to [get_clocks {A_CLK_50MHz}] -setup 0.000  
set_clock_uncertainty -fall_from [get_clocks {A_EXT_CLK}] -rise_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}] -setup 0.000  
set_clock_uncertainty -fall_from [get_clocks {A_EXT_CLK}] -fall_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[1]}] -setup 0.000  
set_clock_uncertainty -fall_from [get_clocks {A_EXT_CLK}] -rise_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]}] -setup 0.000  
set_clock_uncertainty -fall_from [get_clocks {A_EXT_CLK}] -fall_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]}] -setup 0.000  
set_clock_uncertainty -fall_from [get_clocks {A_EXT_CLK}] -rise_to [get_clocks {A_CLK_24MHz}]  0.000  
set_clock_uncertainty -fall_from [get_clocks {A_EXT_CLK}] -fall_to [get_clocks {A_CLK_24MHz}]  0.000  
set_clock_uncertainty -fall_from [get_clocks {A_EXT_CLK}] -rise_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}] -setup 0.000  
set_clock_uncertainty -fall_from [get_clocks {A_EXT_CLK}] -fall_to [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}] -setup 0.000  
set_clock_uncertainty -fall_from [get_clocks {A_EXT_CLK}] -rise_to [get_clocks {A_EXT_CLK}] -setup 0.000  
set_clock_uncertainty -fall_from [get_clocks {A_EXT_CLK}] -rise_to [get_clocks {A_EXT_CLK}] -hold 0.020  
set_clock_uncertainty -fall_from [get_clocks {A_EXT_CLK}] -fall_to [get_clocks {A_EXT_CLK}] -setup 0.000  
set_clock_uncertainty -fall_from [get_clocks {A_EXT_CLK}] -fall_to [get_clocks {A_EXT_CLK}] -hold 0.020  
set_clock_uncertainty -fall_from [get_clocks {A_EXT_CLK}] -rise_to [get_clocks {A_CLK_50MHz}] -setup 0.000  
set_clock_uncertainty -fall_from [get_clocks {A_EXT_CLK}] -fall_to [get_clocks {A_CLK_50MHz}] -setup 0.000  


#**************************************************************
# Set Input Delay
#**************************************************************

set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_CLK_24MHz}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_CLK_50MHz}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_EXT_CLK}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_CDS}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_DSC}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_ESC}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_SD}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_SDO}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_TD}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_VW}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_nBOO}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_nBZO}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_MIL1_BOI}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_MIL1_BZI}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[0]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[1]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[2]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[3]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[4]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[5]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[6]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[7]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[8]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[9]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[10]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[11]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[12]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[13]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[14]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[15]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[16]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[17]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[18]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[19]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[20]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[21]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[22]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[23]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[24]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[25]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[26]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[27]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[28]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[29]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[30]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[31]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[32]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[33]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[34]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[35]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[36]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[37]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[38]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[39]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[40]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[41]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[42]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[43]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[44]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[45]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_SEL_B[0]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_SEL_B[1]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_SEL_B[2]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_SEL_B[3]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_UMIL5V}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_UMIL15V}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_WAKE}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nMANUAL_RES}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nSEL_6408}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[0]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[1]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[2]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[3]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[4]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[5]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[6]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[7]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[8]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[9]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[10]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[11]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[12]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[13]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[14]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[15]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_28}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_29}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_30}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_31}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_32}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_33}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_34}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_35}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_ADR[0]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_ADR[1]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_ADR[2]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_ADR[3]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_ADR[4]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_ADR[5]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_ADR[6]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_ADR[7]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_ADR[8]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_ADR[9]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_ADR[10]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS_ADR[11]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {FS_DATA}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {IN_NDRQ}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {IN_NINL}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {IN_NRDY}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {LEMO_IN}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {MCU_NMI}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {MCU_RESET_FILTER}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {RDN}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {RUP}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[3]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[4]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[5]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[6]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[7]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[8]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[9]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[10]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[11]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[12]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[13]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[14]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[15]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[16]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[17]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[18]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[19]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[20]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[21]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[22]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[23]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[24]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[25]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[26]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[27]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[28]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[29]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[30]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[31]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[1]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[2]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[3]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[4]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[5]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[6]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[7]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[8]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[9]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[10]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[11]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[12]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[13]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[14]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[15]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[16]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[17]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[18]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[19]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[20]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[21]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[22]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[23]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[24]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[25]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[26]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[27]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[28]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[29]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[30]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[31]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[32]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[1]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[2]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[3]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[4]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[5]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[6]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[7]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[8]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[9]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[10]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[11]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[12]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[13]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[14]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[15]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[16]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[17]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[18]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[19]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[20]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[21]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[22]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[23]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[24]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[25]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[26]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[27]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[28]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[29]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[30]}]
set_input_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[31]}]


#**************************************************************
# Set Output Delay
#**************************************************************

set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_AC19_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_AC22_AC25_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_AC26_AC27_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_AC28_AC31_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_B18_B24_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_C20_21_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_IO1_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_I_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_I_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_K_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_K_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_L_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_L_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_12MHZ}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_BOI}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_BZI}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_EE}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_SDI}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_SS}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_ME_UDI}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_MIL1_OUT_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_MIL1_nBOO}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_MIL1_nBZO}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_MIL1_nIN_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_M_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_M_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_NLED_EVT_INR}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_N_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_N_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[0]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[1]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[2]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[3]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[4]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[5]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[6]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[7]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[8]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[9]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[10]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[11]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[12]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[13]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[14]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[15]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[16]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[17]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[18]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[19]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[20]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[21]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[22]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[23]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[24]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[25]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[26]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[27]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[28]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[29]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[30]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[31]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[32]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[33]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[34]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[35]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[36]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[37]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[38]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[39]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[40]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[41]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[42]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[43]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[44]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_P[45]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[0]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[1]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[2]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[3]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[4]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[5]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[6]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[7]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[8]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[9]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[10]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[11]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[12]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[13]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[14]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_Test[15]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X1_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X1_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X2_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X2_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X3_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X3_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X4_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X4_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X5_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X5_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X6_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X6_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X7_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X7_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X8_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X8_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X9_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X9_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X10_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X10_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X11_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X11_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X12_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_X12_Out}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nAC19_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nAC22_AC25_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nAC26_AC27_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nAC28_AC31_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nB18_B24_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nC20_21_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nIO1_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[0]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[1]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[2]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[3]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[4]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[5]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[6]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[7]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[8]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[9]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[10]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[11]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[12]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[13]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[14]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[15]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[16]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[17]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[18]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[19]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[20]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[21]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED[22]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nLED_EXTCLK}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nOPK_DRDY}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nOPK_DRQ}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nOPK_INL}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {A_nSWITCH_Ena}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[0]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[1]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[2]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[3]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[4]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[5]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[6]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[7]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[8]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[9]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[10]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[11]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[12]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[13]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[14]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {EPIOS[15]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {FS_ASDI}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {FS_DCLK}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {FS_nCS}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {LEMO_OUT}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {OE_Test}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {SHOW_CONFIG}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {SPARE[1]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {SPARE[2]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {SPARE[3]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {SPARE[4]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {SPARE[5]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {SPARE[6]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {SPARE[7]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {SPARE[8]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {SPARE[9]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {SPARE[10]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {SPARE[11]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[3]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[4]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[5]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[6]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[7]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[8]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[9]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[10]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[11]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[12]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[13]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[14]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[15]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[16]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[17]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[18]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[19]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[20]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[21]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[22]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[23]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[24]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[25]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[26]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[27]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[28]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[29]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[30]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_A[31]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[1]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[2]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[3]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[4]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[5]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[6]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[7]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[8]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[9]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[10]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[11]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[12]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[13]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[14]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[15]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[16]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[17]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[18]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[19]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[20]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[21]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[22]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[23]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[24]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[25]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[26]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[27]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[28]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[29]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[30]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[31]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_B[32]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[1]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[2]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[3]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[4]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[5]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[6]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[7]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[8]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[9]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[10]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[11]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[12]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[13]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[14]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[15]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[16]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[17]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[18]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[19]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[20]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[21]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[22]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[23]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[24]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[25]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[26]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[27]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[28]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[29]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[30]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {V_C[31]}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {nFAILSAVE_LD_LED}]
set_output_delay -add_delay  -clock [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[2]}]  0.000 [get_ports {nUSER_LD_LED}]


#**************************************************************
# Set Clock Groups
#**************************************************************

set_clock_groups -asynchronous -group [get_clocks {altera_reserved_tck}] 


#**************************************************************
# Set False Path
#**************************************************************

set_false_path  -from  [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]}]  -to  [get_clocks *]
set_false_path  -from  [get_clocks {A_EXT_CLK}]  -to  [get_clocks {ClkTimer|PLL_1_inst|altpll_component|auto_generated|pll1|clk[0]}]


#**************************************************************
# Set Multicycle Path
#**************************************************************



#**************************************************************
# Set Maximum Delay
#**************************************************************



#**************************************************************
# Set Minimum Delay
#**************************************************************



#**************************************************************
# Set Input Transition
#**************************************************************

