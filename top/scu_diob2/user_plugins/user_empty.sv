/*
	An empty user plugin with no function.
*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"
`include "../flex_bus/flex_helpers.vh"
`include "user_helpers.vh"

module user_empty #(
		`STD_USER_PARAMS
	)(
		`STD_USER_PORTS
	);

assign virtual_out = 0;
assign proc_plugin_select = 0;
assign backplane_out = 0;
assign irq = 0;

endmodule