--_____________________________________________________________________________|
--                             VME TO WB INTERFACE                             |
--                                                                             |
--                                CERN,BE/CO-HT                                |
--_____________________________________________________________________________|
-- File:                      xVME64xCore_Top.vhd                              |
--_____________________________________________________________________________|
-- Description:
-- Wrapper for the VME64xCore_Top, for documentation about the core see VME64xCore_Top.vhd
--______________________________________________________________________________
-- Authors:                                      
--               Cesar Prados Boda (c.prados@gsi.de)                             
-- Date         7/2013                                                                           
-- Version      v0.03  
--______________________________________________________________________________
--                               GNU LESSER GENERAL PUBLIC LICENSE                                
--                              ------------------------------------       
-- Copyright (c) 2009 - 2011 CERN                        
-- This source file is free software; you can redistribute it and/or modify it under the terms of 
-- the GNU Lesser General Public License as published by the Free Software Foundation; either     
-- version 2.1 of the License, or (at your option) any later version.                             
-- This source is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;       
-- without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     
-- See the GNU Lesser General Public License for more details.                                    
-- You should have received a copy of the GNU Lesser General Public License along with this       
-- source; if not, download it from http://www.gnu.org/licenses/lgpl-2.1.html                     
---------------------------------------------------------------------------------------

  library IEEE;
  use IEEE.STD_LOGIC_1164.all;
  use IEEE.numeric_std.all;
  use work.xvme64x_pack.all;
  use work.wishbone_pkg.all;
  use work.VME_CR_pack.all;
  use work.VME_Buffer_pack.all;

--===========================================================================
-- Entity declaration
--===========================================================================
  entity xVME64xCore_Top is
    generic(
      -- clock period (ns)
      g_clock          : integer := c_clk_period;        -- 100 MHz
      --WB data width:
      g_wb_data_width  : integer := c_width;             -- must be 32 or 64
      --WB address width:
      g_wb_addr_width  : integer := c_addr_width;        -- 64 or less
      -- CRAM 
      g_cram_size      : integer := c_CRAM_SIZE;
      -- Board ID; each board shall have an unique ID. eg: SVEC_ID = 408.
      -- loc: 0x33, 0x37, 0x3B, 0x3F   CR space
      g_BoardID        : integer := c_SVEC_ID;           -- 4 bytes: 0x00000198
      -- Manufacturer ID: eg the CERN ID is 0x080030
      -- loc: 0x27, 0x2B, 0x2F   CR space
      g_ManufacturerID : integer := c_CERN_ID;           -- 3 bytes: 0x080030
      -- Revision ID
      -- loc: 0x43, 0x47, 0x4B, 0x4F   CR space
      g_RevisionID     : integer := c_RevisionID;        -- 4 bytes: 0x00000001
      -- Program ID: this is the firmware ID
      -- loc: 0x7f    CR space
      g_ProgramID      : integer := 90;                  -- 1 byte : 0x5a 
      -- VME base address setting 
      g_base_addr      : base_addr  := GEOGRAPHICAL_ADDR     -- MECHANICALLY or , legacy
	 );
   port(
     clk_i            : in std_logic;              
     -- for the IRQ_Generator and relative registers 
     reset_o          : out std_logic;   -- asserted when '1'
     -- VME                            
     VME_AS_n_i       : in    std_logic;
     VME_RST_n_i      : in    std_logic;  -- asserted when '0'
     VME_WRITE_n_i    : in    std_logic;
     VME_AM_i         : in    std_logic_vector(5 downto 0);
     VME_DS_n_i       : in    std_logic_vector(1 downto 0);
     VME_GA_i         : in    std_logic_vector(5 downto 0);
     VME_BERR_o       : out   std_logic;  -- [In the VME standard this line is asserted when low.
	                                       -- Here is asserted when high indeed the logic will be 
														-- inverted again in the VME transceivers on the board]*.
     VME_DTACK_n_o    : out   std_logic;
     VME_RETRY_n_o    : out   std_logic;
     VME_LWORD_n_i    : in    std_logic;
	  VME_LWORD_n_o    : out   std_logic;
     VME_ADDR_i       : in    std_logic_vector(31 downto 1);
	  VME_ADDR_o       : out   std_logic_vector(31 downto 1);
     VME_DATA_i       : in    std_logic_vector(31 downto 0);
	  VME_DATA_o       : out   std_logic_vector(31 downto 0);
     VME_IRQ_o        : out   std_logic_vector(6 downto 0);  -- the same as []*
     VME_IACKIN_n_i   : in    std_logic;
     VME_IACK_n_i     : in    std_logic;
     VME_IACKOUT_n_o  : out   std_logic;

     -- VME buffers
     VME_DTACK_OE_o   : out   std_logic;
     
     --VME_DATA_BUFF_o  : out   t_VME_BUFFER;                                          
     --VME_ADDR_BUFF_o  : out   t_VME_BUFFER;
     VME_BUFFER_o     : out   t_VME_BUFFER;

     --VME_DATA_DIR_o   : out   std_logic;
     --VME_DATA_OE_N_o  : out   std_logic;
     --VME_ADDR_DIR_o   : out   std_logic;
     --VME_ADDR_OE_N_o  : out   std_logic;

     VME_RETRY_OE_o   : out   std_logic;
     
	  -- WishBone
     master_o         : out t_wishbone_master_out;
     master_i         : in  t_wishbone_master_in;

     -- IRQ Generator
     INT_ack_o        : out   std_logic;   -- when the IRQ controller acknowledges the Interrupt
	                                        -- cycle it sends a pulse to the IRQ Generator
     IRQ_i            : in    std_logic;   -- Interrupt request; the IRQ Generator/your Wb application
                                           -- sends a pulse to the IRQ Controller which asserts one of 
														 -- the IRQ lines.
     -- Added by Davide for debug:
     debug            : out   std_logic_vector(7 downto 0)
    );

  end xVME64xCore_Top;

--===========================================================================
-- Architecture declaration
--===========================================================================

  architecture RTL of xVME64xCore_Top is
  
  signal s_CRAMdataOut             : std_logic_vector(7 downto 0);
  signal s_CRAMaddr                : std_logic_vector(f_log2_size(g_cram_size)-1 downto 0);
  signal s_CRAMdataIn              : std_logic_vector(7 downto 0);
  signal s_CRAMwea                 : std_logic;
  signal s_CRaddr                  : std_logic_vector(11 downto 0);
  signal s_CRdata                  : std_logic_vector(7 downto 0);
  signal s_RW                      : std_logic;
  signal s_reset                   : std_logic;
  signal s_IRQlevelReg             : std_logic_vector(7 downto 0);
  signal s_FIFOreset               : std_logic;
  signal s_VME_DATA_IRQ            : std_logic_vector(31 downto 0);
  signal s_VME_DATA_VMEbus         : std_logic_vector(31 downto 0);
  signal s_VME_DATA_b              : std_logic_vector(31 downto 0);
  signal s_fifo                    : std_logic;
  signal s_VME_DTACK_VMEbus        : std_logic;
  signal s_VME_DTACK_IRQ           : std_logic;
  signal s_VME_DTACK_OE_VMEbus     : std_logic;
  signal s_VME_DTACK_OE_IRQ        : std_logic;
  --signal s_VME_DATA_DIR_VMEbus     : std_logic;
  --signal s_VME_DATA_BUFF_VMEbus    : t_VME_BUFFER;
  --signal s_VME_DATA_DIR_IRQ        : std_logic;
  --signal s_VME_DATA_BUFF_IRQ       : t_VME_BUFFER;
  signal s_VME_BUFFER_VMEbus       : t_VME_BUFFER;
  signal s_VME_BUFFER_IRQ          : t_VME_BUFFER;
  signal s_INT_Level               : std_logic_vector(7 downto 0);
  signal s_INT_Vector              : std_logic_vector(7 downto 0);
  signal s_VME_IRQ_n_o             : std_logic_vector(6 downto 0);
  signal s_reset_IRQ               : std_logic;
  signal s_CSRData_o               : std_logic_vector(7 downto 0);
  signal s_CSRData_i               : std_logic_vector(7 downto 0);
  signal s_CrCsrOffsetAddr         : std_logic_vector(18 downto 0);
  signal s_Ader0                   : std_logic_vector(31 downto 0);
  signal s_Ader1                   : std_logic_vector(31 downto 0);
  signal s_Ader2                   : std_logic_vector(31 downto 0);
  signal s_Ader3                   : std_logic_vector(31 downto 0);
  signal s_Ader4                   : std_logic_vector(31 downto 0);
  signal s_Ader5                   : std_logic_vector(31 downto 0);
  signal s_Ader6                   : std_logic_vector(31 downto 0);
  signal s_Ader7                   : std_logic_vector(31 downto 0);
  signal s_en_wr_CSR               : std_logic;
  signal s_err_flag                : std_logic;
  signal s_reset_flag              : std_logic;
  signal s_Sw_Reset                : std_logic;
  signal s_ModuleEnable            : std_logic;
  signal s_Endian                  : std_logic_vector(2 downto 0);
  signal s_BAR                     : std_logic_vector(4 downto 0);
  signal s_time                    : std_logic_vector(39 downto 0);
  signal s_bytes                   : std_logic_vector(12 downto 0);
  signal s_IRQ                     : std_logic;
  
  -- Oversampled input signals 
  signal VME_RST_n_oversampled     : std_logic;
  signal VME_AS_n_oversampled      : std_logic;   
  signal VME_AS_n_oversampled1     : std_logic;  -- for the IRQ_Controller
  --signal VME_LWORD_n_oversampled   : std_logic;
  signal VME_WRITE_n_oversampled   : std_logic;
  signal VME_DS_n_oversampled      : std_logic_vector(1 downto 0);
  signal VME_DS_n_oversampled_1    : std_logic_vector(1 downto 0);
  signal VME_GA_oversampled        : std_logic_vector(5 downto 0);   
  signal VME_IACK_n_oversampled    : std_logic;
  signal VME_IACKIN_n_oversampled  : std_logic;
  signal s_reg_1                    : std_logic_vector(1 downto 0);
  signal s_reg_2                    : std_logic_vector(1 downto 0);
--===========================================================================
-- Architecture begin
--===========================================================================
begin
---------------------METASTABILITY-----------------------------------------
  -- Input oversampling & edge detection; oversampling the input data is necessary to avoid 
  -- metastability problems. With 3 samples the probability of metastability problem will 
  -- be very low but of course the transfer rate will be slow down a little. 
  GAinputSample : RegInputSample
  generic map(
              width => 6
            )
  port map(
             reg_i => VME_GA_i,
             reg_o => VME_GA_oversampled,
             clk_i => clk_i
           );

--  DSinputSample : RegInputSample
  RegInputSample : process(clk_i)
	begin
		if rising_edge(clk_i) then
			s_reg_1 <= VME_DS_n_i;
			s_reg_2 <= s_reg_1;	
			VME_DS_n_oversampled <= s_reg_2;	 
		end if;
  end process;
			
-- to avoid timing problem during BLT and MBLT accesses
  VME_DS_n_oversampled_1 <= s_reg_2;
  
  WRITEinputSample : SigInputSample
  port map(
            sig_i => VME_WRITE_n_i,
            sig_o => VME_WRITE_n_oversampled,
            clk_i => clk_i
         );
		  
  ASinputSample : SigInputSample
  port map(
            sig_i => VME_AS_n_i,
            sig_o => VME_AS_n_oversampled,
            clk_i => clk_i
        );

  RSTinputSample : SigInputSample
  port map(
            sig_i => VME_RST_n_i,
            sig_o => VME_RST_n_oversampled,
            clk_i => clk_i
         );

  IACKinputSample : SigInputSample
  port map(
            sig_i => VME_IACK_n_i,
            sig_o => VME_IACK_n_oversampled,
            clk_i => clk_i
         ); 
			
  IACKINinputSample : SigInputSample
     port map(
              sig_i => VME_IACKIN_n_i,
              sig_o => VME_IACKIN_n_oversampled,
              clk_i => clk_i
            );			
				
  IrqrisingEdge : RisEdgeDetection
  port map (
              sig_i      => IRQ_i,
              clk_i      => clk_i,
              RisEdge_o  => s_IRQ
          );
				
  Inst_VME_bus: VME_bus 
  generic map(
              g_clock          => g_clock,
              g_wb_data_width  => g_wb_data_width,
				  g_wb_addr_width  => g_wb_addr_width, 
				  g_cram_size      => g_cram_size
           )
  port map(
       clk_i                => clk_i,
		 reset_o              => s_reset,  -- asserted when '1'
       -- VME 
		 VME_RST_n_i          => VME_RST_n_oversampled,
		 VME_AS_n_i           => VME_AS_n_oversampled,
		 VME_LWORD_n_o        => VME_LWORD_n_o,
		 VME_LWORD_n_i        => VME_LWORD_n_i,
		 VME_RETRY_n_o        => VME_RETRY_n_o,
		 VME_RETRY_OE_o       => VME_RETRY_OE_o,
		 VME_WRITE_n_i        => VME_WRITE_n_oversampled,
		 VME_DS_n_i           => VME_DS_n_oversampled,
		 VME_DS_ant_n_i       => VME_DS_n_oversampled_1,
		 VME_DTACK_n_o        => s_VME_DTACK_VMEbus,
		 VME_DTACK_OE_o       => s_VME_DTACK_OE_VMEbus,
		 VME_BERR_o           => VME_BERR_o,
		 VME_ADDR_i           => VME_ADDR_i,
		 VME_ADDR_o           => VME_ADDR_o,
       VME_BUFFER_o         => s_VME_BUFFER_VMEbus,
       --VME_ADDR_BUFF_o      => VME_ADDR_BUFF_o
		 --VME_ADDR_DIR_o       => VME_ADDR_DIR_o,
		 --VME_ADDR_OE_N_o      => VME_ADDR_OE_N_o,
		 VME_DATA_i           => VME_DATA_i,
		 VME_DATA_o           => s_VME_DATA_VMEbus,
       --VME_DATA_BUFF_o      => s_VME_DATA_BUFF_VMEbus
		 --VME_DATA_DIR_o       => s_VME_DATA_DIR_VMEbus,
		 --VME_DATA_OE_N_o      => VME_DATA_OE_N_o,
		 VME_AM_i             => VME_AM_i,
		 VME_IACK_n_i         => VME_IACK_n_oversampled,
		 -- WB
       memReq_o             => master_o.STB,
		 memAckWB_i           => master_i.ACK,
		 wbData_o             => master_o.DAT,
		 wbData_i             => master_i.DAT,
		 locAddr_o            => master_o.ADR,
		 wbSel_o              => master_o.SEL,
		 RW_o                 => s_RW,
		 cyc_o                => master_o.CYC,
		 err_i                => master_i.ERR,
		 rty_i                => master_i.RTY,
	 	 stall_i              => master_i.STALL,
		 -- CR/CSR signals
		 CRAMaddr_o           => s_CRAMaddr,
		 CRAMdata_o           => s_CRAMdataIn,
		 CRAMdata_i           => s_CRAMdataOut,
		 CRAMwea_o            => s_CRAMwea,
		 CRaddr_o             => s_CRaddr,
		 CRdata_i             => s_CRdata,
		 en_wr_CSR            => s_en_wr_CSR,
		 CrCsrOffsetAddr      => s_CrCsrOffsetAddr,
		 CSRData_o            => s_CSRData_o,
		 CSRData_i            => s_CSRData_i,
		 err_flag_o           => s_err_flag,
		 reset_flag_i         => s_reset_flag,
		 Ader0                => s_Ader0,
		 Ader1                => s_Ader1,
		 Ader2                => s_Ader2,
		 Ader3                => s_Ader3,
		 Ader4                => s_Ader4,
		 Ader5                => s_Ader5,
		 Ader6                => s_Ader6,
		 Ader7                => s_Ader7,
		 ModuleEnable         => s_ModuleEnable,
		 Endian_i             => s_Endian,
		 Sw_Reset             => s_Sw_Reset,
		 BAR_i                => s_BAR,
		 numBytes             => s_bytes,
	    transfTime           => s_time,
       -- debug
		 leds                 => debug
	       );
			 
---------------------------------------------------------------------------------
    -- output
    --VME_IRQ_o        <= not s_VME_IRQ_n_o; --The buffers will invert again the logic level
	 VME_IRQ_o        <= s_VME_IRQ_n_o; 		-- My buffers doesn't invert the logic!!!!!
    --WE_o             <= not s_RW;   
    master_o.we      <= not s_RW;   
    reset_o          <= s_reset;
    INT_ack_o        <= s_VME_DTACK_IRQ;
--------------------------------------------------------------------------------	 
    --Multiplexer added on the output signal used by either VMEbus.vhd and the IRQ_controller.vhd  
    VME_DATA_o       <= s_VME_DATA_VMEbus       when  VME_IACK_n_oversampled ='1' else 
                        s_VME_DATA_IRQ;
    VME_DTACK_n_o    <= s_VME_DTACK_VMEbus      when  VME_IACK_n_oversampled ='1' else 
                        s_VME_DTACK_IRQ;		
    VME_DTACK_OE_o   <= s_VME_DTACK_OE_VMEbus   when  VME_IACK_n_oversampled ='1' else 
                        s_VME_DTACK_OE_IRQ;					
    --VME_DATA_DIR_o   <= s_VME_DATA_DIR_VMEbus   when  VME_IACK_n_oversampled ='1' else 
    --                    s_VME_DATA_DIR_IRQ;					
    --VME_DATA_BUFF_o  <= s_VME_DATA_BUFF_VMEbus   when  VME_IACK_n_oversampled ='1' else 
    --                    s_VME_DATA_DIR_IRQ;
    VME_BUFFER_o     <= s_VME_BUFFER_VMEbus     when  VME_IACK_n_oversampled ='1' else 
                        s_VME_BUFFER_IRQ;
--------------------------------------------------------------------------------
    --  Interrupter
   Inst_VME_IRQ_Controller: VME_IRQ_Controller port map(
         		 clk_i             => clk_i,
	         	 reset_n_i         => s_reset_IRQ,  -- asserted when low
		          VME_IACKIN_n_i    => VME_IACKIN_n_oversampled,
         		 VME_AS_n_i        => VME_AS_n_oversampled,
					 VME_AS1_n_i       => VME_AS_n_i,
	          	 VME_DS_n_i        => VME_DS_n_oversampled,
        		    VME_LWORD_n_i     => VME_LWORD_n_i,
         		 VME_ADDR_123_i    => VME_ADDR_i(3 downto 1),
         		 INT_Level_i       => s_INT_Level,
         		 INT_Vector_i      => s_INT_Vector ,
	          	 INT_Req_i         => s_IRQ,
		          VME_IRQ_n_o       => s_VME_IRQ_n_o,
         		 VME_IACKOUT_n_o   => VME_IACKOUT_n_o,
         		 VME_DTACK_n_o     => s_VME_DTACK_IRQ,
         		 VME_DTACK_OE_o    => s_VME_DTACK_OE_IRQ,
         		 VME_DATA_o        => s_VME_DATA_IRQ,
         		 --VME_DATA_DIR_o    => s_VME_DATA_DIR_IRQ
                --VME_DATA_BUFF_o   => s_VME_DATA_BUFF_IRQ
                VME_BUFFER_o      => s_VME_BUFFER_IRQ
                  	);
    
    s_reset_IRQ    <= not(s_reset);
--------------------------------------------------------------------------
--------------------------------------------------------------------------
    --CR/CSR space
   Inst_VME_CR_CSR_Space: VME_CR_CSR_Space 
	generic map(
				  g_cram_size      => g_cram_size,
				  g_wb_data_width  => g_wb_data_width,
				  g_CRspace        => c_cr_array,
				  g_BoardID        => g_BoardID,
				  g_ManufacturerID => g_ManufacturerID,
				  g_RevisionID     => g_RevisionID,
				  g_ProgramID      => g_ProgramID,
				  g_base_addr		 => g_base_addr
              )
	port map(
       		 clk_i               => clk_i,
		       reset               => s_reset,
	          CR_addr             => s_CRaddr,
		       CR_data             => s_CRdata,
		       CRAM_addr           => s_CRAMaddr,
		       CRAM_data_o         => s_CRAMdataOut,
	 	       CRAM_data_i         => s_CRAMdataIn,
		       CRAM_Wen            => s_CRAMwea,
         	 en_wr_CSR           => s_en_wr_CSR,
	          CrCsrOffsetAddr     => s_CrCsrOffsetAddr,
		       VME_GA_oversampled  => VME_GA_oversampled,
		       locDataIn           => s_CSRData_o,
		       err_flag            => s_err_flag,
		       reset_flag          => s_reset_flag,
		       CSRdata             => s_CSRData_i,
		       Ader0               => s_Ader0,
		       Ader1               => s_Ader1,
		       Ader2               => s_Ader2,
		       Ader3               => s_Ader3,
		       Ader4               => s_Ader4,
		       Ader5               => s_Ader5,
		       Ader6               => s_Ader6,
		       Ader7               => s_Ader7,
		       ModuleEnable        => s_ModuleEnable,
		       Sw_Reset            => s_Sw_Reset,
		       Endian_o            => s_Endian,
		       BAR_o               => s_BAR,
		       INT_Level           => s_INT_Level,
		       numBytes            => s_bytes,
	          transfTime          => s_time,
		       INT_Vector          => s_INT_Vector
	);

  end RTL;
--===========================================================================
-- Architecture end
--===========================================================================
