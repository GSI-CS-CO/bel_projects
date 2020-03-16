
module ref_pll10 (
	cntsel,
	locked,
	num_phase_shifts,
	outclk_2,
	outclk_3,
	outclk_4,
	phase_done,
	phase_en,
	refclk,
	rst,
	scanclk,
	updn,
	lvds_clk,
	loaden);	

	input	[4:0]	cntsel;
	output		locked;
	input	[2:0]	num_phase_shifts;
	output		outclk_2;
	output		outclk_3;
	output		outclk_4;
	output		phase_done;
	input		phase_en;
	input		refclk;
	input		rst;
	input		scanclk;
	input		updn;
	output	[1:0]	lvds_clk;
	output	[1:0]	loaden;
endmodule
