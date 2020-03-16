`timescale 1ns/10ps
module  ref_pll10_altera_iopll_160_d3szf3y(

	// interface 'reset'
	input wire rst,

	// interface 'refclk'
	input wire refclk,

	// interface 'locked'
	output wire locked,

	// interface 'scanclk'
	input wire scanclk,

	// interface 'phase_en'
	input wire phase_en,

	// interface 'updn'
	input wire updn,

	// interface 'cntsel'
	input wire [4:0] cntsel,

	// interface 'phase_done'
	output wire phase_done,

	// interface 'num_phase_shifts'
	input wire [2:0] num_phase_shifts,

	// interface 'outclk0'
	output wire outclk_0,

	// interface 'outclk1'
	output wire outclk_1,

	// interface 'outclk2'
	output wire outclk_2,

	// interface 'outclk3'
	output wire outclk_3,

	// interface 'outclk4'
	output wire outclk_4
);

	altera_iopll #(
		.c_cnt_bypass_en0("false"),
		.c_cnt_bypass_en1("false"),
		.c_cnt_bypass_en2("false"),
		.c_cnt_bypass_en3("true"),
		.c_cnt_bypass_en4("false"),
		.c_cnt_bypass_en5("true"),
		.c_cnt_bypass_en6("true"),
		.c_cnt_bypass_en7("true"),
		.c_cnt_bypass_en8("true"),
		.c_cnt_hi_div0(4),
		.c_cnt_hi_div1(3),
		.c_cnt_hi_div2(20),
		.c_cnt_hi_div3(256),
		.c_cnt_hi_div4(1),
		.c_cnt_hi_div5(256),
		.c_cnt_hi_div6(256),
		.c_cnt_hi_div7(256),
		.c_cnt_hi_div8(256),
		.c_cnt_lo_div0(4),
		.c_cnt_lo_div1(2),
		.c_cnt_lo_div2(20),
		.c_cnt_lo_div3(256),
		.c_cnt_lo_div4(7),
		.c_cnt_lo_div5(256),
		.c_cnt_lo_div6(256),
		.c_cnt_lo_div7(256),
		.c_cnt_lo_div8(256),
		.c_cnt_odd_div_duty_en0("false"),
		.c_cnt_odd_div_duty_en1("true"),
		.c_cnt_odd_div_duty_en2("false"),
		.c_cnt_odd_div_duty_en3("false"),
		.c_cnt_odd_div_duty_en4("false"),
		.c_cnt_odd_div_duty_en5("false"),
		.c_cnt_odd_div_duty_en6("false"),
		.c_cnt_odd_div_duty_en7("false"),
		.c_cnt_odd_div_duty_en8("false"),
		.c_cnt_ph_mux_prst0(0),
		.c_cnt_ph_mux_prst1(0),
		.c_cnt_ph_mux_prst2(0),
		.c_cnt_ph_mux_prst3(0),
		.c_cnt_ph_mux_prst4(0),
		.c_cnt_ph_mux_prst5(0),
		.c_cnt_ph_mux_prst6(0),
		.c_cnt_ph_mux_prst7(0),
		.c_cnt_ph_mux_prst8(0),
		.c_cnt_prst0(1),
		.c_cnt_prst1(1),
		.c_cnt_prst2(1),
		.c_cnt_prst3(1),
		.c_cnt_prst4(1),
		.c_cnt_prst5(1),
		.c_cnt_prst6(1),
		.c_cnt_prst7(1),
		.c_cnt_prst8(1),
		.clock_name_0("outclk0"),
		.clock_name_1("outclk1"),
		.clock_name_2("outclk2"),
		.clock_name_3("outclk3"),
		.clock_name_4("outclk4"),
		.clock_name_5(""),
		.clock_name_6(""),
		.clock_name_7(""),
		.clock_name_8(""),
		.clock_name_global_0("false"),
		.clock_name_global_1("false"),
		.clock_name_global_2("false"),
		.clock_name_global_3("false"),
		.clock_name_global_4("false"),
		.clock_name_global_5("false"),
		.clock_name_global_6("false"),
		.clock_name_global_7("false"),
		.clock_name_global_8("false"),
		.duty_cycle0(50),
		.duty_cycle1(50),
		.duty_cycle2(50),
		.duty_cycle3(50),
		.duty_cycle4(12),
		.duty_cycle5(50),
		.duty_cycle6(50),
		.duty_cycle7(50),
		.duty_cycle8(50),
		.m_cnt_bypass_en("false"),
		.m_cnt_hi_div(4),
		.m_cnt_lo_div(4),
		.m_cnt_odd_div_duty_en("false"),
		.n_cnt_bypass_en("true"),
		.n_cnt_hi_div(256),
		.n_cnt_lo_div(256),
		.n_cnt_odd_div_duty_en("false"),
		.number_of_clocks(5),
		.operation_mode("lvds"),
		.output_clock_frequency0("125.000000 MHz"),
		.output_clock_frequency1("200.000000 MHz"),
		.output_clock_frequency2("25.000000 MHz"),
		.output_clock_frequency3("1000.000000 MHz"),
		.output_clock_frequency4("125.000000 MHz"),
		.output_clock_frequency5("0 ps"),
		.output_clock_frequency6("0 ps"),
		.output_clock_frequency7("0 ps"),
		.output_clock_frequency8("0 ps"),
		.phase_shift0("0 ps"),
		.phase_shift1("0 ps"),
		.phase_shift2("0 ps"),
		.phase_shift3("0 ps"),
		.phase_shift4("0 ps"),
		.phase_shift5("0 ps"),
		.phase_shift6("0 ps"),
		.phase_shift7("0 ps"),
		.phase_shift8("0 ps"),
		.pll_bw_sel("Low"),
		.pll_bwctrl("pll_bw_res_setting3"),
		.pll_cp_current("pll_cp_setting14"),
		.pll_extclk_0_cnt_src("pll_extclk_cnt_src_vss"),
		.pll_extclk_1_cnt_src("pll_extclk_cnt_src_vss"),
		.pll_fbclk_mux_1("pll_fbclk_mux_1_lvds"),
		.pll_fbclk_mux_2("pll_fbclk_mux_2_fb_1"),
		.pll_m_cnt_in_src("c_m_cnt_in_src_ph_mux_clk"),
		.pll_output_clk_frequency("1000.0 MHz"),
		.pll_slf_rst("false"),
		.pll_subtype("DPS"),
		.pll_type("Arria 10"),
		.reference_clock_frequency("125.0 MHz")
	) altera_iopll_i (
		.refclk1	(1'b0),
		.rst	(rst),
		.fbclk	(1'b0),
		.fboutclk	( ),
		.zdbfbclk	( ),
		.locked	(locked),
		.loaden	( ),
		.phase_done	(phase_done),
		.reconfig_to_pll	(64'b0),
		.refclk	(refclk),
		.scanclk	(scanclk),
		.phout	( ),
		.num_phase_shifts	(num_phase_shifts),
		.cntsel	(cntsel),
		.clkbad	( ),
		.extclk_out	( ),
		.lvds_clk	( ),
		.outclk	({outclk_4, outclk_3, outclk_2, outclk_1, outclk_0}),
		.phase_en	(phase_en),
		.extswitch	(1'b0),
		.cascade_out	( ),
		.activeclk	( ),
		.adjpllin	(1'b0),
		.updn	(updn),
		.reconfig_from_pll	( )
	);
endmodule

