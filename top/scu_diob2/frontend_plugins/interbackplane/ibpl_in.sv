/*
	A typical input cardlet
*/

`timescale 1 ps / 1 ps
`include "../../blackbox_defines.vh"
`include "../../blackbox_config.vh"
`include "interbackplane_helpers.vh"


module ibpl_in #(
		parameter invert_signals = 0,
		`STD_CARDLET_PARAMS
	)(
		`STD_CARDLET_PORTS
	);
	
assign diob_dir		= 6'h00;
assign diob_out		= 6'h00;
assign internal_in	= {2'b0, invert_signals ? ~diob_in : diob_in};
assign diob_led2 	= {2'b0, input_enable[5:0]};
assign diob_led1 	= {2'b0, input_act[5:0]};

	// Generate module error if ANY of the signals has output enabled but input disabled
assign plugin_error = |((output_enable & ~input_enable) & 'h3F);

endmodule
