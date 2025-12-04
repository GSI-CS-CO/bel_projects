/*
	A simple PIO to read/write all signals.

*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"
`include "../flex_bus/flex_helpers.vh"
`include "user_helpers.vh"

module user_gpio #(
		`STD_USER_PARAMS
	)(
		`STD_USER_PORTS
	);

// ********** Some defines **********

localparam nr_bus_slaves 			= 6;

localparam nr_registers 			= `ceildiv(nr_virt_ios, data_bus_width);
localparam nr_config_registers 		= nr_registers * proc_sel_bits;
localparam nr_backplane_registers 	= `ceildiv(nr_backplane_ios, data_bus_width);

// ********** Secondary bus **********

`MAKE_SEC_BUS(SEC, nr_bus_slaves, addr_bus_width, data_bus_width)

// ********** Main hub ********** 

`FLEX_SUPERHUB(flex_superhub_main,, SEC, 0, addr_bus_width, 0, 0, nr_bus_slaves, addr_bus_width, data_bus_width)

// ********** Input ********** 

`FLEX_IN(0, flex_in_sig, SEC, `BB_BASE_gpio_in_reg, nr_registers, addr_bus_width, data_bus_width, virtual_in)

// ********** Output ********** 

`FLEX_OUT(1, flex_out_sig, SEC, `BB_BASE_gpio_out_reg, nr_registers, addr_bus_width, data_bus_width, virtual_out)

// ********** Proc plugin selection ********** 

`FLEX_OUT(2, flex_out_proc_plugin_select, SEC, `BB_BASE_gpio_proc_sel, nr_config_registers, addr_bus_width, data_bus_width, proc_plugin_select)

// ********** Backplane input ********** 

`FLEX_IN(3, flex_in_bp, SEC, `BB_BASE_gpio_bp_in_reg, nr_backplane_registers, addr_bus_width, data_bus_width, backplane_in)

// ********** Backplane output ********** 

`FLEX_OUT(4, flex_out_bp, SEC, `BB_BASE_gpio_bp_out_reg, nr_backplane_registers, addr_bus_width, data_bus_width, backplane_out)

// ********** Backplane direction ********** 

`FLEX_OUT(5, flex_out_bpdir, SEC, `BB_BASE_gpio_bp_dir_reg, nr_backplane_registers, addr_bus_width, data_bus_width, backplane_dir)

endmodule