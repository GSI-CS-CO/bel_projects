set_global_assignment -entity "sys_pll10" -library "lib_sys_pll10" -name IP_TOOL_NAME "Qsys"
set_global_assignment -entity "sys_pll10" -library "lib_sys_pll10" -name IP_TOOL_VERSION "16.0"
set_global_assignment -entity "sys_pll10" -library "lib_sys_pll10" -name IP_TOOL_ENV "Qsys"
set_global_assignment -library "lib_sys_pll10" -name SPD_FILE [file join $::quartus(sip_path) "sys_pll10.spd"]
set_global_assignment -library "lib_sys_pll10" -name MISC_FILE [file join $::quartus(sip_path) "../sys_pll10.qsys"]

set_global_assignment -library "lib_sys_pll10" -name MISC_FILE [file join $::quartus(sip_path) "sim/sys_pll10.vhd"]
set_global_assignment -library "lib_sys_pll10" -name MISC_FILE [file join $::quartus(sip_path) "altera_iopll_160/sim/sys_pll10_altera_iopll_160_pcex3oq.vo"]
