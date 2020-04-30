set_global_assignment -entity "stub_pll" -library "lib_stub_pll" -name IP_TOOL_NAME "Qsys"
set_global_assignment -entity "stub_pll" -library "lib_stub_pll" -name IP_TOOL_VERSION "18.1"
set_global_assignment -entity "stub_pll" -library "lib_stub_pll" -name IP_TOOL_ENV "Qsys"
set_global_assignment -library "lib_stub_pll" -name SPD_FILE [file join $::quartus(sip_path) "stub_pll.spd"]
set_global_assignment -library "lib_stub_pll" -name MISC_FILE [file join $::quartus(sip_path) "../stub_pll.qsys"]

set_global_assignment -library "lib_stub_pll" -name MISC_FILE [file join $::quartus(sip_path) "sim/stub_pll.v"]
set_global_assignment -library "lib_stub_pll" -name MISC_FILE [file join $::quartus(sip_path) "altera_iopll_181/sim/stub_pll_altera_iopll_181_arpnjwy.vo"]
