	ref_pll10 u0 (
		.rst              (<connected-to-rst>),              //            reset.reset
		.refclk           (<connected-to-refclk>),           //           refclk.clk
		.locked           (<connected-to-locked>),           //           locked.export
		.outclk_0         (<connected-to-outclk_0>),         //          outclk0.clk
		.outclk_1         (<connected-to-outclk_1>),         //          outclk1.clk
		.outclk_2         (<connected-to-outclk_2>),         //          outclk2.clk
		.outclk_3         (<connected-to-outclk_3>),         //          outclk3.clk
		.outclk_4         (<connected-to-outclk_4>),         //          outclk4.clk
		.scanclk          (<connected-to-scanclk>),          //          scanclk.scanclk
		.phase_en         (<connected-to-phase_en>),         //         phase_en.phase_en
		.updn             (<connected-to-updn>),             //             updn.updn
		.cntsel           (<connected-to-cntsel>),           //           cntsel.cntsel
		.phase_done       (<connected-to-phase_done>),       //       phase_done.phase_done
		.num_phase_shifts (<connected-to-num_phase_shifts>)  // num_phase_shifts.num_phase_shifts
	);

