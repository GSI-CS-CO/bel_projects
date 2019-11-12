
module ref_pll10 (
	cntsel,
	loaden,
	locked,
	lvds_clk,
	num_phase_shifts,
	outclk_2,
	outclk_3,
	outclk_4,
	phase_done,
	phase_en,
	refclk,
	rst,
	scanclk,
	updn);	

	input	[4:0]	cntsel;
	output	[1:0]	loaden;
	output		locked;
	output	[1:0]	lvds_clk;
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
endmodule
