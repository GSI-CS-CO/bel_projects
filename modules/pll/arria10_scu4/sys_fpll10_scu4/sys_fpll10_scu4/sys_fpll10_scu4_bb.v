
module sys_fpll10_scu4 (
	pll_refclk0,
	pll_powerdown,
	pll_locked,
	pll_cal_busy,
	outclk0,
	outclk1,
	outclk2,
	outclk3);	

	input		pll_refclk0;
	input		pll_powerdown;
	output		pll_locked;
	output		pll_cal_busy;
	output		outclk0;
	output		outclk1;
	output		outclk2;
	output		outclk3;
endmodule
