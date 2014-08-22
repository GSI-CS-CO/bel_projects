
# -----------------------------------------------------------------
# JTAG access (open/close)
# -----------------------------------------------------------------

# Open the DE2 Cyclone II device using the on-board USB-Blaster.
proc jtag_open {} {
	# Get the list of JTAG controllers
	set hardware_names [get_hardware_names]
	
	# Select the first JTAG controller
	set hardware_name [lindex $hardware_names 0]
	
	# Get the list of FPGAs in the JTAG chain
	set device_names [get_device_names\
		-hardware_name $hardware_name]
	
	# Select the first FPGA
	set device_name [lindex $device_names 0]

	puts "\nJTAG: $hardware_name, FPGA: $device_name"
	open_device -hardware_name $hardware_name\
		-device_name $device_name
}

# Close the device
proc jtag_close {} {
	close_device
}

# -----------------------------------------------------------------
# JTAG instructions
# -----------------------------------------------------------------

# Read the JTAG ID code
#  * Expected ID code = 0x020B40DD
#
proc read_idcode {} {
	device_lock -timeout 10000

	# Shift-IR: = IDCODE = 6
	device_ir_shift -ir_value 6 -no_captured_ir_value
	
	# Shift-DR: read 32-bits
	set val 0x[device_dr_shift -length 32 -value_in_hex]

	device_unlock
	return $val
}

# Read the JTAG USERCODE code
#  * If its not set, then the expected value is 0xFFFFFFFF
#  * The USERCODE can be set using, eg.
# 	set_global_assignment -name STRATIX_JTAG_USER_CODE 12345678
#
proc read_usercode {} {
	device_lock -timeout 10000

	# Shift-IR: = USERCODE = 7
	device_ir_shift -ir_value 7 -no_captured_ir_value
	
	# Shift-DR: read 32-bits
	set val 0x[device_dr_shift -length 32 -value_in_hex]

	device_unlock
	return $val
}

# Pulse CONFIG#
#
# There is no documentation on what to do with this command.
# The Shift-IR command alone does not do anything (until
# a subsequent command is issued). Issuing a shift on
# DR causes the configuration to be cleared.
#
# On the DE2 board, this will cause the factory design to
# be reloaded (blinking LEDs, counting hex displays).
#
proc pulse_nconfig {} {
	device_lock -timeout 10000

	# Shift-IR: = PULSE_NCONFIG = 1
	device_ir_shift -ir_value 1 -no_captured_ir_value

	# Shift-DR: read 1-bitt
	device_dr_shift -length 1 -dr_value 0

	device_unlock
	return
}

# -----------------------------------------------------------------
# JTAG HUB interrogation
# -----------------------------------------------------------------

# Print the hub info
#  * The DE2 board returns (with generic VIR_WIDTH = 3)
#
#         Hub info: 0x08086E04
#        VIR Width: 4
#  Manufacturer ID: 0x6E
#  Number of nodes: 1
#       IP Version: 1
#
# The VIR width field is the width of the Virtual instruction 
# register minus the hub ADDR bits, i.e., its the USER1 shift-DR
# length excluding the ADDR bits (which is 1-bit for the DE2 
# example).
#       
proc print_hub_info {} {

	set h(info)    [hub_info]
	set h(sum)     [hub_vir_width_m $h(info)]
	set h(mfg_id)  [hub_mfg_id $h(info)]
	set h(nodes)   [hub_number_of_nodes $h(info)]
	set h(version) [hub_version $h(info)]
	puts "         Hub info: $h(info)"
	puts "      VIR m-width: $h(sum)"
	puts "  Manufacturer ID: $h(mfg_id)"
	puts "  Number of nodes: $h(nodes)"
	puts "       IP Version: $h(version)"
}


# Read the hub info (see Appendix A in the Virtual JTAG users guide)
# * The DE2 board returns 0x08086E04
#
proc hub_info {} {
	device_lock -timeout 10000

	# Shift-IR: = USER1 = 0xE
	device_ir_shift -ir_value 14 -no_captured_ir_value

	# Shift-DR: write 64-bits
	device_dr_shift -length 64 -value_in_hex -dr_value 0000000000000000
	
	# Shift-IR: = USER0 = 0xC
	device_ir_shift -ir_value 12 -no_captured_ir_value

	# Shift-DR: read 4-bits, 8 times
	set data {}
	for {set i 0} {$i < 8} {incr i} {
		set byte [device_dr_shift -length 4 -value_in_hex]

		# Build up a hex string
		set data $byte$data
	}
	device_unlock
	return 0x$data
}

proc hub_vir_width_m {data} {
	return [expr {$data & 0xFF}]
}

proc hub_mfg_id {data} {
	return [format 0x%X [expr {($data >> 8) & 0x7FF}]]
}

proc hub_number_of_nodes {data} {
	return [expr {($data >> 19) & 0xFF}]
}

proc hub_version {data} {
	return [expr {($data >> 27) & 0x1F}]
}

proc print_node_info {} {

	# Get a list of node info responses
	set ninfo [node_info]
	
	# Loop over the list and print the fields
	foreach node $ninfo {
		set n(inst) [expr {$node       & 0xFF}]
		set n(mfg)  [expr {($node>>8)  & 0x7FF}]
		set n(id)   [expr {($node>>19) & 0xFF}]
		set n(ver)  [expr {($node>>27) & 0x1F}]

		puts ""
		puts "    Node instance: [format "%4d (0x%X)" $n(inst) $n(inst)]"
		puts "Node manufacturer: [format "%4d (0x%X)" $n(mfg)  $n(mfg)]"
		puts "          Node ID: [format "%4d (0x%X)" $n(id)   $n(id)]"
		puts "     Node version: [format "%4d (0x%X)" $n(ver)  $n(ver)]"

	}
}
       
proc node_info {} {
	device_lock -timeout 10000

	# Shift-IR: = USER1 = 0xE
	device_ir_shift -ir_value 14 -no_captured_ir_value

	# Shift-DR: write 64-bits
	device_dr_shift -length 64 -value_in_hex -dr_value 0000000000000000
	
	# Shift-IR: = USER0 = 0xC
	device_ir_shift -ir_value 12 -no_captured_ir_value

	# Shift-DR: read 4-bits, 8 times
	set data {}
	for {set i 0} {$i < 8} {incr i} {
		set byte [device_dr_shift -length 4 -value_in_hex]

		# Build up a hex string
		set data $byte$data
	}
	
	# Number of nodes
	set nodes [hub_number_of_nodes 0x$data]
	
	set ndata {}
	for {set i 0} {$i < $nodes} {incr i} {
	
		# Read out the SLD_NODE_INFO registers (4-bit nibble at a time)
		set data {}
		for {set j 0} {$j < 8} {incr j} {
			set byte [device_dr_shift -length 4 -value_in_hex]

			# Build up a hex string
			set data $byte$data
		}
		lappend ndata 0x$data
	}
	device_unlock
	return $ndata
}

# -----------------------------------------------------------------
# Virtual JTAG instructions
# -----------------------------------------------------------------

# Control the state of the LEDs on the DE2 board by writing 
# Virtual Instruction register values
#
proc jtag_vir {val} {
	device_lock -timeout 1000
	set ret [device_virtual_ir_shift -instance_index 0 -ir_value $val]
	device_unlock
	return $ret
}

# Generate serial transaction on the TDI signal
#  * Note that you first need to select the design's DR path by
#    issuing a VIR command.
#  * The design's TDO toggles, so reads return strings of
#    5555 or AAAA's
#
proc jtag_vdr {val {len 8}} {
	
	# Virtual shift-DR can be of arbitrary length, however, the
	# logic in this procedure uses Tcl 32-bit integer logic, so 
	# limit length to 32-bits
	#
	if {$len > 32} {
		error "Length must be less than or equal to 32"
	}

	# Convert the input value to a hex string of the correct length
	#
	# Number of hex characters required in the argument to shift-DR
	set hlen [expr {int(ceil(double($len)/4.0))}]
	#
	# Force val to be less than this value
	set mask [expr {(1<<4*$hlen)-1}]
	set val  [expr {$val & $mask}]
	#
	# Create the hex string (without leading 0x)
	set hval [format %0${hlen}X $val]

	device_lock -timeout 1000
	set ret 0x[device_virtual_dr_shift -instance_index 0 -length $len -dr_value $hval -value_in_hex]
	device_unlock
	return $ret
}

# Pulse CONFIG#
#
# There is no documentation on what to do with this command.
# The Shift-IR command alone does not do anything (until
# a subsequent command is issued). Issuing a shift on
# DR causes the configuration to be cleared.
#
# On the DE2 board, this will cause the factory design to
# be reloaded (blinking LEDs, counting hex displays).
#
proc pulse_nconfig {} {
	device_lock -timeout 10000

	# Shift-IR: = PULSE_NCONFIG = 1
	device_ir_shift -ir_value 1 -no_captured_ir_value

	# Shift-DR: read 1-bitt
	#device_dr_shift -length 1 -dr_value 0

	device_unlock
	return
}

proc send_factory {} {
	device_lock -timeout 10000

	device_ir_shift -ir_value 641 -no_captured_ir_value

	device_unlock
	return
}

proc ver_key {} {
	device_lock -timeout 10000

	device_ir_shift -ir_value 19 -no_captured_ir_value

	set val 0x[device_dr_shift -length 32 -value_in_hex]

	device_unlock
	return $val
}


proc clear_key {} {
	device_lock -timeout 10000

	# Shift-IR: = USERCODE = 41
	device_ir_shift -ir_value 41 -no_captured_ir_value

	device_unlock
	return 
}
