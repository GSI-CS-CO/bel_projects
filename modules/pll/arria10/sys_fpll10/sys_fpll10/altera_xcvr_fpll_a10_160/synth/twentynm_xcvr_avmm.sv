// (C) 2001-2016 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera MegaCore Function License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


`timescale 1ps/1ps


module twentynm_xcvr_avmm
  #(
    //PARAM_LIST_START
    parameter avmm_interfaces     = 1,         //Number of AVMM interfaces required - one for each bonded_lane, PLL, and Master CGB
    parameter rcfg_enable         = 0,         //Enable/disable reconfig interface in the Native PHY or PLL IP
    parameter enable_avmm         = 1,         //Enable/disable AVMM atom instantiation
    parameter arbiter_ctrl        = "pld",     //Calibration request at start-up. Valid values: "uc","pld".
                                               //"uc" =Initial calibration needed at start-up. Internal DPRIO interface is controlled by uC.
                                               //"pld"=Initial calibration is not needed at start-up. Internal DPRIO interface is controlled by PLD.
    parameter calibration_en      = "disable", //Indicates whether calibration by Hard Nios is enabled or not.Should be set to DISABLE in case if Nios is absent or needs to be bypassed. Valid values: enable, disable
    parameter avmm_busy_en        = "disable", //Provides a separate interface for determining control of the AVMM bus, and separates its behavior from the avmm_waitreqeust
    parameter hip_cal_en          = "disable", //Indicates whether HIP is enabled or not. Valid values: disable, enable
    parameter cal_done            = "cal_done_assert"   //Indicates whether calibration is done. This is the start-up value for the corresponding CRAM. THe CRAM is eventually accessed and updated by the Hard uC during calibration. Valid values: cal_done_assert, cal_done_deassert
    //PARAM_LIST_END
  )  (
    //PORT_LIST_START
    // AVMM slave interface signals (user)
    input  wire  [avmm_interfaces-1     :0] avmm_clk,
    input  wire  [avmm_interfaces-1     :0] avmm_reset,
    input  wire  [avmm_interfaces*8-1   :0] avmm_writedata, 
    input  wire  [avmm_interfaces*9-1   :0] avmm_address,
    input  wire  [avmm_interfaces-1     :0] avmm_write,
    input  wire  [avmm_interfaces-1     :0] avmm_read,
    output wire  [avmm_interfaces*8-1   :0] avmm_readdata, 
    output wire  [avmm_interfaces-1     :0] avmm_waitrequest,
    //AVMM interface busy with calibration
    output wire  [avmm_interfaces-1     :0] avmm_busy,
    // Calibration Done
    output wire  [avmm_interfaces-1     :0] hip_cal_done, //To HIP
    output wire  [avmm_interfaces-1     :0] pld_cal_done, //To PLD
    
    // Channel/PLL AVMM interface signals (from AVMM atom fanned-out to all the Channel/PLL atoms)
    output  wire  [avmm_interfaces-1    :0] chnl_pll_avmm_clk,
    output  wire  [avmm_interfaces-1    :0] chnl_pll_avmm_rstn,
    output  wire  [avmm_interfaces*8-1  :0] chnl_pll_avmm_writedata, //Internal AVMM interface is 8-bit wide
    output  wire  [avmm_interfaces*9-1  :0] chnl_pll_avmm_address,
    output  wire  [avmm_interfaces-1    :0] chnl_pll_avmm_write,
    output  wire  [avmm_interfaces-1    :0] chnl_pll_avmm_read,
   
    // PMA AVMM signals 
    input   wire  [avmm_interfaces*8-1  :0] pma_avmmreaddata_tx_ser,                 // TX SER readdata               (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pma_avmmreaddata_tx_cgb,                 // TX Slave CGB readdata         (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pma_avmmreaddata_tx_buf,                 // TX BUF readdata               (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pma_avmmreaddata_rx_deser,               // RX Deser readdata             (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pma_avmmreaddata_rx_buf,                 // RX BUF readdata               (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pma_avmmreaddata_rx_sd,                  // RX SD readdata                (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pma_avmmreaddata_rx_odi,                 // RX ODI readdata               (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pma_avmmreaddata_rx_dfe,                 // RX DFE readdata               (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pma_avmmreaddata_cdr_pll,                // CDR readdata               (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pma_avmmreaddata_cdr_refclk_select,      // CDR refclk mux readdata    (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pma_avmmreaddata_pma_adapt,              // PMA adaptation readdata    (8 for each lane)

    input   wire  [avmm_interfaces-1    :0] pma_blockselect_tx_ser,                  // TX SER blockselect            (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pma_blockselect_tx_cgb,                  // TX Slave CGB blockselect      (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pma_blockselect_tx_buf,                  // TX BUF blockselect            (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pma_blockselect_rx_deser,                // RX Deser blockselect          (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pma_blockselect_rx_buf,                  // RX BUF blockselect            (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pma_blockselect_rx_sd,                   // RX SD blockselect             (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pma_blockselect_rx_odi,                  // RX ODI blockselect            (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pma_blockselect_rx_dfe,                  // RX DFE blockselect            (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pma_blockselect_cdr_pll,                 // CDR blockselect            (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pma_blockselect_cdr_refclk_select,       // CDR refclk mux blockselect (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pma_blockselect_pma_adapt,               // PMA adaptation blockselect (1 for each lane)

    // PCS Channel AVMM signals 
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_8g_rx_pcs,              // 8G RX PCS readdata            (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_pipe_gen1_2,            // Gen1/2 PIPE readdata          (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_8g_tx_pcs,              // 8G TX PCS readdata            (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_10g_rx_pcs,             // 10G RX PCS readdata           (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_10g_tx_pcs,             // 10G TX PCS readdata           (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_gen3_rx_pcs,            // GEN3 RX PCS readdata          (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_pipe_gen3,              // GEN3 PIPE readdata            (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_gen3_tx_pcs,            // GEN3 TX PCS readdata          (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_krfec_rx_pcs,           // KRFEC RX PCS readdata         (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_krfec_tx_pcs,           // KRFEC TX PCS readdata         (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_fifo_rx_pcs,            // FIFO RX PCS readdata          (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_fifo_tx_pcs,            // FIFO TX PCS readdata          (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_rx_pcs_pld_if,          // RX PCS PLD IF readdata        (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_com_pcs_pld_if,         // COM PCS PLD IF readdata       (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_tx_pcs_pld_if,          // TX PCS PLD IF readdata        (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_rx_pcs_pma_if,          // RX PCS PMA IF readdata        (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_com_pcs_pma_if,         // COm PCS PMA IF readdata       (8 for each lane)
    input   wire  [avmm_interfaces*8-1  :0] pcs_avmmreaddata_tx_pcs_pma_if,          // TX PCS PMA IF readdata        (8 for each lane)

    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_8g_rx_pcs,               // 8G RX PCS blockselect         (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_pipe_gen1_2,             // Gen1/2 PIPE blockselect       (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_8g_tx_pcs,               // 8G TX PCS blockselect         (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_10g_rx_pcs,              // 10G RX PCS blockselect        (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_10g_tx_pcs,              // 10G TX PCS blockselect        (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_gen3_rx_pcs,             // GEN3 RX PCS blockselect       (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_pipe_gen3,               // GEN3 PIPE blockselect         (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_gen3_tx_pcs,             // GEN3 TX PCS blockselect       (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_krfec_rx_pcs,            // KRFEC RX PCS blockselect      (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_krfec_tx_pcs,            // KRFEC TX PCS blockselect      (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_fifo_rx_pcs,             // FIFO RX PCS blockselect       (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_fifo_tx_pcs,             // FIFO TX PCS blockselect       (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_rx_pcs_pld_if,           // RX PCS PLD IF blockselect     (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_com_pcs_pld_if,          // COM PCS PLD IF blockselect    (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_tx_pcs_pld_if,           // TX PCS PLD IF blockselect     (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_rx_pcs_pma_if,           // RX PCS PMA IF blockselect     (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_com_pcs_pma_if,          // COM PCS PMA IF blockselect    (1 for each lane)
    input   wire  [avmm_interfaces-1    :0] pcs_blockselect_tx_pcs_pma_if,           // TX PCS PMA IF blockselect     (1 for each lane)

    // PLL AVMM signals
    input   wire  [avmm_interfaces*8-1  :0] pll_avmmreaddata_lc_pll,                 // LC PLL readdata                  (8 for each PLL)
    input   wire  [avmm_interfaces*8-1  :0] pll_avmmreaddata_lc_refclk_select,       // LC Refclk Select Mux readdata    (8 for each PLL)
    input   wire  [avmm_interfaces*8-1  :0] pll_avmmreaddata_cgb_master,             // Master CGB readdata              (8 for each PLL)
    input   wire  [avmm_interfaces*8-1  :0] pll_avmmreaddata_cmu_fpll,               // CMU-FPLL readdata                (8 for each PLL)
    input   wire  [avmm_interfaces*8-1  :0] pll_avmmreaddata_cmu_fpll_refclk_select, // CMU-FPLL refclk mux readdata     (8 for each PLL)
    // CDR Tx PLL will connect to pma_avmmreaddata_cdr_pll and pma_avmmreaddata_cdr_refclk_select 
    input   wire  [avmm_interfaces-1    :0] pll_blockselect_lc_pll,                  // LC PLL blockselect               (1 for each PLL)
    input   wire  [avmm_interfaces-1    :0] pll_blockselect_lc_refclk_select,        // LC Refclk Select Mux blockselect (1 for each PLL)
    input   wire  [avmm_interfaces-1    :0] pll_blockselect_cgb_master,              // Master CGB blockselect           (1 for each PLL)
    input   wire  [avmm_interfaces-1    :0] pll_blockselect_cmu_fpll,                // CMU-FPLL blockselect             (1 for each PLL)
    input   wire  [avmm_interfaces-1    :0] pll_blockselect_cmu_fpll_refclk_select   // CMU-FPLL refclk mux blockselect  (1 for each PLL)
    // CDR Tx PLL will connect to pma_blockselect_cdr_pll and pma_blockselect_cdr_refclk_select 

    //PORT_LIST_END
  );

wire  [avmm_interfaces-1  :0] avmm_reset_sync;
reg   [avmm_interfaces-1  :0] avmm_read_r ={avmm_interfaces{1'b0}};
wire  [avmm_interfaces-1  :0] avmm_waitrequest_read;
wire  [avmm_interfaces-1  :0] avmm_waitrequest_write;
reg   [avmm_interfaces-1  :0] avmm_waitrequest_write_int;
reg   [avmm_interfaces*3-1:0] avmm_read_cycles_cnt;     
reg   [avmm_interfaces-1  :0] avmm_busy_r1 = {avmm_interfaces{1'b0}};
reg   [avmm_interfaces-1  :0] avmm_busy_r2 = {avmm_interfaces{1'b0}};

wire  [avmm_interfaces-1  :0] avmm_request;

localparam calibration_type             = "one_time"; //Not used for now. Virtual attribute with no associated BCM settings. Intended to be set by the IP based on the chosen calibration option and used only by the AVMM sim model to decide whether to release the AVMM interface to the user after start-up or hold on to it until the user request according to the hardware implementation.  Valid values: one_time, continuous. Export as parameter if needed.

localparam AVMM_READ_LATENCY            = 3'b011; //Read latency in the hardware
localparam AVMM_READ_CYCLES_CNT_RST_VAL = 3'b111; //Reset value of avmm_read_cycles_cnt. Non-zero and >AVMM_READ_LATENCY for avmm_waitrequest to assert during reset 
localparam ARBITER_BASE_ADDR            = 9'h000; //AVMM<->uC arbiter base address
  
generate begin
  genvar ig;
  genvar jg;
  for(ig=0;ig<avmm_interfaces;ig=ig+1) begin : avmm_atom_insts
     // Size of blockselect bus as defined in the AVMM atom = Number of HSSI atoms + some buffer = 70
     wire  [70-1:0]   chnl_pll_avmm_blockselect;
     wire  [7:0]      chnl_pll_avmm_readdata [0:(70-1)];
     wire  [70*8-1:0] chnl_pll_avmm_readdatabus;

    // Assign the incoming avmmreaddata and blockselect signals from all the atoms to 
    // the readdata bus and blockselect bus of the AVMM atom respectively
    for(jg = 0; jg < 70; jg = jg + 1) begin:avmm_assigns
        assign  chnl_pll_avmm_readdatabus[jg*8+:8] = chnl_pll_avmm_readdata[jg];
        assign  {chnl_pll_avmm_readdata[jg],chnl_pll_avmm_blockselect[jg]} = 
          // TX PMA connections
          (jg ==  0) ? {pma_avmmreaddata_tx_ser                  [ig*8+:8],pma_blockselect_tx_ser                  [ig]} :
          (jg ==  1) ? {pma_avmmreaddata_tx_cgb                  [ig*8+:8],pma_blockselect_tx_cgb                  [ig]} :
          (jg ==  2) ? {pma_avmmreaddata_tx_buf                  [ig*8+:8],pma_blockselect_tx_buf                  [ig]} :
          // RX PMA (includes CDR) connections
          (jg ==  3) ? {pma_avmmreaddata_rx_deser                [ig*8+:8],pma_blockselect_rx_deser                [ig]} :
          (jg ==  4) ? {pma_avmmreaddata_rx_buf                  [ig*8+:8],pma_blockselect_rx_buf                  [ig]} :
          (jg ==  5) ? {pma_avmmreaddata_rx_sd                   [ig*8+:8],pma_blockselect_rx_sd                   [ig]} :
          (jg ==  6) ? {pma_avmmreaddata_rx_odi                  [ig*8+:8],pma_blockselect_rx_odi                  [ig]} :
          (jg ==  7) ? {pma_avmmreaddata_rx_dfe                  [ig*8+:8],pma_blockselect_rx_dfe                  [ig]} :
          (jg ==  8) ? {pma_avmmreaddata_cdr_pll                 [ig*8+:8],pma_blockselect_cdr_pll                 [ig]} : 
          (jg ==  9) ? {pma_avmmreaddata_cdr_refclk_select       [ig*8+:8],pma_blockselect_cdr_refclk_select       [ig]} : 
          (jg ==  10) ? {pma_avmmreaddata_pma_adapt              [ig*8+:8],pma_blockselect_pma_adapt               [ig]} : 
          // PCS connections
          (jg ==  20) ? {pcs_avmmreaddata_8g_rx_pcs              [ig*8+:8],pcs_blockselect_8g_rx_pcs               [ig]} :
          (jg ==  21) ? {pcs_avmmreaddata_pipe_gen1_2            [ig*8+:8],pcs_blockselect_pipe_gen1_2             [ig]} :
          (jg ==  22) ? {pcs_avmmreaddata_8g_tx_pcs              [ig*8+:8],pcs_blockselect_8g_tx_pcs               [ig]} :
          (jg ==  23) ? {pcs_avmmreaddata_10g_rx_pcs             [ig*8+:8],pcs_blockselect_10g_rx_pcs              [ig]} :
          (jg ==  24) ? {pcs_avmmreaddata_10g_tx_pcs             [ig*8+:8],pcs_blockselect_10g_tx_pcs              [ig]} :
          (jg ==  25) ? {pcs_avmmreaddata_gen3_rx_pcs            [ig*8+:8],pcs_blockselect_gen3_rx_pcs             [ig]} :
          (jg ==  26) ? {pcs_avmmreaddata_pipe_gen3              [ig*8+:8],pcs_blockselect_pipe_gen3               [ig]} :
          (jg ==  27) ? {pcs_avmmreaddata_gen3_tx_pcs            [ig*8+:8],pcs_blockselect_gen3_tx_pcs             [ig]} :
          (jg ==  28) ? {pcs_avmmreaddata_krfec_rx_pcs           [ig*8+:8],pcs_blockselect_krfec_rx_pcs            [ig]} :
          (jg ==  29) ? {pcs_avmmreaddata_krfec_tx_pcs           [ig*8+:8],pcs_blockselect_krfec_tx_pcs            [ig]} :
          (jg ==  30) ? {pcs_avmmreaddata_fifo_rx_pcs            [ig*8+:8],pcs_blockselect_fifo_rx_pcs             [ig]} :
          (jg ==  31) ? {pcs_avmmreaddata_fifo_tx_pcs            [ig*8+:8],pcs_blockselect_fifo_tx_pcs             [ig]} :
          (jg ==  32) ? {pcs_avmmreaddata_rx_pcs_pld_if          [ig*8+:8],pcs_blockselect_rx_pcs_pld_if           [ig]} :
          (jg ==  33) ? {pcs_avmmreaddata_com_pcs_pld_if         [ig*8+:8],pcs_blockselect_com_pcs_pld_if          [ig]} :
          (jg ==  34) ? {pcs_avmmreaddata_tx_pcs_pld_if          [ig*8+:8],pcs_blockselect_tx_pcs_pld_if           [ig]} :
          (jg ==  35) ? {pcs_avmmreaddata_rx_pcs_pma_if          [ig*8+:8],pcs_blockselect_rx_pcs_pma_if           [ig]} :
          (jg ==  36) ? {pcs_avmmreaddata_com_pcs_pma_if         [ig*8+:8],pcs_blockselect_com_pcs_pma_if          [ig]} :
          (jg ==  37) ? {pcs_avmmreaddata_tx_pcs_pma_if          [ig*8+:8],pcs_blockselect_tx_pcs_pma_if           [ig]} :
          // PLL connections
          (jg ==  50) ? {pll_avmmreaddata_lc_pll                 [ig*8+:8],pll_blockselect_lc_pll                  [ig]} :
          (jg ==  51) ? {pll_avmmreaddata_lc_refclk_select       [ig*8+:8],pll_blockselect_lc_refclk_select        [ig]} :
          (jg ==  52) ? {pll_avmmreaddata_cgb_master             [ig*8+:8],pll_blockselect_cgb_master              [ig]} :
          (jg ==  53) ? {pll_avmmreaddata_cmu_fpll               [ig*8+:8],pll_blockselect_cmu_fpll                [ig]} :
          (jg ==  54) ? {pll_avmmreaddata_cmu_fpll_refclk_select [ig*8+:8],pll_blockselect_cmu_fpll_refclk_select  [ig]} :
          {8'd0,1'b0}; //unused indices
      end //avmm_assigns
 
      if (enable_avmm == 1) begin 
        // AVMM atom
        twentynm_hssi_avmm_if 
        #(
           .arbiter_ctrl        (arbiter_ctrl                        ),             
           .calibration_en      (calibration_en                      ),             
           .hip_cal_en          (hip_cal_en                          ),             
           .cal_done            (cal_done                            ),             
           .calibration_type    (calibration_type                    )
         ) twentynm_hssi_avmm_if_inst
         (
           .avmmrstn            (1'b1                                ),  // Tie-off
           .avmmclk             (avmm_clk                 [ig]       ),  
           .avmmwrite           (avmm_write               [ig]       ),
           .avmmread            (avmm_read                [ig]       ),
           .avmmaddress         (avmm_address             [ig*9+:9]  ),
           .avmmwritedata       (avmm_writedata           [ig*8+:8]  ),
           .avmmreaddata        (avmm_readdata            [ig*8+:8]  ),
           .avmmbusy            (avmm_busy                [ig]       ), 
           .avmmrequest         (avmm_request             [ig]       ), 
           .hipcaldone          (hip_cal_done             [ig]       ), 
           .pldcaldone          (pld_cal_done             [ig]       ), 
           
           .clkchnl             (chnl_pll_avmm_clk        [ig]       ),
           .rstnchnl            (chnl_pll_avmm_rstn       [ig]       ),
           .writechnl           (chnl_pll_avmm_write      [ig]       ),
           .readchnl            (chnl_pll_avmm_read       [ig]       ),
           .regaddrchnl         (chnl_pll_avmm_address    [ig*9+:9]  ),
           .writedatachnl       (chnl_pll_avmm_writedata  [ig*8+:8]  ),
           
           .readdatachnl        (chnl_pll_avmm_readdatabus           ),
           .blockselect         (chnl_pll_avmm_blockselect           ),
          
           .scanmoden           (1'b1                                ),  
           .scanshiftn          (1'b1                                ),  
           .avmmreservedin      (/*unused*/                          ),  
           .avmmreservedout     (/*unused*/                          )   
       );
      
      //***********************************************************************************
      //
      // Generate avmm_waitrequest
      //
      //***********************************************************************************
      // AVMM slave in hardware:
      //  avmm_write -> variable latency 
      //    arbiter register = multi-cycle latency with avmm_busy asserted 
      //    other registers  = fixed 0-cycle latency
      //  avmm_read  -> fixed 3-cycle latency 
      
      // Synchronize the falling edge of incoming reconfig reset
      // Waitrequest is asserted during reset
      alt_xcvr_resync #(
          .SYNC_CHAIN_LENGTH(2),  // Number of flip-flops for retiming
          .WIDTH            (1),  // Number of bits to resync
          .INIT_VALUE       (1'b1)
      ) avmm_reset_sync_inst (
        .clk    (avmm_clk[ig]       ),
        .reset  (avmm_reset[ig]     ),
        .d      (1'b0               ),
        .q      (avmm_reset_sync[ig])
      );
     
      //***********************************************************************************
      //**************** Generate waitrequest for read operation *************************
      // Register avmm_read
      always @ (posedge avmm_clk[ig] or posedge avmm_reset_sync[ig]) begin
        if (avmm_reset_sync[ig]) begin
          avmm_read_r[ig]         <= 1'b0; 
        end else begin
          avmm_read_r[ig]         <= avmm_read[ig];
        end
      end

      // Pipeline registers for avmm_busy 
      // Do not reset these registers so they can track avmm_busy accurately 
      always @ (posedge avmm_clk[ig]) begin
        avmm_busy_r1[ig]         <= avmm_busy[ig];
        avmm_busy_r2[ig]         <= avmm_busy_r1[ig];
      end

      // Read counter
      // - reset the counter to a value between 0 and AVMM_READ_LATENCY so that waitrequest is asserted during reset
      // - after reset de-assertion, flip the counter back to 0 for waitrequest to be de-asserted
      // - increment the count:
      //   - when avmm_read is asserted 
      //   - when avmm_busy_r1 is not asserted => the read operation immediately after recalibration request is served after
      //     avmm_busy_r1 is deasserted
       //   - until AVMM_READ_LATENCY count
      always @ (posedge avmm_clk[ig] or posedge avmm_reset_sync[ig]) begin  
        if (avmm_reset_sync[ig])
          avmm_read_cycles_cnt[ig*3+:3] <= AVMM_READ_CYCLES_CNT_RST_VAL;     
        else begin
          if (~avmm_busy_r1[ig] & avmm_read[ig] & (avmm_read_cycles_cnt[ig*3+:3] == 3'b000))
            avmm_read_cycles_cnt[ig*3+:3] <= avmm_read_cycles_cnt[ig*3+:3] + 1'b1; 
          else if (~avmm_busy_r1[ig] & (avmm_read_cycles_cnt[ig*3+:3] > 3'b000 && avmm_read_cycles_cnt[ig*3+:3] <= AVMM_READ_LATENCY))
            avmm_read_cycles_cnt[ig*3+:3] <= avmm_read_cycles_cnt[ig*3+:3] + 1'b1; 
          else 
            avmm_read_cycles_cnt[ig*3+:3] <= 3'b000; 
        end
      end   

      // waitrequest for read
      assign avmm_waitrequest_read[ig] = (avmm_read[ig] & !avmm_read_r[ig]) |  //when avmm_read is asserted
                                         (avmm_read_cycles_cnt[ig*3+:3] > 3'b000 && avmm_read_cycles_cnt[ig*3+:3] <= AVMM_READ_LATENCY) | //during current read and possible back-to-back reads wihtout avmm_read being deasserted
                                         (avmm_read_cycles_cnt[ig*3+:3] == AVMM_READ_CYCLES_CNT_RST_VAL) ; //during reset
      
      //***********************************************************************************
      //**************** Generate waitrequest for write operation ************************
      // Logic enabled only when the calibration feature is enabled
      // Assertion   - very next clock cycle after receiving the arbiter register write request -> 0x0[0] = 1'b1. 
      //             - this write indicates that the user wants to give-up the AVMM interface so that the Microcontroller can perform calibration (recalibration or adaptive).
      // Deassertion - two clock cycles after avmm_busy is deasserted 
      if(calibration_en == "enable") begin : g_cal_en_wreq
        always @ (posedge avmm_clk[ig] or posedge avmm_reset_sync[ig]) begin
          if (avmm_reset_sync[ig])
            avmm_waitrequest_write_int[ig] <= 1'b0;
          else begin
            if ((avmm_write[ig] == 1'b1) && (avmm_address[ig*9+:9] == ARBITER_BASE_ADDR) && (avmm_writedata[ig*8] == 1'b1) && (!avmm_waitrequest_write[ig]))
              avmm_waitrequest_write_int[ig] <= 1'b1;
            else if (avmm_waitrequest_write_int[ig] & ~avmm_busy_r1[ig] & ~avmm_busy_r2[ig])
              avmm_waitrequest_write_int[ig] <= 1'b1;
            else
              avmm_waitrequest_write_int[ig] <= avmm_busy_r1[ig];
          end             
        end
        assign avmm_waitrequest_write[ig] = avmm_waitrequest_write_int[ig];
      end else begin : g_cal_dis_wreq
        assign avmm_waitrequest_write[ig] = 1'b0;
      end 

      //***********************************************************************************
      // user waitrequest
      // case: 250182
      // Allow de-coupling the avmm_busy from the avmm waitrequest to support nios and CPU based 
      // reconfiguration sequences and masters.  This will address issues with returning the 
      // avmm bus back to the user, which in turn, due to the long waitrequest, causes the system
      // to timeout.
      if ( avmm_busy_en == "enable" ) begin
        assign avmm_waitrequest[ig] = avmm_waitrequest_read[ig];
      end else begin
        assign avmm_waitrequest[ig] = avmm_waitrequest_write[ig] | avmm_waitrequest_read[ig];
      end


      //***************************************************************************************************************
      //
      // Generate avmm_request 
      //
      // When Tx term resistance calibration is enabled in continuous mode, the usemodel
      // is that the Nios will hold the AVMM interface until requested by the user. 
      // The user will request the AVMM interface, perform reconfiguration, and return
      // the interface back to the Nios to continue with the adaptive calibration.

      // Offset 0x0: 
      //   bit 0 = arbiter control (0=pld, 1=uc)
      //   bit 1 = cal_done status (0=not done, 1=done) --> updated by Nios. Should not be touched by the user.
      //
      // If reconfig interface is enabled in the PHY GUI:
      //   * if calibration is enabled and continuous mode is set
      //     Steps to access AVMM interface 
      //     1. User requests the AVMM interface -> offset=0x0, RMW bit 0 = 0  
      //        --> Internally, this module generates avmm_request signal that maps to a status register bit in hardware
      //        --> Nios reads this bit if the continuous calibration is enabled and gives up the AVMM interface
      //     2. When waitrequest is de-asserted, user performs reconfiguration
      //     3. User requests recalibration and thereby give up the AVMM interface to Nios
      //         -> offset=0x0, RMW bit 0 = 1
      //   * if either calibration is disabled or continuous mode is not set
      //     tie-off avmm_request to 1'b1 => park the avmm_request bit to always be in the user "request" mode
      //
      // If reconfig interface is NOT enabled in the PHY GUI:
      //     tie-off avmm_request to 1'b0 => park the avmm_request bit to always be in the "not request" mode => allow Nios to do adaptive calibration
      
      //***************************************************************************************************************
      // Assertion   - very next clock cycle after receiving the arbiter register write request -> 0x0[0] = 1'b0
      //             - this write indicates that the user wants to get the AVMM interface from the Microcontroller so that the user can perform reconfiguration.
      // Deassertion - after avmm_busy goes low 
      //             - this indicates that the user has received the AVMM interface
      if(rcfg_enable == 1) begin: g_rcfg_en
        if ((calibration_en == "enable") && (calibration_type == "continuous")) begin : g_cal_en_req
          reg   [avmm_interfaces-1  :0] avmm_request_int = 1'b0;
          
          always @ (posedge avmm_clk[ig] or posedge avmm_reset_sync[ig]) begin
            if (avmm_reset_sync[ig])
              avmm_request_int[ig] <= 1'b0;
            else begin
              if ((avmm_write[ig] == 1'b1) && (avmm_address[ig*9+:9] == ARBITER_BASE_ADDR) && (avmm_writedata[ig*8] == 1'b0) && (!avmm_request_int[ig]))
                avmm_request_int[ig] <= 1'b1;
              else if (~avmm_busy_r2[ig])
                avmm_request_int[ig] <= 1'b0;
            end   
          end
          assign avmm_request[ig] = avmm_request_int[ig];          
        end else begin: g_cal_dis_req
          //Park the interface request favoring the user AVMM 
          assign avmm_request[ig] = 1'b1; 
        end
      end else begin : g_rcfg_dis
        //Park the interface request favoring the Nios if reconfiguration is not enabled by the user
        assign avmm_request[ig] = 1'b0; 
      end 
      
      end else begin //if !(enable_avmm), tie-off AVMM atom outputs
        assign  chnl_pll_avmm_clk       [ig]      = 1'b0;
        assign  chnl_pll_avmm_rstn      [ig]      = 1'b1;
        assign  chnl_pll_avmm_writedata [ig*8+:8] = 8'd0;
        assign  chnl_pll_avmm_address   [ig*9+:9] = 9'd0;
        assign  chnl_pll_avmm_write     [ig]      = 1'b0;
        assign  chnl_pll_avmm_read      [ig]      = 1'b0;
     end
  end
end
endgenerate

endmodule
