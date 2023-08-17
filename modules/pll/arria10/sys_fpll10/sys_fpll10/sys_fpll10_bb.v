
module sys_fpll10 (
	outclk0,
	outclk1,
	outclk2,
	outclk3,
	pll_cal_busy,
	pll_locked,
	pll_powerdown,
	pll_refclk0);	

	output		outclk0;
	output		outclk1;
	output		outclk2;
	output		outclk3;
	output		pll_cal_busy;
	output		pll_locked;
	input		pll_powerdown;
	input		pll_refclk0;
endmodule
