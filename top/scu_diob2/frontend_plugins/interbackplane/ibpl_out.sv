/*
	A typical output cardlet
*/

`timescale 1 ps / 1 ps
`include "../../blackbox_defines.vh"
`include "../../blackbox_config.vh"
`include "interbackplane_helpers.vh"


module ibpl_out #(
		`STD_CARDLET_PARAMS
	)(
		`STD_CARDLET_PORTS
	);
	
assign diob_dir		= 6'h3F;
assign diob_out		= internal_out[5:0];
assign internal_in	= 8'b0;
assign diob_led1	= {2'b0, output_enable[5:0]};
assign diob_led2	= {2'b0, output_act[5:0]};

	// Generate module error if ANY of the signals has input enabled but output disabled
assign plugin_error = |((input_enable & ~output_enable) & 'h3F);

endmodule