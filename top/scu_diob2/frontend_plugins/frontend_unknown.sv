/*
	A module without connections.
	To be used as dummy if no module is found.
*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"
`include "frontend_helpers.vh"

module frontend_unknown #(
		`STD_FRONTEND_PARAMS
	)(
		`STD_FRONTEND_PORTS
	);

// Some random assignments

assign diob_out[nr_diob_ios-1:0]		= 0;
assign diob_dir[nr_diob_ios-1:0]		= 0;
assign internal_in[nr_virt_ios-1:0]		= 0;
assign plugin_error 					= 1;	//Unknown module plugin always returns an error.

assign plugin_status 					= 0;

endmodule