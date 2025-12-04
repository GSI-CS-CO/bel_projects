/*
	Process plugin which just passes signals in both directions without buffering.
*/

`timescale 1 ps / 1 ps
`include "../blackbox_defines.vh"
`include "../blackbox_config.vh"
`include "proc_helpers.vh"


module proc_pass #(
		`STD_PROC_PARAMS
	)(
		`STD_PROC_PORTS
	);

assign internal_out 	= virtual_out;
assign virtual_in 		= internal_in;
assign output_enable	= 1;
assign input_enable		= 1;



endmodule