	arria10_scu4_lvds_pll u0 (
		.rst      (<connected-to-rst>),      //    reset.reset
		.refclk   (<connected-to-refclk>),   //   refclk.clk
		.locked   (<connected-to-locked>),   //   locked.export
		.lvds_clk (<connected-to-lvds_clk>), // lvds_clk.lvds_clk
		.loaden   (<connected-to-loaden>),   //   loaden.loaden
		.outclk_2 (<connected-to-outclk_2>)  //  outclk2.clk
	);

