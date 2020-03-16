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


`timescale 1 ps/1 ps

package a10_avmm_h;

  // localparam to define unused bus
  localparam RD_UNUSED                  = 8'h0;

  // localparams for common capability registers
  localparam A10_XR_ADDR_ID_0           = 9'h0;
  localparam A10_XR_ADDR_ID_1           = 9'h1;
  localparam A10_XR_ADDR_ID_2           = 9'h2;
  localparam A10_XR_ADDR_ID_3           = 9'h3;
  localparam A10_XR_ADDR_STATUS_EN      = 9'h4;
  localparam A10_XR_ADDR_CONTROL_EN     = 9'h5;
  // Reserve Address 9'h6 to 9'hF for common capablities

  // native phy capability
  localparam A10_XR_ADDR_NAT_CHNLS      = 9'h10;
  localparam A10_XR_ADDR_NAT_CHNL_NUM   = 9'h11;
  localparam A10_XR_ADDR_NAT_DUPLEX     = 9'h12;
  localparam A10_XR_ADDR_NAT_PRBS_EN    = 9'h13;
  localparam A10_XR_ADDR_NAT_ODI_EN     = 9'h14;

  // pll ip capability
  localparam A10_XR_ADDR_PLL_MCGB_EN    = 9'h10;

  // localparams for csr for pll locked and cal busy
  localparam A10_XR_ADDR_GP_PLL_LOCK    = 9'h80;
  localparam A10_XR_OFFSET_GP_LOCK      = 0;
  localparam A10_XR_OFFSET_GP_CAL_BUSY  = 1;
  localparam A10_XR_OFFSET_GP_AVMM_BUSY = 2;
  localparam A10_XR_OFFSET_LOCK_UNUSED  = 3;
  localparam A10_XR_LOCK_UNUSED_LEN     = 5;

  // localparams for pll powerdown
  localparam A10_XR_ADDR_GP_PLL_RST     = 9'hE0;
  localparam A10_XR_OFFSET_PLL_RST      = 0;
  localparam A10_XR_OFFSET_PLL_RST_OVR  = 1;
  localparam A10_XR_OFFSET_PLL_RST_UNUSED = 2;
  localparam A10_XR_PLL_RST_UNUSED_LEN  = 6;

  // localparams for csr for lock to ref and lock to data
  localparam A10_XR_ADDR_GP_RD_LTR      = 9'h80;
  localparam A10_XR_OFFSET_RD_LTD       = 0;
  localparam A10_XR_OFFSET_RD_LTR       = 1;
  localparam A10_XR_OFFSET_LTR_UNUSED   = 2;
  localparam A10_XR_LTR_UNUSED_LEN      = 6;

  // localparams for csr for cal busy
  localparam A10_XR_ADDR_GP_CAL_BUSY    = 9'h81;
  localparam A10_XR_OFFSET_TX_CAL_BUSY  = 0;
  localparam A10_XR_OFFSET_RX_CAL_BUSY  = 1;
  localparam A10_XR_OFFSET_AVMM_BUSY    = 2;
  localparam A10_XR_OFFSET_CAL_DUMMY    = 3;
  localparam A10_XR_OFFSET_TX_CAL_MASK  = 4;
  localparam A10_XR_OFFSET_RX_CAL_MASK  = 5;
  localparam A10_XR_OFFSET_CAL_UNUSED   = 6;
  localparam A10_XR_CAL_UNUSED_LEN      = 2;

  // localparams for setting lock to ref and lock to data
  localparam A10_XR_ADDR_GP_SET_LTR     = 9'hE0;
  localparam A10_XR_OFFSET_SET_LTD      = 0;
  localparam A10_XR_OFFSET_SET_LTR      = 1;
  localparam A10_XR_OFFSET_SET_LTD_OVR  = 2;
  localparam A10_XR_OFFSET_SET_LTR_OVR  = 3;
  localparam A10_XR_OFFSET_SET_LTR_UNUSED = 4;
  localparam A10_XR_SET_LTR_UNUSED_LEN   = 4;

  // localparams for setting loopback
  localparam A10_XR_ADDR_GP_LPBK        = 9'hE1;
  localparam A10_XR_OFFSET_LPBK         = 0;
  localparam A10_XR_OFFSET_LPBK_UNUSED  = 1;
  localparam A10_XR_LPBK_UNUSED_LEN     = 7;

  // localparams for setting channel resets
  localparam A10_XR_ADDR_CHNL_RESET     = 9'hE2;
  localparam A10_XR_OFFSET_RX_ANA       = 0; 
  localparam A10_XR_OFFSET_RX_DIG       = 1; 
  localparam A10_XR_OFFSET_TX_ANA       = 2; 
  localparam A10_XR_OFFSET_TX_DIG       = 3; 
  localparam A10_XR_OFFSET_RX_ANA_OVR   = 4; 
  localparam A10_XR_OFFSET_RX_DIG_OVR   = 5; 
  localparam A10_XR_OFFSET_TX_ANA_OVR   = 6; 
  localparam A10_XR_OFFSET_TX_DIG_OVR   = 7; 

  // localparams for prbs addresses
  localparam A10_XR_ADDR_PRBS_CTRL      = 9'h100;
  localparam A10_XR_ADDR_PRBS_ERR_0     = 9'h101;
  localparam A10_XR_ADDR_PRBS_ERR_1     = 9'h102;
  localparam A10_XR_ADDR_PRBS_ERR_2     = 9'h103;
  localparam A10_XR_ADDR_PRBS_ERR_3     = 9'h104;
  localparam A10_XR_ADDR_PRBS_ERR_4     = 9'h105;
  localparam A10_XR_ADDR_PRBS_ERR_5     = 9'h106;
  localparam A10_XR_ADDR_PRBS_ERR_6     = 9'h107;
  localparam A10_XR_ADDR_PRBS_BIT_0     = 9'h10D;
  localparam A10_XR_ADDR_PRBS_BIT_1     = 9'h10E;
  localparam A10_XR_ADDR_PRBS_BIT_2     = 9'h10F;
  localparam A10_XR_ADDR_PRBS_BIT_3     = 9'h110;
  localparam A10_XR_ADDR_PRBS_BIT_4     = 9'h111;
  localparam A10_XR_ADDR_PRBS_BIT_5     = 9'h112;
  localparam A10_XR_ADDR_PRBS_BIT_6     = 9'h113;
  
  // localparams for prbs bit offsets
  localparam A10_XR_OFFSET_PRBS_EN      = 0;
  localparam A10_XR_OFFSET_PRBS_RESET   = 1;
  localparam A10_XR_OFFSET_PRBS_SNAP    = 2;
  localparam A10_XR_OFFSET_PRBS_DONE    = 3;
  localparam A10_XR_OFFSET_PRBS_UNUSED  = 4;
  localparam A10_XR_PRBS_UNUSED_LEN     = 4;

  // localparams for odi addresses
  localparam A10_XR_ADDR_ODI_CTRL       = 9'h120;
  localparam A10_XR_ADDR_ODI_ERR_0      = 9'h121;
  localparam A10_XR_ADDR_ODI_ERR_1      = 9'h122;
  localparam A10_XR_ADDR_ODI_ERR_2      = 9'h123;
  localparam A10_XR_ADDR_ODI_ERR_3      = 9'h124;
  localparam A10_XR_ADDR_ODI_ERR_4      = 9'h125;
  localparam A10_XR_ADDR_ODI_ERR_5      = 9'h126;
  localparam A10_XR_ADDR_ODI_ERR_6      = 9'h127;
  localparam A10_XR_ADDR_ODI_BIT_0      = 9'h12D;
  localparam A10_XR_ADDR_ODI_BIT_1      = 9'h12E;
  localparam A10_XR_ADDR_ODI_BIT_2      = 9'h12F;
  localparam A10_XR_ADDR_ODI_BIT_3      = 9'h130;
  localparam A10_XR_ADDR_ODI_BIT_4      = 9'h131;
  localparam A10_XR_ADDR_ODI_BIT_5      = 9'h132;
  localparam A10_XR_ADDR_ODI_BIT_6      = 9'h133;

  // localparams for odi bit offsets
  localparam A10_XR_OFFSET_ODI_EN       = 0;
  localparam A10_XR_OFFSET_ODI_RESET    = 1;
  localparam A10_XR_OFFSET_ODI_SNAP     = 2;
  localparam A10_XR_OFFSET_ODI_DONE     = 3;
  localparam A10_XR_OFFSET_ODI_UNUSED   = 4;
  localparam A10_XR_ODI_UNUSED_LEN      = 4;

  // localparams for embedded reconfig addresses
  // Control reg and offsets
  localparam A10_XR_ADDR_EMBED_RCFG_CTRL        = 9'h140;
  localparam A10_XR_OFFSET_EMBED_RCFG_CFG_SEL   = 0;
  localparam A10_XR_EMBED_RCFG_CFG_SEL_LEN      = 6; //bits [5:0] are alloted for cfg_sel even though GUI currently only supports upto 8 profiles.

  localparam A10_XR_OFFSET_EMBED_RCFG_BCAST_EN  = 6;
  localparam A10_XR_OFFSET_EMBED_RCFG_CFG_LOAD  = 7;

  // Status reg and offsets
  localparam A10_XR_ADDR_EMBED_RCFG_STATUS      = 9'h141;
  localparam A10_XR_OFFSET_EMBED_RCFG_STRM_BUSY = 0;


endpackage
