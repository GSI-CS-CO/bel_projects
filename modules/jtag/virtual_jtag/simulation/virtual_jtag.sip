set_global_assignment -entity "virtual_jtag" -library "lib_virtual_jtag" -name IP_TOOL_NAME "Qsys"
set_global_assignment -entity "virtual_jtag" -library "lib_virtual_jtag" -name IP_TOOL_VERSION "18.1"
set_global_assignment -entity "virtual_jtag" -library "lib_virtual_jtag" -name IP_TOOL_ENV "Qsys"
set_global_assignment -library "lib_virtual_jtag" -name SPD_FILE [file join $::quartus(sip_path) "../virtual_jtag.spd"]
set_global_assignment -library "lib_virtual_jtag" -name MISC_FILE [file join $::quartus(sip_path) "../../virtual_jtag.qsys"]

set_global_assignment -library "lib_virtual_jtag" -name MISC_FILE [file join $::quartus(sip_path) "virtual_jtag.vhd"]
set_global_assignment -library "lib_virtual_jtag" -name MISC_FILE [file join $::quartus(sip_path) "submodules/sld_virtual_jtag.v"]
