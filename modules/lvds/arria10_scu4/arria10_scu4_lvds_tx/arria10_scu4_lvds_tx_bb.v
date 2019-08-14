
module arria10_scu4_lvds_tx (
	tx_in,
	tx_out,
	tx_coreclock,
	pll_areset,
	ext_fclk,
	ext_loaden,
	ext_coreclock);	

	input	[7:0]	tx_in;
	output	[0:0]	tx_out;
	output		tx_coreclock;
	input		pll_areset;
	input		ext_fclk;
	input		ext_loaden;
	input		ext_coreclock;
endmodule
