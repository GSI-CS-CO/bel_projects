// arria10_scu4_lvds_rx.v

// Generated using ACDS version 18.1 625

`timescale 1 ps / 1 ps
module arria10_scu4_lvds_rx (
		input  wire       ext_coreclock, // ext_coreclock.export
		input  wire       ext_fclk,      //      ext_fclk.export
		input  wire       ext_loaden,    //    ext_loaden.export
		input  wire [0:0] rx_in,         //         rx_in.export
		output wire [7:0] rx_out         //        rx_out.export
	);

	arria10_scu4_lvds_rx_altera_lvds_181_pparsma lvds_0 (
		.rx_in         (rx_in),         //         rx_in.export
		.rx_out        (rx_out),        //        rx_out.export
		.ext_fclk      (ext_fclk),      //      ext_fclk.export
		.ext_loaden    (ext_loaden),    //    ext_loaden.export
		.ext_coreclock (ext_coreclock)  // ext_coreclock.export
	);

endmodule
