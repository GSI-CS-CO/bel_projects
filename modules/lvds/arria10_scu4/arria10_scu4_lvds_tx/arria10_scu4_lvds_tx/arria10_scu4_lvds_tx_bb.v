
module arria10_scu4_lvds_tx (
	ext_coreclock,
	ext_fclk,
	ext_loaden,
	tx_coreclock,
	tx_in,
	tx_out);	

	input		ext_coreclock;
	input		ext_fclk;
	input		ext_loaden;
	output		tx_coreclock;
	input	[7:0]	tx_in;
	output	[0:0]	tx_out;
endmodule
