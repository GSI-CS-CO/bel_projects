# This logic lock is needed for all SERDES/LVDS drivers.
# Without this PLL logic lock, all SERDES/LVDS outputs will generate glitches.
set_instance_assignment -name LL_MEMBER_OF ref_pll_out0_125mhz -to "monster:main|ref_pll5:\\ref_a5:ref_inst|ref_pll5_0002:ref_pll5_inst|altera_pll:altera_pll_i|altera_pll_dps_lcell_comb:lcell_cntsel_int_0|combout" -section_id ref_pll_out0_125mhz
set_instance_assignment -name LL_MEMBER_OF ref_pll_out1_200mhz -to "monster:main|ref_pll5:\\ref_a5:ref_inst|ref_pll5_0002:ref_pll5_inst|altera_pll:altera_pll_i|altera_pll_dps_lcell_comb:lcell_cntsel_int_1|combout" -section_id ref_pll_out1_200mhz
set_instance_assignment -name LL_MEMBER_OF ref_pll_out2_25mhz -to "monster:main|ref_pll5:\\ref_a5:ref_inst|ref_pll5_0002:ref_pll5_inst|altera_pll:altera_pll_i|altera_pll_dps_lcell_comb:lcell_cntsel_int_2|combout" -section_id ref_pll_out2_25mhz
set_instance_assignment -name LL_MEMBER_OF ref_pll_out3_1000mhz -to "monster:main|ref_pll5:\\ref_a5:ref_inst|ref_pll5_0002:ref_pll5_inst|altera_pll:altera_pll_i|altera_pll_dps_lcell_comb:lcell_cntsel_int_3|combout" -section_id ref_pll_out3_1000mhz
set_instance_assignment -name LL_MEMBER_OF ref_pll_out4_125mhz_p1_8 -to "monster:main|ref_pll5:\\ref_a5:ref_inst|ref_pll5_0002:ref_pll5_inst|altera_pll:altera_pll_i|altera_pll_dps_lcell_comb:lcell_cntsel_int_4|combout" -section_id ref_pll_out4_125mhz_p1_8
set_global_assignment -name LL_AUTO_SIZE OFF -section_id ref_pll_out0_125mhz
set_global_assignment -name LL_AUTO_SIZE OFF -section_id ref_pll_out1_200mhz
set_global_assignment -name LL_AUTO_SIZE OFF -section_id ref_pll_out2_25mhz
set_global_assignment -name LL_AUTO_SIZE OFF -section_id ref_pll_out3_1000mhz
set_global_assignment -name LL_AUTO_SIZE OFF -section_id ref_pll_out4_125mhz_p1_8
set_global_assignment -name LL_CORE_ONLY OFF -section_id ref_pll_out0_125mhz
set_global_assignment -name LL_CORE_ONLY OFF -section_id ref_pll_out1_200mhz
set_global_assignment -name LL_CORE_ONLY OFF -section_id ref_pll_out2_25mhz
set_global_assignment -name LL_CORE_ONLY OFF -section_id ref_pll_out3_1000mhz
set_global_assignment -name LL_CORE_ONLY OFF -section_id ref_pll_out4_125mhz_p1_8
set_global_assignment -name LL_ENABLED ON -section_id ref_pll_out0_125mhz
set_global_assignment -name LL_ENABLED ON -section_id ref_pll_out1_200mhz
set_global_assignment -name LL_ENABLED ON -section_id ref_pll_out2_25mhz
set_global_assignment -name LL_ENABLED ON -section_id ref_pll_out3_1000mhz
set_global_assignment -name LL_ENABLED ON -section_id ref_pll_out4_125mhz_p1_8
set_global_assignment -name LL_HEIGHT 1 -section_id ref_pll_out0_125mhz
set_global_assignment -name LL_HEIGHT 1 -section_id ref_pll_out1_200mhz
set_global_assignment -name LL_HEIGHT 1 -section_id ref_pll_out2_25mhz
set_global_assignment -name LL_HEIGHT 1 -section_id ref_pll_out3_1000mhz
set_global_assignment -name LL_HEIGHT 1 -section_id ref_pll_out4_125mhz_p1_8
set_global_assignment -name LL_IGNORE_IO_BANK_SECURITY_CONSTRAINT OFF -section_id ref_pll_out0_125mhz
set_global_assignment -name LL_IGNORE_IO_BANK_SECURITY_CONSTRAINT OFF -section_id ref_pll_out1_200mhz
set_global_assignment -name LL_IGNORE_IO_BANK_SECURITY_CONSTRAINT OFF -section_id ref_pll_out2_25mhz
set_global_assignment -name LL_IGNORE_IO_BANK_SECURITY_CONSTRAINT OFF -section_id ref_pll_out3_1000mhz
set_global_assignment -name LL_IGNORE_IO_BANK_SECURITY_CONSTRAINT OFF -section_id ref_pll_out4_125mhz_p1_8
set_global_assignment -name LL_ORIGIN X36_Y88 -section_id ref_pll_out4_125mhz_p1_8
set_global_assignment -name LL_ORIGIN X37_Y87 -section_id ref_pll_out1_200mhz
set_global_assignment -name LL_ORIGIN X37_Y88 -section_id ref_pll_out2_25mhz
set_global_assignment -name LL_ORIGIN X39_Y88 -section_id ref_pll_out3_1000mhz
set_global_assignment -name LL_ORIGIN X41_Y87 -section_id ref_pll_out0_125mhz
set_global_assignment -name LL_PR_REGION OFF -section_id ref_pll_out0_125mhz
set_global_assignment -name LL_PR_REGION OFF -section_id ref_pll_out1_200mhz
set_global_assignment -name LL_PR_REGION OFF -section_id ref_pll_out2_25mhz
set_global_assignment -name LL_PR_REGION OFF -section_id ref_pll_out3_1000mhz
set_global_assignment -name LL_PR_REGION OFF -section_id ref_pll_out4_125mhz_p1_8
set_global_assignment -name LL_RESERVED OFF -section_id ref_pll_out0_125mhz
set_global_assignment -name LL_RESERVED OFF -section_id ref_pll_out1_200mhz
set_global_assignment -name LL_RESERVED OFF -section_id ref_pll_out2_25mhz
set_global_assignment -name LL_RESERVED OFF -section_id ref_pll_out3_1000mhz
set_global_assignment -name LL_RESERVED OFF -section_id ref_pll_out4_125mhz_p1_8
set_global_assignment -name LL_ROUTING_REGION_EXPANSION_SIZE 2147483647 -section_id ref_pll_out0_125mhz
set_global_assignment -name LL_ROUTING_REGION_EXPANSION_SIZE 2147483647 -section_id ref_pll_out1_200mhz
set_global_assignment -name LL_ROUTING_REGION_EXPANSION_SIZE 2147483647 -section_id ref_pll_out2_25mhz
set_global_assignment -name LL_ROUTING_REGION_EXPANSION_SIZE 2147483647 -section_id ref_pll_out3_1000mhz
set_global_assignment -name LL_ROUTING_REGION_EXPANSION_SIZE 2147483647 -section_id ref_pll_out4_125mhz_p1_8
set_global_assignment -name LL_SECURITY_ROUTING_INTERFACE OFF -section_id ref_pll_out0_125mhz
set_global_assignment -name LL_SECURITY_ROUTING_INTERFACE OFF -section_id ref_pll_out1_200mhz
set_global_assignment -name LL_SECURITY_ROUTING_INTERFACE OFF -section_id ref_pll_out2_25mhz
set_global_assignment -name LL_SECURITY_ROUTING_INTERFACE OFF -section_id ref_pll_out3_1000mhz
set_global_assignment -name LL_SECURITY_ROUTING_INTERFACE OFF -section_id ref_pll_out4_125mhz_p1_8
set_global_assignment -name LL_STATE LOCKED -section_id ref_pll_out0_125mhz
set_global_assignment -name LL_STATE LOCKED -section_id ref_pll_out1_200mhz
set_global_assignment -name LL_STATE LOCKED -section_id ref_pll_out2_25mhz
set_global_assignment -name LL_STATE LOCKED -section_id ref_pll_out3_1000mhz
set_global_assignment -name LL_STATE LOCKED -section_id ref_pll_out4_125mhz_p1_8
set_global_assignment -name LL_WIDTH 2 -section_id ref_pll_out0_125mhz
set_global_assignment -name LL_WIDTH 2 -section_id ref_pll_out1_200mhz
set_global_assignment -name LL_WIDTH 2 -section_id ref_pll_out2_25mhz
set_global_assignment -name LL_WIDTH 2 -section_id ref_pll_out3_1000mhz
set_global_assignment -name LL_WIDTH 2 -section_id ref_pll_out4_125mhz_p1_8
# To be evaluated
set_location_assignment FRACTIONALPLL_X0_Y18_N0 -to "monster:main|dmtd_pll5:\\dmtd_a5:dmtd_inst|dmtd_pll5_0002:dmtd_pll5_inst|altera_pll:altera_pll_i|general[0].gpll~FRACTIONAL_PLL"
set_location_assignment FRACTIONALPLL_X0_Y60_N0 -to "monster:main|sys_pll5:\\sys_a5:sys_inst|sys_pll5_0002:sys_pll5_inst|altera_pll:altera_pll_i|general[0].gpll~FRACTIONAL_PLL"
set_location_assignment FRACTIONALPLL_X43_Y65_N0 -to "monster:main|ref_pll5:\\ref_a5:ref_inst|ref_pll5_0002:ref_pll5_inst|altera_pll:altera_pll_i|altera_arriav_pll:arriav_pll|altera_arriav_pll_base:fpll_0|cntnen"
set_location_assignment PLLOUTPUTCOUNTER_X0_Y20_N1 -to "monster:main|dmtd_pll5:\\dmtd_a5:dmtd_inst|dmtd_pll5_0002:dmtd_pll5_inst|altera_pll:altera_pll_i|outclk_wire[0]"
set_location_assignment PLLOUTPUTCOUNTER_X0_Y64_N1 -to "monster:main|sys_pll5:\\sys_a5:sys_inst|sys_pll5_0002:sys_pll5_inst|altera_pll:altera_pll_i|outclk_wire[3]"
set_location_assignment PLLOUTPUTCOUNTER_X0_Y65_N1 -to "monster:main|sys_pll5:\\sys_a5:sys_inst|sys_pll5_0002:sys_pll5_inst|altera_pll:altera_pll_i|outclk_wire[2]"
set_location_assignment PLLOUTPUTCOUNTER_X0_Y66_N1 -to "monster:main|sys_pll5:\\sys_a5:sys_inst|sys_pll5_0002:sys_pll5_inst|altera_pll:altera_pll_i|outclk_wire[1]"
set_location_assignment PLLOUTPUTCOUNTER_X0_Y67_N1 -to "monster:main|sys_pll5:\\sys_a5:sys_inst|sys_pll5_0002:sys_pll5_inst|altera_pll:altera_pll_i|outclk_wire[0]"
set_location_assignment PLLOUTPUTCOUNTER_X43_Y61_N1 -to "monster:main|ref_pll5:\\ref_a5:ref_inst|ref_pll5_0002:ref_pll5_inst|altera_pll:altera_pll_i|altera_arriav_pll:arriav_pll|cascade_wire[2]"
set_location_assignment PLLOUTPUTCOUNTER_X43_Y62_N1 -to "monster:main|ref_pll5:\\ref_a5:ref_inst|ref_pll5_0002:ref_pll5_inst|altera_pll:altera_pll_i|altera_arriav_pll:arriav_pll|cascade_wire[1]"
set_location_assignment PLLOUTPUTCOUNTER_X43_Y63_N1 -to "monster:main|ref_pll5:\\ref_a5:ref_inst|ref_pll5_0002:ref_pll5_inst|altera_pll:altera_pll_i|altera_arriav_pll:arriav_pll|cascade_wire[0]"
