// (C) 2001-2018 Intel Corporation. All rights reserved.
// Your use of Intel Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files from any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Intel Program License Subscription 
// Agreement, Intel FPGA IP License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Intel and sold by 
// Intel or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


module alt_xcvr_pll_avmm_csr #(
  parameter dbg_capability_reg_enable   = 0,
  parameter dbg_user_identifier         = 0,
  parameter dbg_stat_soft_logic_enable  = 0,
  parameter dbg_ctrl_soft_logic_enable  = 0,
  parameter en_master_cgb               = 0,
  parameter rcfg_emb_strm_enable        = 0,
  parameter rcfg_emb_strm_cfg_sel_width = 2
) (
  // avmm signals
  input               avmm_clk,
  input               avmm_reset,
  input  [8:0]        avmm_address,
  input  [7:0]        avmm_writedata,
  input               avmm_write,
  input               avmm_read,
  output reg [7:0]    avmm_readdata,
  output              avmm_waitrequest,

  // PLL Status signal
  input               pll_powerdown,
  input               pll_locked,
  input               pll_cal_busy,
  input               avmm_busy,

 // embedded reconfig signals
  input                                   rcfg_emb_strm_busy,
  input                                   rcfg_emb_strm_chan_sel,
  output [rcfg_emb_strm_cfg_sel_width-1:0] rcfg_emb_strm_cfg_sel,
  output                                  rcfg_emb_strm_bcast_en,
  output                                  rcfg_emb_strm_cfg_load,

  // PLL Control Signals
  output              csr_pll_powerdown
);

// Import package with parameters for the soft addresses and offsets
import a10_avmm_h::*;

// Reg for generating waitrequest and data valid
reg         avmm_valid;

/**********************************************************************/
// wires and bus declaration
/**********************************************************************/
wire [7:0]  rd_system_id;
wire [7:0]  rd_status_en;
wire [7:0]  rd_control_en;
wire [7:0]  rd_mcgb_en;
wire [7:0]  rd_ctrl_pll_lock;
wire [7:0]  rd_pll_reset;
wire [7:0]  rd_rcfg_emb_ctrl;
wire [7:0]  rd_rcfg_emb_status;

/**********************************************************************/
//generate waitrequest
/**********************************************************************/
assign avmm_waitrequest = (~avmm_valid & avmm_read);

/**********************************************************************/
// soft CSRs for embedded debug
/**********************************************************************/
always@(posedge avmm_clk) begin
  if(~avmm_read) begin
    avmm_valid    <= 1'b0;
    avmm_readdata <= RD_UNUSED;
  end else begin
    avmm_valid    <= avmm_waitrequest;
    case(avmm_address)
      A10_XR_ADDR_ID_0:       avmm_readdata <= rd_system_id;
      A10_XR_ADDR_STATUS_EN:  avmm_readdata <= rd_status_en;
      A10_XR_ADDR_CONTROL_EN: avmm_readdata <= rd_control_en;
      A10_XR_ADDR_PLL_MCGB_EN:avmm_readdata <= rd_mcgb_en;

      A10_XR_ADDR_GP_PLL_LOCK:avmm_readdata <= rd_ctrl_pll_lock;

      A10_XR_ADDR_GP_PLL_RST: avmm_readdata <= rd_pll_reset;

      //Embedded reconfig
      A10_XR_ADDR_EMBED_RCFG_CTRL:   avmm_readdata <= rd_rcfg_emb_ctrl;
      A10_XR_ADDR_EMBED_RCFG_STATUS: avmm_readdata <= rd_rcfg_emb_status;

      default:                avmm_readdata <= RD_UNUSED;
    endcase
  end
end


/**********************************************************************/
// Capability Registers
/**********************************************************************/
generate if(dbg_capability_reg_enable == 1) begin: enable_pll_capability_reg
  assign rd_system_id   = dbg_user_identifier[7:0];
  assign rd_status_en   = dbg_stat_soft_logic_enable[7:0];
  assign rd_control_en  = dbg_ctrl_soft_logic_enable[7:0];
  assign rd_mcgb_en     = en_master_cgb[7:0];
end else begin
  assign rd_system_id   = RD_UNUSED; 
  assign rd_status_en   = RD_UNUSED; 
  assign rd_control_en  = RD_UNUSED; 
  assign rd_mcgb_en     = RD_UNUSED; 
end
endgenerate


/**********************************************************************/
// Generate registers for status signals
/**********************************************************************/
generate if(dbg_stat_soft_logic_enable == 1) begin: en_stat_reg

    /**********************************************************************/
    // wires for synchronizers
    /**********************************************************************/
    wire pll_cal_busy_sync;
    wire pll_locked_sync;
    reg  r_avmm_busy;


    /**********************************************************************/
    // readback data at OFFSET synchronize the incoming signals
    /**********************************************************************/
    alt_xcvr_resync #(
      .SYNC_CHAIN_LENGTH ( 3 ),
      .WIDTH             ( 2 )  // two bits, one for locktodata and one for locktoref
    ) rx_is_locked_sync (
      .clk                       (avmm_clk),
      .reset                     (avmm_reset),
      .d                         ({pll_cal_busy, pll_locked}),
      .q                         ({pll_cal_busy_sync, pll_locked_sync}) 
    );


    assign rd_ctrl_pll_lock[A10_XR_OFFSET_GP_LOCK]      = pll_locked_sync;
    assign rd_ctrl_pll_lock[A10_XR_OFFSET_GP_CAL_BUSY]  = pll_cal_busy_sync;
    assign rd_ctrl_pll_lock[A10_XR_OFFSET_GP_AVMM_BUSY] = r_avmm_busy;
    assign rd_ctrl_pll_lock[A10_XR_OFFSET_LOCK_UNUSED+:A10_XR_LOCK_UNUSED_LEN] = {A10_XR_LOCK_UNUSED_LEN{1'b0}};

    always@(posedge avmm_clk) begin
      r_avmm_busy           <= avmm_busy;
    end

  end else begin
    assign rd_ctrl_pll_lock   = RD_UNUSED;
  end
endgenerate


/**********************************************************************/
// Generate registers for control signals
/**********************************************************************/
generate if(dbg_ctrl_soft_logic_enable == 1) begin: en_ctrl_reg
    
    // register for embedded debug-driven powerdown
    reg r_pll_reset;
    reg r_pll_reset_override;

    // readback control signals for the pll powerdown
    assign rd_pll_reset[A10_XR_OFFSET_PLL_RST]        = r_pll_reset;
    assign rd_pll_reset[A10_XR_OFFSET_PLL_RST_OVR]    = r_pll_reset_override;
    assign rd_pll_reset[A10_XR_OFFSET_PLL_RST_UNUSED+:A10_XR_PLL_RST_UNUSED_LEN] = {A10_XR_PLL_RST_UNUSED_LEN{1'b0}};

    // assign the output control signal to the pll
    assign csr_pll_powerdown = (rd_pll_reset[A10_XR_OFFSET_PLL_RST_OVR]) ? rd_pll_reset[A10_XR_OFFSET_PLL_RST] : pll_powerdown;

    // write control registers for pll_powerodwn
    always@(posedge avmm_clk or posedge avmm_reset) begin
      if(avmm_reset) begin
        r_pll_reset           <= 1'b0;
        r_pll_reset_override  <= 1'b0;
      end else if(avmm_write && avmm_address == A10_XR_ADDR_GP_PLL_RST) begin
        r_pll_reset           <= avmm_writedata[A10_XR_OFFSET_PLL_RST];
        r_pll_reset_override  <= avmm_writedata[A10_XR_OFFSET_PLL_RST_OVR];
      end
    end
  end else begin
    // assign pll powerdown when the ctrl registers arn't used
    assign rd_pll_reset       = RD_UNUSED;
    assign csr_pll_powerdown  = (pll_powerdown);
  end
endgenerate

/**********************************************************************/
// Embedded reconfig registers
/**********************************************************************/
generate if(rcfg_emb_strm_enable) begin: en_rcfg_reg

    /**********************************************************************/
    // Generate registers and wires for the reconfig soft logic
    /**********************************************************************/
    reg [rcfg_emb_strm_cfg_sel_width-1:0] r_rcfg_emb_strm_cfg_sel;
    reg                                   r_rcfg_emb_strm_cfg_load;
    reg                                   r_rcfg_emb_strm_bcast_en;
    reg                                   rcfg_emb_strm_cfg_load_lock = 1'b0;
  
    // readback the embedded reconfig control
    assign rd_rcfg_emb_ctrl               = {r_rcfg_emb_strm_cfg_load, r_rcfg_emb_strm_bcast_en, {(A10_XR_EMBED_RCFG_CFG_SEL_LEN-rcfg_emb_strm_cfg_sel_width){1'b0}}, r_rcfg_emb_strm_cfg_sel};
    assign rd_rcfg_emb_status             = {7'b0, rcfg_emb_strm_busy};
  
    // assign the output signals to the channel
    assign rcfg_emb_strm_cfg_sel          = r_rcfg_emb_strm_cfg_sel;
    assign rcfg_emb_strm_cfg_load         = r_rcfg_emb_strm_cfg_load;
    assign rcfg_emb_strm_bcast_en         = r_rcfg_emb_strm_bcast_en;
  
    always@(posedge avmm_clk or posedge avmm_reset) begin
      if(avmm_reset) begin
        r_rcfg_emb_strm_cfg_sel           <= {rcfg_emb_strm_cfg_sel_width{1'b0}};
        r_rcfg_emb_strm_cfg_load          <= 1'b0;
        r_rcfg_emb_strm_bcast_en          <= 1'b0;
        rcfg_emb_strm_cfg_load_lock       <= 1'b0;
      end else if(avmm_write && avmm_address == A10_XR_ADDR_EMBED_RCFG_CTRL) begin
        // Write to this register
        r_rcfg_emb_strm_cfg_sel           <= avmm_writedata[A10_XR_OFFSET_EMBED_RCFG_CFG_SEL +: rcfg_emb_strm_cfg_sel_width ]; 
        r_rcfg_emb_strm_cfg_load          <= avmm_writedata[A10_XR_OFFSET_EMBED_RCFG_CFG_LOAD]; 
        r_rcfg_emb_strm_bcast_en          <= avmm_writedata[A10_XR_OFFSET_EMBED_RCFG_BCAST_EN]; 
      end else if(rcfg_emb_strm_chan_sel & rcfg_emb_strm_busy & ~rcfg_emb_strm_cfg_load_lock) begin
        // Reset the cfg_load bit when the streaming has started
        r_rcfg_emb_strm_cfg_load          <= 1'b0;
        rcfg_emb_strm_cfg_load_lock       <= 1'b1;
      end else if(~rcfg_emb_strm_busy & rcfg_emb_strm_cfg_load_lock)
        rcfg_emb_strm_cfg_load_lock       <= 1'b0;
    end
  end else begin: g_rcfg_reg_dis
    assign rd_rcfg_emb_ctrl               = RD_UNUSED;
    assign rd_rcfg_emb_status             = RD_UNUSED;
    assign rcfg_emb_strm_cfg_sel          = 1'b0; 
    assign rcfg_emb_strm_bcast_en         = 1'b0; 
    assign rcfg_emb_strm_cfg_load         = 1'b0;
  end
endgenerate //End generate g_rcfg_reg

endmodule
