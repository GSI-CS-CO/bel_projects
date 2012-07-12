trans_loop_inst : trans_loop PORT MAP (
		cal_blk_clk	 => cal_blk_clk_sig,
		pll_inclk	 => pll_inclk_sig,
		reconfig_clk	 => reconfig_clk_sig,
		reconfig_togxb	 => reconfig_togxb_sig,
		rx_datain	 => rx_datain_sig,
		rx_digitalreset	 => rx_digitalreset_sig,
		tx_digitalreset	 => tx_digitalreset_sig,
		reconfig_fromgxb	 => reconfig_fromgxb_sig,
		rx_bistdone	 => rx_bistdone_sig,
		rx_bisterr	 => rx_bisterr_sig,
		rx_clkout	 => rx_clkout_sig,
		rx_signaldetect	 => rx_signaldetect_sig,
		tx_clkout	 => tx_clkout_sig,
		tx_dataout	 => tx_dataout_sig
	);
