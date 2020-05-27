	asmi10 u0 (
		.addr          (<connected-to-addr>),          //          addr.addr
		.busy          (<connected-to-busy>),          //          busy.busy
		.clkin         (<connected-to-clkin>),         //         clkin.clk
		.data_valid    (<connected-to-data_valid>),    //    data_valid.data_valid
		.datain        (<connected-to-datain>),        //        datain.datain
		.dataout       (<connected-to-dataout>),       //       dataout.dataout
		.en4b_addr     (<connected-to-en4b_addr>),     //     en4b_addr.en4b_addr
		.fast_read     (<connected-to-fast_read>),     //     fast_read.fast_read
		.illegal_erase (<connected-to-illegal_erase>), // illegal_erase.illegal_erase
		.illegal_write (<connected-to-illegal_write>), // illegal_write.illegal_write
		.rden          (<connected-to-rden>),          //          rden.rden
		.rdid_out      (<connected-to-rdid_out>),      //      rdid_out.rdid_out
		.read_address  (<connected-to-read_address>),  //  read_address.read_address
		.read_rdid     (<connected-to-read_rdid>),     //     read_rdid.read_rdid
		.read_status   (<connected-to-read_status>),   //   read_status.read_status
		.reset         (<connected-to-reset>),         //         reset.reset
		.sce           (<connected-to-sce>),           //           sce.sce
		.sector_erase  (<connected-to-sector_erase>),  //  sector_erase.sector_erase
		.shift_bytes   (<connected-to-shift_bytes>),   //   shift_bytes.shift_bytes
		.status_out    (<connected-to-status_out>),    //    status_out.status_out
		.wren          (<connected-to-wren>),          //          wren.wren
		.write         (<connected-to-write>),         //         write.write
		.read_dummyclk (<connected-to-read_dummyclk>)  // read_dummyclk.read_dummyclk
	);

