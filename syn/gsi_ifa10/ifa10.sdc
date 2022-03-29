## Generated SDC file "ifa10.sdc"

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

## DATE    "Mon Aug 31 10:54:02 2020"

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

create_clock -name {A_CLK_50MHz} -period 20.000 -waveform { 0.000 10.000 } [get_ports {A_CLK_50MHz}]
create_clock -name {A_EXT_CLK} -period 100.000 -waveform { 0.000 50.000 } [get_ports { A_EXT_CLK }]
#create_clock -name {IFA8_X:Modul|fg:inst20|fkt:2|fktcntrl:59|FKT_DECO:197|RES_DFF} -period 100.000 -waveform { 0.000 50.000 } [get_registers { IFA8_X:Modul|fg:inst20|fkt:2|fktcntrl:59|FKT_DECO:197|RES_DFF }]
create_clock -name {A_CLK_24MHz} -period 41.666 -waveform { 0.000 20.833 } [get_ports {A_CLK_24MHz}]

#**************************************************************
# Create Generated Clock
#**************************************************************

#**************************************************************
# Set Clock Latency
#**************************************************************



#**************************************************************
# Set Clock Uncertainty
#**************************************************************



#**************************************************************
# Set Input Delay
#**************************************************************



#**************************************************************
# Set Output Delay
#**************************************************************



#**************************************************************
# Set Clock Groups
#**************************************************************



#**************************************************************
# Set False Path
#**************************************************************



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

