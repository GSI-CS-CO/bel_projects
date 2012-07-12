trans_rcfg_inst : trans_rcfg PORT MAP (
		logical_channel_address	 => logical_channel_address_sig,
		reconfig_clk	 => reconfig_clk_sig,
		reconfig_fromgxb	 => reconfig_fromgxb_sig,
		busy	 => busy_sig,
		reconfig_togxb	 => reconfig_togxb_sig
	);
