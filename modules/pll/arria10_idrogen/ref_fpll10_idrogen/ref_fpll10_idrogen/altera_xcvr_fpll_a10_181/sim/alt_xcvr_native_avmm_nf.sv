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

module alt_xcvr_native_avmm_nf #(
    parameter CHANNELS        = 1,
    parameter RECONFIG_SHARED = 0,
    parameter JTAG_ENABLED    = 0,
    parameter ADME_SLAVE_MAP  = "altera_xcvr_native_a10",
    parameter ADME_ASSGN_MAP  = " ",
    // The following are not intended to be directly set
    parameter IFACES          = RECONFIG_SHARED ? 1 : CHANNELS,
    parameter ADDR_BITS       = 9,
    parameter SEL_BITS        = (RECONFIG_SHARED ? clogb2(CHANNELS-1) : 0)
  ) (
  // Reconfig interface ports
  input   wire  [IFACES-1:0]    reconfig_clk,
  input   wire  [IFACES-1:0]    reconfig_reset,
  input   wire  [IFACES-1:0]    reconfig_write,
  input   wire  [IFACES-1:0]    reconfig_read,
  input   wire  [IFACES*(ADDR_BITS+SEL_BITS)-1:0] reconfig_address,
  input   wire  [IFACES*32-1:0] reconfig_writedata,
  output  wire  [IFACES*32-1:0] reconfig_readdata,
  output  wire  [IFACES-1:0]    reconfig_waitrequest,

  // AVMM ports to transceiver
  output  wire  [CHANNELS-1:0]            avmm_clk,
  output  wire  [CHANNELS-1:0]            avmm_reset,
  output  wire  [CHANNELS-1:0]            avmm_write,
  output  wire  [CHANNELS-1:0]            avmm_read,
  output  wire  [CHANNELS*ADDR_BITS-1:0]  avmm_address,
  output  wire  [CHANNELS*8-1:0]          avmm_writedata,
  input   wire  [CHANNELS*8-1:0]          avmm_readdata,
  input   wire  [CHANNELS-1:0]            avmm_waitrequest
);

// AVMM connections from the interface sharing logic to the JTAG arbitration
wire  [IFACES-1:0]    arb_write;
wire  [IFACES-1:0]    arb_read;
wire  [IFACES*(ADDR_BITS+SEL_BITS)-1:0] arb_address;
wire  [IFACES*32-1:0] arb_writedata;
wire  [IFACES*32-1:0] arb_readdata;
wire  [IFACES-1:0]    arb_waitrequest;

// Set the slave type for the ADME.  Since the span neesd to be a string, 2^(total addr_bits) will
// give the max value, however since the adme uses byte alignment, shift the span by two bits.
localparam set_slave_span = int2str(2**(ADDR_BITS+SEL_BITS+2));
localparam set_slave_map = {"{typeName ",ADME_SLAVE_MAP," address 0x0 span ",set_slave_span," hpath {}",ADME_ASSGN_MAP,"}"};

genvar ig;

//***************************************************************************
//********************** Embedded JTAG Debug Master *************************
generate
if(!JTAG_ENABLED) begin : g_no_jtag
  assign  arb_address   = reconfig_address;
  assign  arb_write     = reconfig_write;
  assign  arb_read      = reconfig_read;
  assign  arb_writedata = reconfig_writedata;
  
  assign  reconfig_readdata   = arb_readdata;
  assign  reconfig_waitrequest= arb_waitrequest;
end else begin : g_jtag
  reg sel;  // Arbitration bit

  wire [(ADDR_BITS+SEL_BITS)-1:0] jtag_address;
  wire [31:0] jtag_readdata;
  wire        jtag_read;
  wire        jtag_write;
  wire [31:0] jtag_writedata;
  wire        jtag_waitrequest;
  wire        jtag_readdatavalid;


  // When doing RTL sims, remove the altera_debug_master_endpoint, as 
  // there is no RTL simulation model.  Pre and Post Fit sims are ok.
  `ifdef ALTERA_RESERVED_QIS
    altera_debug_master_endpoint
    #(
      .ADDR_WIDTH                     ( (ADDR_BITS+SEL_BITS) ),
      .DATA_WIDTH                     ( 32 ),
      .HAS_RDV                        ( 0 ),
      .SLAVE_MAP                      ( set_slave_map ),
      .PREFER_HOST                    ( " " ),
      .CLOCK_RATE_CLK                 ( 0 )
    ) adme (
      .clk                                   (reconfig_clk),
      .reset                                 (reconfig_reset),
      .master_write                          (jtag_write),
      .master_read                           (jtag_read),
      .master_address                        (jtag_address),
      .master_writedata                      (jtag_writedata),
      .master_waitrequest                    (jtag_waitrequest),
      .master_readdatavalid                  (jtag_readdatavalid),
      .master_readdata                       (jtag_readdata)
    );
  `else
    assign jtag_write       = 1'b0;
    assign jtag_read        = 1'b0;
    assign jtag_writedata   = 32'b0;
    assign jtag_address     = {ADDR_BITS+SEL_BITS{1'b0}};
  `endif

  //************************************************************************
  //*********************** JTAG<->Reconfig Arbitration ********************
  // Drop the lower two address bits from the jtag master (byte addressed)
  assign  arb_address   = sel ? jtag_address[(ADDR_BITS+SEL_BITS-1):0] : reconfig_address;
  assign  arb_write     = sel ? jtag_write      : reconfig_write;
  assign  arb_read      = sel ? jtag_read       : reconfig_read;
  assign  arb_writedata = sel ? jtag_writedata  : reconfig_writedata;
  
  assign  reconfig_readdata   = arb_readdata;
  assign  jtag_readdata       = arb_readdata;
  assign  reconfig_waitrequest= arb_waitrequest | sel;
  assign  jtag_waitrequest    = arb_waitrequest | ~sel;

  // Arbitration
  always @(posedge reconfig_clk or posedge reconfig_reset)
    if(reconfig_reset)  sel <= 1'b0;
    else begin
      if(sel)           sel <= arb_waitrequest;
      else              sel <= (jtag_write|jtag_read) & ~(reconfig_write|reconfig_read);
    end
  //********************* End JTAG<->Reconfig Arbitration ******************
  //************************************************************************
end 
endgenerate

//******************** End Embedded JTAG Debug Master ***********************
//***************************************************************************


//***************************************************************************
//********************** AVMM Reconfig Connections **************************
generate
  if(!RECONFIG_SHARED) begin : g_not_shared
    // We wire straight between the interfaces if there is no sharing logic
    assign  avmm_clk        = reconfig_clk;
    assign  avmm_reset      = reconfig_reset;
    assign  avmm_write      = arb_write;
    assign  avmm_read       = arb_read;
    assign  avmm_address    = arb_address;
    assign  arb_waitrequest = avmm_waitrequest;
    
    for(ig=0;ig<CHANNELS;ig=ig+1) begin : g_shared
      assign  avmm_writedata[ig*8 +:8]    = arb_writedata[ig*32 +: 8];
      assign  arb_readdata  [ig*32 +:32]  = {24'd0,avmm_readdata[ig*8 +: 8]};
    end

  end else begin : g_shared
    wire  [SEL_BITS-1:0]  arb_sel;

    assign  arb_sel = arb_address[ADDR_BITS+:SEL_BITS];

    for(ig=0;ig<CHANNELS;ig=ig+1) begin : g_shared
      assign  avmm_clk      [ig]  = reconfig_clk;
      assign  avmm_reset    [ig]  = reconfig_reset;
      // Use the upper address bits as the interface select if shared
      assign  avmm_write    [ig]                         = arb_write & (arb_sel == ig);
      assign  avmm_read     [ig]                         = arb_read  & (arb_sel == ig);
      assign  avmm_address  [ig*ADDR_BITS +: ADDR_BITS]  = arb_address[0+:ADDR_BITS];
      assign  avmm_writedata[ig*8 +: 8]                  = arb_writedata[7:0];
    end

      assign  arb_readdata    = {24'd0,avmm_readdata[arb_sel*8 +: 8]};
      assign  arb_waitrequest = avmm_waitrequest[arb_sel];
  end
endgenerate

//********************** AVMM Reconfig Connections **************************
//***************************************************************************

////////////////////////////////////////////////////////////////////
// Return the number of bits required to represent an integer
// E.g. 0->1; 1->1; 2->2; 3->2 ... 31->5; 32->6
//
function integer clogb2;
  input integer input_num;
  begin
    for (clogb2=0; input_num>0; clogb2=clogb2+1)
      input_num = input_num >> 1;
    if(clogb2 == 0)
      clogb2 = 1;
  end
endfunction

// Returns an inst as a string for using string concatenation
function [30*8-1:0] int2str(
  input integer in_int
);
  integer i;
  integer this_char;
  i = 0;
  int2str = "";
  do
  begin
    this_char = (in_int % 10) + 48;
    int2str[i*8+:8] = this_char[7:0];
    i=i+1;
    in_int = in_int / 10;
  end
  while(in_int > 0);
endfunction

endmodule

