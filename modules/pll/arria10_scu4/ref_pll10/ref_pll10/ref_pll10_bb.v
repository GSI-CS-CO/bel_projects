
module ref_pll10 (
	rst,
	refclk,
	locked,
	outclk_0,
	outclk_1,
	outclk_2,
	outclk_3,
	outclk_4,
	scanclk,
	phase_en,
	updn,
	cntsel,
	phase_done,
	num_phase_shifts);	

	input		rst;
	input		refclk;
	output		locked;
	output		outclk_0;
	output		outclk_1;
	output		outclk_2;
	output		outclk_3;
	output		outclk_4;
	input		scanclk;
	input		phase_en;
	input		updn;
	input	[4:0]	cntsel;
	output		phase_done;
	input	[2:0]	num_phase_shifts;
endmodule
