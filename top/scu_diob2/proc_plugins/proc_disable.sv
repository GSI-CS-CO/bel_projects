/*
	Process plugin which just blocks all signals
*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"
`include "proc_helpers.vh"


module proc_disable #(
		`STD_PROC_PARAMS
	)(
		`STD_PROC_PORTS
	);

assign internal_out 	= 0;
assign virtual_in 		= 0;
assign output_enable	= 0;
assign input_enable		= 0;


endmodule