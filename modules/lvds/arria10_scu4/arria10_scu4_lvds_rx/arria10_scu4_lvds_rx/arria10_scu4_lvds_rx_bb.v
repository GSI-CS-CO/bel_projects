
module arria10_scu4_lvds_rx (
	ext_fclk,
	ext_loaden,
	ext_coreclock,
	rx_in,
	rx_out);	

	input		ext_fclk;
	input		ext_loaden;
	input		ext_coreclock;
	input	[0:0]	rx_in;
	output	[7:0]	rx_out;
endmodule
