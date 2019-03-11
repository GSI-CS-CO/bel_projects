set_global_assignment -entity "ref_pll10" -library "lib_ref_pll10" -name IP_TOOL_NAME "Qsys"
set_global_assignment -entity "ref_pll10" -library "lib_ref_pll10" -name IP_TOOL_VERSION "18.1"
set_global_assignment -entity "ref_pll10" -library "lib_ref_pll10" -name IP_TOOL_ENV "Qsys"
set_global_assignment -library "lib_ref_pll10" -name SPD_FILE [file join $::quartus(sip_path) "ref_pll10.spd"]
set_global_assignment -library "lib_ref_pll10" -name MISC_FILE [file join $::quartus(sip_path) "../ref_pll10.qsys"]

set_global_assignment -library "lib_ref_pll10" -name MISC_FILE [file join $::quartus(sip_path) "sim/ref_pll10.v"]
set_global_assignment -library "lib_ref_pll10" -name MISC_FILE [file join $::quartus(sip_path) "altera_iopll_181/sim/ref_pll10_altera_iopll_181_c5qxdni.vo"]
