--______________________________________________________________________
--                             VME TO WB INTERFACE
--
--                                CERN,BE/CO-HT 
--______________________________________________________________________
-- File:                       vme64x_pack.vhd                              
--______________________________________________________________________
-- Authors:                                     
--               Pablo Alvarez Sanchez (Pablo.Alvarez.Sanchez@cern.ch)                             
--               Davide Pedretti       (Davide.Pedretti@cern.ch)  
-- Date         11/2012                                                                           
-- Version      v0.03 
--_______________________________________________________________________
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
use work.wishbone_pkg.all;
--use work.VME_CR_pack.all;
package vme64x_pack is
--__________________________________________________________________________________
-- Records:
   type t_rom_cell is
      record
         add : integer;
         len : integer;
      end record;
   type t_cr_add_table is array (natural range <>) of t_rom_cell;
   
   type vme_buffer_eo is  (      ADDR_BUFF,
                                 DATA_BUFF,
                                 DATA_ADDR_BUFF);
  
   type t_VME_BUFFER is
      record
			s_addrDir      :  std_logic;
			s_dataDir      :  std_logic;
			s_clk          :  std_logic;
         s_buffer_eo    :  vme_buffer_eo;
         s_latch_oe     :  std_logic;
	end record;

   type t_FSM is
      record
         s_memReq         :  std_logic;
         s_decode         :  std_logic;
         s_dtackOE        :  std_logic;
         s_mainDTACK      :  std_logic;
         s_buffer         :  t_VME_BUFFER;
         --s_dataBuf        :  t_VME_BUFFER;
         --s_addrBuf        :  t_VME_BUFFER;
         --s_dataDir        :  std_logic;
         --s_dataOE         :  std_logic;
         --s_addrDir        :  std_logic;
         --s_addrOE         :  std_logic;
         s_DSlatch        :  std_logic;
         s_incrementAddr  :  std_logic;
         s_dataPhase      :  std_logic;
         s_dataToOutput   :  std_logic;
         s_dataToAddrBus  :  std_logic;
         s_transferActive :  std_logic;
         s_2eLatchAddr    :  std_logic_vector(1 downto 0);
         s_retry          :  std_logic;
         s_berr           :  std_logic;
         s_BERR_out       :  std_logic;
   end record;
		
   type t_FSM_IRQ is
      record
			s_IACKOUT   :  std_logic;
			--s_DataDir   :  std_logic; 
         s_buffer    :  t_VME_BUFFER;
			s_DTACK     :  std_logic;
			s_enableIRQ :  std_logic;
			s_resetIRQ  :  std_logic;
			s_DSlatch   :  std_logic;
			s_DTACK_OE  :  std_logic;
	end record;   
   
  --_______________________________________________________________________________
  -- Constants:
  --SDB address
  constant c_sdb_address    : t_wishbone_address := x"00300000";
  --WB data width:
   constant c_width          : integer := 32; --must be 32 or 64!
	--CRAM size in the CR/CSR space (bytes):
	constant c_CRAM_SIZE      : integer := 1024; 
	-- remember to set properly the "END_CRAM" register in the CR space
   -- WB addr width:
	--constant c_addr_width     : integer := 9;
	constant c_addr_width     : integer := 32;
	--
   constant DFS              : integer := 2;    -- for accessing at the ADEM's bit 2
   constant XAM_MODE         : integer := 0;  -- for accessing at the ADER's bit 0
	-- Tclk in ns used to calculate the data transfer rate
   constant c_clk_period     : integer := 10;   --c_CLK_PERIOD     : std_logic_vector(19 downto 0) := "00000000000000001010";
	-- add here the default boards ID:
	constant c_SVEC_ID        : integer := 408;        -- 0x00000198
   constant c_VETAR_ID       : integer := 409;        -- 0x00000199
	constant c_CERN_ID        : integer := 524336;     -- 0x080030
	constant c_GSI_ID         : integer := 524337;     -- 0x080031
	constant c_RevisionID     : integer := 1;          -- 0x00000001
 	--BoardID positions:
	constant c_BOARD_ID_p1    : integer := 12;
	constant c_BOARD_ID_p2    : integer := 13;
	constant c_BOARD_ID_p3    : integer := 14;
	constant c_BOARD_ID_p4    : integer := 15;
	--ManufacturerID positions:
	constant c_Manuf_ID_p1    : integer := 9;
	constant c_Manuf_ID_p2    : integer := 10;
	constant c_Manuf_ID_p3    : integer := 11;
	--RevisionID positions:
	constant c_Rev_ID_p1    : integer := 16;
	constant c_Rev_ID_p2    : integer := 17;
	constant c_Rev_ID_p3    : integer := 18;
	constant c_Rev_ID_p4    : integer := 19;
	--ProgramID positions:
	constant c_Prog_ID_p    : integer := 31;
  -- AM table. 
  -- References:
  -- Table 2-3 "Address Modifier Codes" pages 21/22 VME64std ANSI/VITA 1-1994
  -- Table 2.4 "Extended Address Modifier Code" page 12 2eSST ANSI/VITA 1.5-2003(R2009)
   constant c_A24_S_sup      : std_logic_vector(5 downto 0) := "111101";     -- hex code 0x3d
   constant c_A24_S          : std_logic_vector(5 downto 0) := "111001";     -- hex code 0x39
   constant c_A24_BLT        : std_logic_vector(5 downto 0) := "111011";     -- hex code 0x3b
   constant c_A24_BLT_sup    : std_logic_vector(5 downto 0) := "111111";     -- hex code 0x3f
   constant c_A24_MBLT       : std_logic_vector(5 downto 0) := "111000";     -- hex code 0x38
   constant c_A24_MBLT_sup   : std_logic_vector(5 downto 0) := "111100";     -- hex code 0x3c
   constant c_A24_LCK        : std_logic_vector(5 downto 0) := "110010";     -- hex code 0x32
   constant c_CR_CSR         : std_logic_vector(5 downto 0) := "101111";     -- hex code 0x2f
   constant c_A16            : std_logic_vector(5 downto 0) := "101001";     -- hex code 0x29
   constant c_A16_sup        : std_logic_vector(5 downto 0) := "101101";     -- hex code 0x2d
   constant c_A16_LCK        : std_logic_vector(5 downto 0) := "101100";     -- hex code 0x2c
   constant c_A32            : std_logic_vector(5 downto 0) := "001001";     -- hex code 0x09
   constant c_A32_sup        : std_logic_vector(5 downto 0) := "001101";     -- hex code 0x0d
   constant c_A32_BLT        : std_logic_vector(5 downto 0) := "001011";     -- hex code 0x0b  
   constant c_A32_BLT_sup    : std_logic_vector(5 downto 0) := "001111";     -- hex code 0x0f
   constant c_A32_MBLT       : std_logic_vector(5 downto 0) := "001000";     -- hex code 0x08
   constant c_A32_MBLT_sup   : std_logic_vector(5 downto 0) := "001100";     -- hex code 0x0c
   constant c_A32_LCK        : std_logic_vector(5 downto 0) := "000101";     -- hex code 0x05
   constant c_A64            : std_logic_vector(5 downto 0) := "000001";     -- hex code 0x01
   constant c_A64_BLT        : std_logic_vector(5 downto 0) := "000011";     -- hex code 0x03
   constant c_A64_MBLT       : std_logic_vector(5 downto 0) := "000000";     -- hex code 0x00
   constant c_A64_LCK        : std_logic_vector(5 downto 0) := "000100";     -- hex code 0x04
   constant c_TWOedge        : std_logic_vector(5 downto 0) := "100000";     -- hex code 0x20
   constant c_A32_2eVME      : std_logic_vector(7 downto 0) := "00000001";   -- hex code 0x21
   constant c_A64_2eVME      : std_logic_vector(7 downto 0) := "00000010";   -- hex code 0x22
   constant c_A32_2eSST      : std_logic_vector(7 downto 0) := "00010001";   -- hex code 0x11
   constant c_A64_2eSST      : std_logic_vector(7 downto 0) := "00010010";   -- hex code 0x12
 --CSR array's index:
   constant BAR                 : integer := 255;
   constant BIT_SET_CLR_REG     : integer := 254;
   constant USR_BIT_SET_CLR_REG : integer := 253;
   constant CRAM_OWNER          : integer := 252;
   constant FUNC7_ADER_0        : integer := 251;
   constant FUNC7_ADER_1        : integer := FUNC7_ADER_0 - 1;
   constant FUNC7_ADER_2        : integer := FUNC7_ADER_0 - 2;
   constant FUNC7_ADER_3        : integer := FUNC7_ADER_0 - 3;
   constant FUNC6_ADER_0        : integer := FUNC7_ADER_0 - 4;
   constant FUNC6_ADER_1        : integer := FUNC7_ADER_0 - 5;
   constant FUNC6_ADER_2        : integer := FUNC7_ADER_0 - 6;
   constant FUNC6_ADER_3        : integer := FUNC7_ADER_0 - 7;
   constant FUNC5_ADER_0        : integer := FUNC7_ADER_0 - 8;
   constant FUNC5_ADER_1        : integer := FUNC7_ADER_0 - 9;
   constant FUNC5_ADER_2        : integer := FUNC7_ADER_0 - 10;
   constant FUNC5_ADER_3        : integer := FUNC7_ADER_0 - 11;
   constant FUNC4_ADER_0        : integer := FUNC7_ADER_0 - 12;
   constant FUNC4_ADER_1        : integer := FUNC7_ADER_0 - 13;
   constant FUNC4_ADER_2        : integer := FUNC7_ADER_0 - 14;
   constant FUNC4_ADER_3        : integer := FUNC7_ADER_0 - 15;
   constant FUNC3_ADER_0        : integer := FUNC7_ADER_0 - 16;
   constant FUNC3_ADER_1        : integer := FUNC7_ADER_0 - 17;
   constant FUNC3_ADER_2        : integer := FUNC7_ADER_0 - 18;
   constant FUNC3_ADER_3        : integer := FUNC7_ADER_0 - 19;
   constant FUNC2_ADER_0        : integer := FUNC7_ADER_0 - 20;
   constant FUNC2_ADER_1        : integer := FUNC7_ADER_0 - 21;
   constant FUNC2_ADER_2        : integer := FUNC7_ADER_0 - 22;
   constant FUNC2_ADER_3        : integer := FUNC7_ADER_0 - 23;
   constant FUNC1_ADER_0        : integer := FUNC7_ADER_0 - 24;
   constant FUNC1_ADER_1        : integer := FUNC7_ADER_0 - 25;
   constant FUNC1_ADER_2        : integer := FUNC7_ADER_0 - 26;
   constant FUNC1_ADER_3        : integer := FUNC7_ADER_0 - 27;
   constant FUNC0_ADER_0        : integer := FUNC7_ADER_0 - 28;
   constant FUNC0_ADER_1        : integer := FUNC7_ADER_0 - 29;
   constant FUNC0_ADER_2        : integer := FUNC7_ADER_0 - 30;
   constant FUNC0_ADER_3        : integer := FUNC7_ADER_0 - 31;
   constant IRQ_Vector          : integer := FUNC0_ADER_3 -1;
   constant IRQ_level           : integer := FUNC0_ADER_3 -2;
   constant TIME0_ns            : integer := FUNC0_ADER_3 -5;
   constant TIME1_ns            : integer := FUNC0_ADER_3 -6;
   constant TIME2_ns            : integer := FUNC0_ADER_3 -7;
   constant TIME3_ns            : integer := FUNC0_ADER_3 -8;
   constant TIME4_ns            : integer := FUNC0_ADER_3 -9;
   constant BYTES0              : integer := FUNC0_ADER_3 -10;
   constant BYTES1              : integer := FUNC0_ADER_3 -11;
	constant WB32bits            : integer := FUNC0_ADER_3 -12;
   constant Endian              : integer := FUNC0_ADER_3 -4;
   constant SDB_ADDR_0           : integer := 200;
   constant SDB_ADDR_1           : integer := SDB_ADDR_1 -1;
   constant SDB_ADDR_2           : integer := SDB_ADDR_1 -2;
   constant SDB_ADDR_3           : integer := SDB_ADDR_1 -3;

   -- Initialization CR:
   constant BEG_USER_CR  : integer := 1;  
   constant END_USER_CR  : integer := 2;
   constant BEG_CRAM     : integer := 3;
   constant END_CRAM     : integer := 4;
   constant BEG_USER_CSR : integer := 5;
   constant END_USER_CSR : integer := 6;
   constant FUNC_AMCAP   : integer := 7;
   constant FUNC_XAMCAP  : integer := 8;
   constant FUNC_ADEM    : integer := 9;

   constant c_CRinitAddr : t_cr_add_table(BEG_USER_CR to FUNC_ADEM) := (
   BEG_USER_CR => (add => 16#020#, len => 3),
   END_USER_CR => (add => 16#023#, len => 3),

   BEG_CRAM => (add => 16#26#, len => 3),
   END_CRAM => (add => 16#29#, len => 3),

   BEG_USER_CSR => (add => 16#02C#, len => 3),
   END_USER_CSR => (add => 16#02F#, len => 3),

   FUNC_AMCAP  => (add => 16#048#, len => 64),
   FUNC_XAMCAP => (add => 16#088#, len => 256),
   FUNC_ADEM   => (add => 16#188#, len => 32));
  
  constant c_buffer_default : t_VME_BUFFER :=(
			s_addrDir      =>  '0',
			s_dataDir      =>  '0',
			s_clk          =>  '0',
         s_buffer_eo    => ADDR_BUFF,
         s_latch_oe     =>  '0'
);
   -- Main Finite State machine signals default:
	-- When the S_FPGA detects the magic sequency, it erases the A_FPGA so 
	-- I don't need to drive the s_dtackOE, s_dataOE, s_addrOE, s_addrDir, s_dataDir 
	-- to 'Z' in the default configuration.
	-- If the S_FPGA will be provided to a core who drive these lines without erase the
	-- A_FPGA the above mentioned lines should be changed to 'Z' !!!
   constant c_FSM_default : t_FSM :=(
   s_memReq         => '0',
   s_decode         => '0',
   s_dtackOE        => '0',  
   s_mainDTACK      => '1',
   s_buffer         => c_buffer_default,
   --s_dataDir        => '0',
   --s_dataOE         => '0',
   --s_addrDir        => '0',  -- during IACK cycle the ADDR lines are input
   --s_addrOE         => '0',
   s_DSlatch        => '0',
   s_incrementAddr  => '0',
   s_dataPhase      => '0',
   s_dataToOutput   => '0',
   s_dataToAddrBus  => '0',
   s_transferActive => '0',
   s_2eLatchAddr    => "00",
   s_retry          => '0',
   s_berr           => '0',
   s_BERR_out       => '0'
);

   constant c_FSM_IRQ : t_FSM_IRQ :=(
		s_IACKOUT   => '1',
		--s_DataDir   => '0', 
      s_buffer    => c_buffer_default,
		s_DTACK     => '1',
		s_enableIRQ => '0',
		s_resetIRQ  => '1',
		s_DSlatch   => '0',
		s_DTACK_OE  => '0'
);

 -- CSR address:
  constant c_BAR_addr             : unsigned(19 downto 0) := x"7FFFF";   -- VME64x defined CSR
  constant c_BIT_SET_REG_addr     : unsigned(19 downto 0) := x"7FFFB";
  constant c_BIT_CLR_REG_addr     : unsigned(19 downto 0) := x"7FFF7";
  constant c_CRAM_OWNER_addr      : unsigned(19 downto 0) := x"7FFF3";
  constant c_USR_BIT_SET_REG_addr : unsigned(19 downto 0) := x"7FFEF";
  constant c_USR_BIT_CLR_REG_addr : unsigned(19 downto 0) := x"7FFEB";
  constant c_FUNC7_ADER_0_addr    : unsigned(19 downto 0) := x"7FFDF";
  constant c_FUNC7_ADER_1_addr    : unsigned(19 downto 0) := x"7FFDB";
  constant c_FUNC7_ADER_2_addr    : unsigned(19 downto 0) := x"7FFD7";
  constant c_FUNC7_ADER_3_addr    : unsigned(19 downto 0) := x"7FFD3";
  constant c_FUNC6_ADER_0_addr    : unsigned(19 downto 0) := x"7FFCF";
  constant c_FUNC6_ADER_1_addr    : unsigned(19 downto 0) := x"7FFCB";
  constant c_FUNC6_ADER_2_addr    : unsigned(19 downto 0) := x"7FFC7";
  constant c_FUNC6_ADER_3_addr    : unsigned(19 downto 0) := x"7FFC3";
  constant c_FUNC5_ADER_0_addr    : unsigned(19 downto 0) := x"7FFBF";
  constant c_FUNC5_ADER_1_addr    : unsigned(19 downto 0) := x"7FFBB";
  constant c_FUNC5_ADER_2_addr    : unsigned(19 downto 0) := x"7FFB7";
  constant c_FUNC5_ADER_3_addr    : unsigned(19 downto 0) := x"7FFB3";
  constant c_FUNC4_ADER_0_addr    : unsigned(19 downto 0) := x"7FFAF";
  constant c_FUNC4_ADER_1_addr    : unsigned(19 downto 0) := x"7FFAB";
  constant c_FUNC4_ADER_2_addr    : unsigned(19 downto 0) := x"7FFA7";
  constant c_FUNC4_ADER_3_addr    : unsigned(19 downto 0) := x"7FFA3";
  constant c_FUNC3_ADER_0_addr    : unsigned(19 downto 0) := x"7FF9F";
  constant c_FUNC3_ADER_1_addr    : unsigned(19 downto 0) := x"7FF9B";
  constant c_FUNC3_ADER_2_addr    : unsigned(19 downto 0) := x"7FF97";
  constant c_FUNC3_ADER_3_addr    : unsigned(19 downto 0) := x"7FF93";
  constant c_FUNC2_ADER_0_addr    : unsigned(19 downto 0) := x"7FF8F";
  constant c_FUNC2_ADER_1_addr    : unsigned(19 downto 0) := x"7FF8B";
  constant c_FUNC2_ADER_2_addr    : unsigned(19 downto 0) := x"7FF87";
  constant c_FUNC2_ADER_3_addr    : unsigned(19 downto 0) := x"7FF83";
  constant c_FUNC1_ADER_0_addr    : unsigned(19 downto 0) := x"7FF7F";
  constant c_FUNC1_ADER_1_addr    : unsigned(19 downto 0) := x"7FF7B";
  constant c_FUNC1_ADER_2_addr    : unsigned(19 downto 0) := x"7FF77";
  constant c_FUNC1_ADER_3_addr    : unsigned(19 downto 0) := x"7FF73";
  constant c_FUNC0_ADER_0_addr    : unsigned(19 downto 0) := x"7FF6F";
  constant c_FUNC0_ADER_1_addr    : unsigned(19 downto 0) := x"7FF6B";
  constant c_FUNC0_ADER_2_addr    : unsigned(19 downto 0) := x"7FF67";
  constant c_FUNC0_ADER_3_addr    : unsigned(19 downto 0) := x"7FF63";  -- VME64x defined CSR
  constant c_IRQ_Vector_addr      : unsigned(19 downto 0) := x"7FF5F";  -- VME64x reserved CSR
  constant c_IRQ_level_addr       : unsigned(19 downto 0) := x"7FF5B";  -- VME64x reserved CSR
  constant c_TIME0_ns_addr        : unsigned(19 downto 0) := x"7FF4f";  -- VME64x reserved CSR
  constant c_TIME1_ns_addr        : unsigned(19 downto 0) := x"7FF4b";
  constant c_TIME2_ns_addr        : unsigned(19 downto 0) := x"7FF47";
  constant c_TIME3_ns_addr        : unsigned(19 downto 0) := x"7FF43";
  constant c_TIME4_ns_addr        : unsigned(19 downto 0) := x"7FF3f";
  constant c_BYTES0_addr          : unsigned(19 downto 0) := x"7FF3b";
  constant c_BYTES1_addr          : unsigned(19 downto 0) := x"7FF37";
  constant c_WB32bits_addr        : unsigned(19 downto 0) := x"7FF33";
  constant c_Endian_addr          : unsigned(19 downto 0) := x"7FF53";  -- VME64x reserved CSR  
  -- USER CSR address:
  constant c_SDB_ADDR             : unsigned(19 downto 0) := x"7A00B";
--___________________________________________________________________________________________
-- TYPE:
  type t_typeOfDataTransfer is (  D08_0,   
                                  D08_1,   
                                  D08_2,   
                                  D08_3,   
                                  D16_01,  
                                  D16_23,  
                                  D32,
                                  D64,
                                  TypeError
                                );

   type t_addressingType is (     A24,
                                  A24_BLT,
                                  A24_MBLT,
                                  CR_CSR,
                                  A16,
                                  A32,
                                  A32_BLT,
                                  A32_MBLT,
                                  A64,
                                  A64_BLT,
                                  A64_MBLT,
                                  TWOedge,
                                  AM_Error
                             );

   type t_transferType is (       SINGLE,
                                  BLT,
                                  MBLT,
                                  TWOe,
                                  error
                             );

   type t_XAMtype is (            A32_2eVME,
                                  A64_2eVME,
                                  A32_2eSST,
                                  A64_2eSST,
                                  A32_2eSSTb,
                                  A64_2eSSTb,
                                  XAM_error
                             );	

   type t_2eType is (             TWOe_VME,
                                  TWOe_SST
                    );							

   type t_mainFSMstates is (      IDLE,
                                  DECODE_ACCESS,
                                  WAIT_FOR_DS,
                                  LATCH_DS1,
											 LATCH_DS2,
											 LATCH_DS3,
											 LATCH_DS4,
                                  CHECK_TRANSFER_TYPE,
                                  MEMORY_REQ,
                                  DATA_TO_BUS,
                                  DTACK_LOW,
                                  DECIDE_NEXT_CYCLE,
                                  INCREMENT_ADDR,
                                  SET_DATA_PHASE
											 -- uncomment for using 2e modes:
--                                  WAIT_FOR_DS_2e,
--                                  ADDR_PHASE_1,
--                                  ADDR_PHASE_2,
--                                  ADDR_PHASE_3,
--                                  DECODE_ACCESS_2e,
--                                  DTACK_PHASE_1,
--                                  DTACK_PHASE_2,
--                                  DTACK_PHASE_3,
--                                  TWOeVME_WRITE,
--                                  TWOeVME_READ,
--                                  TWOeVME_MREQ_RD,
--                                  WAIT_WR_1,
--                                  WAIT_WR_2,
--                                  WAIT_WB_ACK_WR,
--                                  WAIT_WB_ACK_RD,
--                                  TWOeVME_TOGGLE_WR,
--                                  TWOeVME_TOGGLE_RD,
--                                  TWOe_FIFO_WRITE,
--                                  TWOe_TOGGLE_DTACK,
--                                  TWOeVME_INCR_ADDR,
--                                  TWOe_WAIT_FOR_DS1,
--                                  TWOe_FIFO_WAIT_READ,
--                                  TWOe_FIFO_READ,
--                                  TWOe_CHECK_BEAT,
--                                  TWOe_RELEASE_DTACK,
--                                  TWOe_END_1,
--                                  TWOe_END_2
                             );

   type t_initState is (          IDLE,            
                                  SET_ADDR,
                                  GET_DATA,
                                  END_INIT
                             );
   type base_addr is   (         GEOGRAPHICAL_ADDR,
                                 MECHANICALLY
                             );

   type t_FUNC_32b_array is array (0 to 7) of unsigned(31 downto 0);  -- ADER register array
   type t_FUNC_64b_array is array (0 to 7) of unsigned(63 downto 0);  -- AMCAP register array
   type t_FUNC_256b_array is array (0 to 7) of unsigned(255 downto 0);  -- XAMCAP register array
   type t_FUNC_32b_array_std is array (0 to 7) of std_logic_vector(31 downto 0);  -- ADER register array
   type t_FUNC_64b_array_std is array (0 to 7) of std_logic_vector(63 downto 0);  -- AMCAP register array
   type t_FUNC_256b_array_std is array (0 to 7) of std_logic_vector(255 downto 0);  -- XAMCAP register array
   type t_CSRarray is array(BAR downto WB32bits) of unsigned(7 downto 0);
   type t_cr_array is array (natural range <>) of std_logic_vector(7 downto 0);

-- functions
function f_div8 (width : integer) return integer;
function f_log2_size (A : natural) return natural;
function f_set_CR_space (BoardID : integer; cr_default : t_cr_array;
ManufacturerID : integer; RevisionID : integer; ProgramID : integer) return t_cr_array;
function f_latchDS (clk_period : integer) return integer;
--_____________________________________________________________________________________________________
--COMPONENTS:

              component VME64xCore_Top is
              generic(
                        g_clock          : integer := c_clk_period;
                        g_wb_data_width  : integer := c_width;
                        g_wb_addr_width  : integer := c_addr_width;
                        g_cram_size      : integer := c_CRAM_SIZE;
                        g_BoardID        : integer := c_SVEC_ID;
                        g_ManufacturerID : integer := c_CERN_ID;       -- 0x00080030
                        g_RevisionID     : integer := c_RevisionID;    -- 0x00000001
                        g_ProgramID      : integer := 96;              -- 0x00000060 
                        g_base_addr      : base_addr  := MECHANICALLY
                        );
                        port(
                              -- VME signals:
                        clk_i           : in    std_logic;
                        VME_AS_n_i      : in    std_logic;
                        VME_RST_n_i     : in    std_logic;
                        VME_WRITE_n_i   : in    std_logic;
                        VME_AM_i        : in    std_logic_vector(5 downto 0);
                        VME_DS_n_i      : in    std_logic_vector(1 downto 0);
                        VME_GA_i        : in    std_logic_vector(5 downto 0);
                        VME_IACKIN_n_i  : in    std_logic;
                        VME_IACK_n_i    : in    std_logic;	
                        VME_LWORD_n_i   : in    std_logic;
                        VME_LWORD_n_o   : out   std_logic;
                        VME_ADDR_i      : in    std_logic_vector(31 downto 1);
                        VME_ADDR_o      : out   std_logic_vector(31 downto 1);
                        VME_DATA_i      : in    std_logic_vector(31 downto 0); 
                        VME_DATA_o      : out   std_logic_vector(31 downto 0); 
                        VME_BERR_o      : out   std_logic;
                        VME_RETRY_n_o   : out   std_logic;
                        VME_RETRY_OE_o  : out   std_logic;
                        VME_IRQ_o       : out   std_logic_vector(6 downto 0);
                        VME_IACKOUT_n_o : out   std_logic;

                        VME_DTACK_n_o   : out   std_logic;
                        VME_DTACK_OE_o  : out   std_logic;

                        VME_Buffer_o    : out   t_VME_BUFFER;

                        DAT_i           : in    std_logic_vector(g_wb_data_width - 1 downto 0);
                        ERR_i           : in    std_logic;
                        RTY_i           : in    std_logic;
                        ACK_i           : in    std_logic;
                        STALL_i         : in    std_logic;
                        DAT_o           : out   std_logic_vector(g_wb_data_width - 1 downto 0);
                        ADR_o           : out   std_logic_vector(g_wb_addr_width - 1 downto 0);
                        CYC_o           : out   std_logic;
                        SEL_o           : out   std_logic_vector(f_div8(g_wb_data_width) - 1 downto 0);
                        STB_o           : out   std_logic;
                        WE_o            : out   std_logic;
                        -- IRQ Generator
                        IRQ_i           : in    std_logic;
                        INT_ack_o       : out   std_logic;
                        reset_o         : out   std_logic;
                        -- for debug:
                        debug           : out   std_logic_vector(7 downto 0)
                           );
              end component VME64xCore_Top;


              component VME_bus is
				  generic(  g_clock         : integer := c_clk_period;
                        g_wb_data_width : integer := c_width;
	                     g_wb_addr_width : integer := c_addr_width;
								g_cram_size     : integer := c_CRAM_SIZE
                        );
                 port(
                        clk_i                : in std_logic;
                        VME_RST_n_i          : in std_logic;
                        VME_AS_n_i           : in std_logic;
                        VME_LWORD_n_i        : in std_logic;
                        VME_WRITE_n_i        : in std_logic;
                        VME_DS_n_i           : in std_logic_vector(1 downto 0);
								VME_DS_ant_n_i       : in std_logic_vector(1 downto 0);
                        VME_ADDR_i           : in std_logic_vector(31 downto 1);
                        VME_DATA_i           : in std_logic_vector(31 downto 0);
                        VME_AM_i             : in std_logic_vector(5 downto 0);
                        VME_IACK_n_i         : in std_logic;
                        memAckWB_i           : in std_logic;
                        wbData_i             : in std_logic_vector(g_wb_data_width - 1 downto 0);
                        err_i                : in std_logic;
                        rty_i                : in std_logic;
                        stall_i              : in std_logic;
                        CRAMdata_i           : in std_logic_vector(7 downto 0);
                        CRdata_i             : in std_logic_vector(7 downto 0);
                        CSRData_i            : in std_logic_vector(7 downto 0);
                        reset_flag_i         : in std_logic;
                        Ader0                : in std_logic_vector(31 downto 0);
                        Ader1                : in std_logic_vector(31 downto 0);
                        Ader2                : in std_logic_vector(31 downto 0);
                        Ader3                : in std_logic_vector(31 downto 0);
                        Ader4                : in std_logic_vector(31 downto 0);
                        Ader5                : in std_logic_vector(31 downto 0);
                        Ader6                : in std_logic_vector(31 downto 0);
                        Ader7                : in std_logic_vector(31 downto 0);
                        ModuleEnable         : in std_logic;
                        Endian_i             : in std_logic_vector(2 downto 0);
                        Sw_Reset             : in std_logic;
                        BAR_i                : in std_logic_vector(4 downto 0);         
                        reset_o              : out std_logic;
                        VME_LWORD_n_o        : out std_logic;
                        VME_RETRY_n_o        : out std_logic;
                        VME_RETRY_OE_o       : out std_logic;
                        VME_DTACK_n_o        : out std_logic;
                        VME_DTACK_OE_o       : out std_logic;
                        VME_BERR_o           : out std_logic;
                        VME_ADDR_o           : out std_logic_vector(31 downto 1);
								VME_BUFFER_o         : out t_VME_BUFFER;
                        VME_DATA_o           : out std_logic_vector(31 downto 0);
                        memReq_o             : out std_logic;
                        wbData_o             : out std_logic_vector(g_wb_data_width - 1 downto 0);
                        locAddr_o            : out std_logic_vector(g_wb_addr_width - 1 downto 0);
                        wbSel_o              : out std_logic_vector(f_div8(g_wb_data_width) - 1 downto 0);
                        RW_o                 : out std_logic;
                        cyc_o                : out std_logic;         
                        CRAMaddr_o           : out std_logic_vector(f_log2_size(g_cram_size)-1 downto 0);
                        CRAMdata_o           : out std_logic_vector(7 downto 0);
                        CRAMwea_o            : out std_logic;
                        CRaddr_o             : out std_logic_vector(11 downto 0);
                        en_wr_CSR            : out std_logic;
                        CrCsrOffsetAddr      : out std_logic_vector(18 downto 0);
                        CSRData_o            : out std_logic_vector(7 downto 0);
                        err_flag_o           : out std_logic;
                        numBytes             : out std_logic_vector(12 downto 0);
                        transfTime           : out std_logic_vector(39 downto 0);
                        leds                 : out std_logic_vector(7 downto 0)                 
                     );
              end component VME_bus;


              component VME_Access_Decode is
                 port (
                        clk_i          : in  std_logic;
                        reset          : in  std_logic;
                        mainFSMreset   : in  std_logic;
                        decode         : in  std_logic;
                        ModuleEnable   : in  std_logic;
                        InitInProgress : in  std_logic;
                        Addr           : in  std_logic_vector(63 downto 0);
                        Ader0          : in  std_logic_vector(31 downto 0);
                        Ader1          : in  std_logic_vector(31 downto 0);
                        Ader2          : in  std_logic_vector(31 downto 0);
                        Ader3          : in  std_logic_vector(31 downto 0);
                        Ader4          : in  std_logic_vector(31 downto 0);
                        Ader5          : in  std_logic_vector(31 downto 0);
                        Ader6          : in  std_logic_vector(31 downto 0);
                        Ader7          : in  std_logic_vector(31 downto 0);
                        Adem0          : in  std_logic_vector(31 downto 0);
                        Adem1          : in  std_logic_vector(31 downto 0);
                        Adem2          : in  std_logic_vector(31 downto 0);
                        Adem3          : in  std_logic_vector(31 downto 0);
                        Adem4          : in  std_logic_vector(31 downto 0);
                        Adem5          : in  std_logic_vector(31 downto 0);
                        Adem6          : in  std_logic_vector(31 downto 0);
                        Adem7          : in  std_logic_vector(31 downto 0);
                        AmCap0         : in  std_logic_vector(63 downto 0);
                        AmCap1         : in  std_logic_vector(63 downto 0);
                        AmCap2         : in  std_logic_vector(63 downto 0);
                        AmCap3         : in  std_logic_vector(63 downto 0);
                        AmCap4         : in  std_logic_vector(63 downto 0);
                        AmCap5         : in  std_logic_vector(63 downto 0);
                        AmCap6         : in  std_logic_vector(63 downto 0);
                        AmCap7         : in  std_logic_vector(63 downto 0);
                        XAmCap0        : in  std_logic_vector(255 downto 0);
                        XAmCap1        : in  std_logic_vector(255 downto 0);
                        XAmCap2        : in  std_logic_vector(255 downto 0);
                        XAmCap3        : in  std_logic_vector(255 downto 0);
                        XAmCap4        : in  std_logic_vector(255 downto 0);
                        XAmCap5        : in  std_logic_vector(255 downto 0);
                        XAmCap6        : in  std_logic_vector(255 downto 0);
                        XAmCap7        : in  std_logic_vector(255 downto 0);
                        Am             : in  std_logic_vector(5 downto 0);
                        XAm            : in  std_logic_vector(7 downto 0);
                        BAR_i          : in  std_logic_vector(4 downto 0);
                        AddrWidth      : in  std_logic_vector(1 downto 0);          
                        Funct_Sel      : out std_logic_vector(7 downto 0);
                        Base_Addr      : out std_logic_vector(63 downto 0);
                        Confaccess     : out std_logic;
                        CardSel        : out std_logic
                     );
              end component VME_Access_Decode;

              component VME_Funct_Match is
                 port(
                        clk_i          : in  std_logic;
                        reset          : in  std_logic;
                        decode         : in  std_logic;
                        mainFSMreset   : in  std_logic;
                        Addr           : in  std_logic_vector(63 downto 0);
                        AddrWidth      : in  std_logic_vector(1 downto 0);
                        Ader0          : in  std_logic_vector(31 downto 0);
                        Ader1          : in  std_logic_vector(31 downto 0);
                        Ader2          : in  std_logic_vector(31 downto 0);
                        Ader3          : in  std_logic_vector(31 downto 0);
                        Ader4          : in  std_logic_vector(31 downto 0);
                        Ader5          : in  std_logic_vector(31 downto 0);
                        Ader6          : in  std_logic_vector(31 downto 0);
                        Ader7          : in  std_logic_vector(31 downto 0);
                        Adem0          : in  std_logic_vector(31 downto 0);
                        Adem1          : in  std_logic_vector(31 downto 0);
                        Adem2          : in  std_logic_vector(31 downto 0);
                        Adem3          : in  std_logic_vector(31 downto 0);
                        Adem4          : in  std_logic_vector(31 downto 0);
                        Adem5          : in  std_logic_vector(31 downto 0);
                        Adem6          : in  std_logic_vector(31 downto 0);
                        Adem7          : in  std_logic_vector(31 downto 0);          
                        FunctMatch     : out std_logic_vector(7 downto 0);
                        DFS_o          : out std_logic_vector(7 downto 0);
                        Nx_Base_Addr   : out std_logic_vector(63 downto 0)
                     );
              end component VME_Funct_Match;

              component VME_CR_CSR_Space is
				  generic(
								g_cram_size      : integer := c_CRAM_SIZE;
								g_wb_data_width  : integer := c_width;
								g_CRspace        : t_cr_array; -- := c_cr_array;
					         g_BoardID        : integer := c_SVEC_ID;
								g_ManufacturerID : integer := c_CERN_ID;
				            g_RevisionID     : integer := c_RevisionID;
				            g_ProgramID      : integer := 96;
								g_base_addr      : base_addr:= GEOGRAPHICAL_ADDR
                        );
								
                 port(
                        clk_i              : in  std_logic;
                        reset              : in  std_logic;
                        CR_addr            : in  std_logic_vector(11 downto 0);
                        CRAM_addr          : in  std_logic_vector(f_log2_size(g_cram_size)-1 downto 0);
                        CRAM_data_i        : in  std_logic_vector(7 downto 0);
                        CRAM_Wen           : in  std_logic;
                        en_wr_CSR          : in  std_logic;
                        CrCsrOffsetAddr    : in  std_logic_vector(18 downto 0);
                        VME_GA_oversampled : in  std_logic_vector(5 downto 0);
                        locDataIn          : in  std_logic_vector(7 downto 0);
                        err_flag           : in  std_logic;          
                        CR_data            : out std_logic_vector(7 downto 0);
                        CRAM_data_o        : out std_logic_vector(7 downto 0);
                        reset_flag         : out std_logic;
                        CSRdata            : out std_logic_vector(7 downto 0);
                        Ader0              : out std_logic_vector(31 downto 0);
                        Ader1              : out std_logic_vector(31 downto 0);
                        Ader2              : out std_logic_vector(31 downto 0);
                        Ader3              : out std_logic_vector(31 downto 0);
                        Ader4              : out std_logic_vector(31 downto 0);
                        Ader5              : out std_logic_vector(31 downto 0);
                        Ader6              : out std_logic_vector(31 downto 0);
                        Ader7              : out std_logic_vector(31 downto 0);
                        ModuleEnable       : out std_logic;
                        Sw_Reset           : out std_logic;
                        numBytes           : in  std_logic_vector(12 downto 0);
                        transfTime         : in  std_logic_vector(39 downto 0);
                        Endian_o           : out std_logic_vector(2 downto 0);
                        BAR_o              : out std_logic_vector(4 downto 0);
                        INT_Level          : out std_logic_vector(7 downto 0);
                        INT_Vector         : out std_logic_vector(7 downto 0)
                     );
              end component VME_CR_CSR_Space;

              component VME_Am_Match is
                 port(
                        clk_i          : in  std_logic;
                        reset          : in  std_logic;
                        mainFSMreset   : in  std_logic;
                        Ader0          : in  std_logic_vector(31 downto 0);
                        Ader1          : in  std_logic_vector(31 downto 0);
                        Ader2          : in  std_logic_vector(31 downto 0);
                        Ader3          : in  std_logic_vector(31 downto 0);
                        Ader4          : in  std_logic_vector(31 downto 0);
                        Ader5          : in  std_logic_vector(31 downto 0);
                        Ader6          : in  std_logic_vector(31 downto 0);
                        Ader7          : in  std_logic_vector(31 downto 0);
                        AmCap0         : in  std_logic_vector(63 downto 0);
                        AmCap1         : in  std_logic_vector(63 downto 0);
                        AmCap2         : in  std_logic_vector(63 downto 0);
                        AmCap3         : in  std_logic_vector(63 downto 0);
                        AmCap4         : in  std_logic_vector(63 downto 0);
                        AmCap5         : in  std_logic_vector(63 downto 0);
                        AmCap6         : in  std_logic_vector(63 downto 0);
                        AmCap7         : in  std_logic_vector(63 downto 0);
                        XAmCap0        : in  std_logic_vector(255 downto 0);
                        XAmCap1        : in  std_logic_vector(255 downto 0);
                        XAmCap2        : in  std_logic_vector(255 downto 0);
                        XAmCap3        : in  std_logic_vector(255 downto 0);
                        XAmCap4        : in  std_logic_vector(255 downto 0);
                        XAmCap5        : in  std_logic_vector(255 downto 0);
                        XAmCap6        : in  std_logic_vector(255 downto 0);
                        XAmCap7        : in  std_logic_vector(255 downto 0);
                        Am             : in  std_logic_vector(5 downto 0);
                        XAm            : in  std_logic_vector(7 downto 0);
                        DFS_i          : in  std_logic_vector(7 downto 0);
                        decode         : in  std_logic;          
                        AmMatch        : out std_logic_vector(7 downto 0)
                     );
              end component VME_Am_Match;

              component VME_Wb_master is
				  generic(
                        g_wb_data_width : integer := c_width;
	                     g_wb_addr_width : integer := c_addr_width
                        );
                 port(
                        memReq_i        : in  std_logic;
                        clk_i           : in  std_logic;
                        cardSel_i       : in  std_logic;
                        reset_i         : in  std_logic;
                        BERRcondition_i : in  std_logic;
                        sel_i           : in  std_logic_vector(7 downto 0);                   
                        locDataInSwap_i : in  std_logic_vector(63 downto 0);
                        rel_locAddr_i   : in  std_logic_vector(63 downto 0);
                        RW_i            : in  std_logic;
                        stall_i         : in  std_logic;
                        rty_i           : in  std_logic;
                        err_i           : in  std_logic;
                        wbData_i        : in  std_logic_vector(g_wb_data_width - 1 downto 0);
                        memAckWB_i      : in  std_logic;          
                        locDataOut_o    : out std_logic_vector(63 downto 0);
                        memAckWb_o      : out std_logic;
                        err_o           : out std_logic;
                        rty_o           : out std_logic;                 
                        cyc_o           : out std_logic;
                        memReq_o        : out std_logic;
                        WBdata_o        : out std_logic_vector(g_wb_data_width - 1 downto 0);
                        locAddr_o       : out std_logic_vector(g_wb_addr_width - 1 downto 0);
                        WbSel_o         : out std_logic_vector(f_div8(g_wb_data_width) - 1 downto 0);
                        RW_o            : out std_logic
                     );
              end component VME_Wb_master;

              component VME_Init is
                 port(
                        clk_i          : in    std_logic;
                        CRAddr_i       : in    std_logic_vector(18 downto 0);
                        CRdata_i       : in    std_logic_vector(7 downto 0);    
                        RSTedge_i      : in    std_logic;      
                        InitReadCount_o  : out   std_logic_vector(8 downto 0);
                        InitInProgress_o : out   std_logic;
                        BEG_USR_CR_o   : out   std_logic_vector(23 downto 0);
                        END_USR_CR_o   : out   std_logic_vector(23 downto 0);
                        BEG_USR_CSR_o  : out   std_logic_vector(23 downto 0);
                        END_USR_CSR_o  : out   std_logic_vector(23 downto 0);
                        BEG_CRAM_o     : out   std_logic_vector(23 downto 0);
                        END_CRAM_o     : out   std_logic_vector(23 downto 0);
                        FUNC0_ADEM_o   : out   std_logic_vector(31 downto 0);
                        FUNC1_ADEM_o   : out   std_logic_vector(31 downto 0);
                        FUNC2_ADEM_o   : out   std_logic_vector(31 downto 0);
                        FUNC3_ADEM_o   : out   std_logic_vector(31 downto 0);
                        FUNC4_ADEM_o   : out   std_logic_vector(31 downto 0);
                        FUNC5_ADEM_o   : out   std_logic_vector(31 downto 0);
                        FUNC6_ADEM_o   : out   std_logic_vector(31 downto 0);
                        FUNC7_ADEM_o   : out   std_logic_vector(31 downto 0);
                        FUNC0_AMCAP_o  : out   std_logic_vector(63 downto 0);
                        FUNC1_AMCAP_o  : out   std_logic_vector(63 downto 0);
                        FUNC2_AMCAP_o  : out   std_logic_vector(63 downto 0);
                        FUNC3_AMCAP_o  : out   std_logic_vector(63 downto 0);
                        FUNC4_AMCAP_o  : out   std_logic_vector(63 downto 0);
                        FUNC5_AMCAP_o  : out   std_logic_vector(63 downto 0);
                        FUNC6_AMCAP_o  : out   std_logic_vector(63 downto 0);
                        FUNC7_AMCAP_o  : out   std_logic_vector(63 downto 0);
                        FUNC0_XAMCAP_o : out   std_logic_vector(255 downto 0);
                        FUNC1_XAMCAP_o : out   std_logic_vector(255 downto 0);
                        FUNC2_XAMCAP_o : out   std_logic_vector(255 downto 0);
                        FUNC3_XAMCAP_o : out   std_logic_vector(255 downto 0);
                        FUNC4_XAMCAP_o : out   std_logic_vector(255 downto 0);
                        FUNC5_XAMCAP_o : out   std_logic_vector(255 downto 0);
                        FUNC6_XAMCAP_o : out   std_logic_vector(255 downto 0);
                        FUNC7_XAMCAP_o : out   std_logic_vector(255 downto 0)
                     );
              end component VME_Init;	

              component VME_swapper is
                 port(
                        d_i : in  std_logic_vector(63 downto 0);
                        sel : in  std_logic_vector(2 downto 0);          
                        d_o : out std_logic_vector(63 downto 0)
                     );
              end component VME_swapper;
				  component Reg32bit is
	             port (
	                   	 reset, clk_i, enable: in std_logic;
	         	          di : in std_logic_vector(31 downto 0);
		                   do: out std_logic_vector(31 downto 0)
		              );
              end component Reg32bit;
              component FlipFlopD is
                 port (
                         reset,enable,sig_i,clk_i     : in  std_logic;
                         sig_o                        : out std_logic := '0'
                      );
              end component FlipFlopD;
              component EdgeDetection is
                 port (
                         sig_i,clk_i : in  std_logic;
                         sigEdge_o   : out std_logic := '0'
                      );
              end component EdgeDetection;

              component FallingEdgeDetection is
                 port (
                 sig_i, clk_i : in  std_logic;
                 FallEdge_o   : out std_logic);
              end component FallingEdgeDetection;

              component RisEdgeDetection is
                 port (
                 sig_i, clk_i : in  std_logic;
                 RisEdge_o    : out std_logic);
              end component RisEdgeDetection;

              component DoubleSigInputSample is
                 port (
                 sig_i, clk_i : in  std_logic;
                 sig_o        : out std_logic);
              end component DoubleSigInputSample;

              component SigInputSample is
                 port (
                 sig_i, clk_i : in  std_logic;
                 sig_o        : out std_logic);
              end component SigInputSample;  

              component DoubleRegInputSample is
                 generic(
                           width : natural := 8
                        );
                 port (
                         reg_i : in  std_logic_vector(width-1 downto 0);
                         reg_o : out std_logic_vector(width-1 downto 0) := (others => '0');
                         clk_i : in  std_logic
                      );
              end component DoubleRegInputSample;

              component RegInputSample is
                 generic(
                           width : natural := 8
                        );
                 port (
                         reg_i : in  std_logic_vector(width-1 downto 0);
                         reg_o : out std_logic_vector(width-1 downto 0) := (others => '0');
                         clk_i : in  std_logic
                      );
              end component RegInputSample;
				  
				  component SingleRegInputSample is
                 generic(
                           width : natural := 8
                        );
                 port (
                         reg_i : in  std_logic_vector(width-1 downto 0);
                         reg_o : out std_logic_vector(width-1 downto 0) := (others => '0');
                         clk_i : in  std_logic
                      );
              end component SingleRegInputSample;
				  

              component VME_IRQ_Controller is
                 port(
                        clk_i           : in  std_logic;
                        reset_n_i       : in  std_logic;
                        VME_IACKIN_n_i  : in  std_logic;
                        VME_AS_n_i      : in  std_logic;
								VME_AS1_n_i     : in  std_logic;
                        VME_DS_n_i      : in  std_logic_vector(1 downto 0);
                        VME_LWORD_n_i   : in  std_logic;
                        VME_ADDR_123_i  : in  std_logic_vector(2 downto 0);
                        INT_Level_i     : in  std_logic_vector(7 downto 0);
                        INT_Vector_i    : in  std_logic_vector(7 downto 0);
                        INT_Req_i       : in  std_logic;          
                        VME_IRQ_n_o     : out std_logic_vector(6 downto 0);
                        VME_IACKOUT_n_o : out std_logic;
                        VME_DTACK_n_o   : out std_logic;
                        VME_DTACK_OE_o  : out std_logic;
                        VME_DATA_o      : out std_logic_vector(31 downto 0);
                        --VME_DATA_DIR_o  : out std_logic
								VME_BUFFER_o    : out t_VME_BUFFER
                     );
              end component VME_IRQ_Controller;

              component VME_CRAM is
                 generic (dl : integer := 8; 		
                          al : integer := f_log2_size(c_CRAM_SIZE)	
                          );    
                 port(
                        clk : in  std_logic;
                        we  : in  std_logic;
                        aw  : in  std_logic_vector(al - 1 downto 0);
                        di  : in  std_logic_vector(dl - 1 downto 0);          
                        dw  : out std_logic_vector(dl - 1 downto 0)
                     );
              end component VME_CRAM;

end vme64x_pack;
package body vme64x_pack is

function f_div8 (width : integer) return integer is
  begin
    for I in 1 to 8 loop      
      if (8*I >= width) then
        return(I);
      end if;
    end loop;
  end function f_div8;

function f_log2_size (A : natural) return natural is
  begin
    for I in 1 to 64 loop               -- Works for up to 64 bits
      if (2**I >= A) then
        return(I);
      end if;
    end loop;
    return(63);
  end function f_log2_size;
  
 function f_set_CR_space(BoardID : integer; cr_default : t_cr_array;
                        ManufacturerID : integer; RevisionID : integer;
								ProgramID : integer) return t_cr_array is
   variable v_CR_space       : t_cr_array(2**12 downto 0);
	variable v_BoardID        : std_logic_vector(31 downto 0); 
	variable v_ManufacturerID : std_logic_vector(23 downto 0);
	variable v_RevisionID     : std_logic_vector(31 downto 0);
	variable v_ProgramID      : std_logic_vector(7 downto 0);
	
	begin
	 v_BoardID        := std_logic_vector(to_unsigned(BoardID,32));
	 v_ManufacturerID := std_logic_vector(to_unsigned(ManufacturerID,24));
	 v_RevisionID     := std_logic_vector(to_unsigned(RevisionID,32));
	 v_ProgramID      := std_logic_vector(to_unsigned(ProgramID,8));
	 for i in cr_default'range loop
	     case i is
				when c_BOARD_ID_p1 => v_CR_space(i) := v_BoardID(31 downto 24);
				when c_BOARD_ID_p2 => v_CR_space(i) := v_BoardID(23 downto 16);
				when c_BOARD_ID_p3 => v_CR_space(i) := v_BoardID(15 downto 8);
				when c_BOARD_ID_p4 => v_CR_space(i) := v_BoardID(7 downto 0);
				when c_Manuf_ID_p1 => v_CR_space(i) := v_ManufacturerID(23 downto 16);
				when c_Manuf_ID_p2 => v_CR_space(i) := v_ManufacturerID(15 downto 8);
				when c_Manuf_ID_p3 => v_CR_space(i) := v_ManufacturerID(7 downto 0);
				when c_Rev_ID_p1   => v_CR_space(i) := v_RevisionID(31 downto 24);
				when c_Rev_ID_p2   => v_CR_space(i) := v_RevisionID(23 downto 16);
				when c_Rev_ID_p3   => v_CR_space(i) := v_RevisionID(15 downto 8);
				when c_Rev_ID_p4   => v_CR_space(i) := v_RevisionID(7 downto 0);
				when c_Prog_ID_p   => v_CR_space(i) := v_ProgramID(7 downto 0);
				when others => v_CR_space(i) := cr_default(i);
			end case;
--	     if    i = c_BOARD_ID_p1 then
--		     v_CR_space(i) := v_BoardID(31 downto 24);
--		  elsif i = c_BOARD_ID_p2 then
--		     v_CR_space(i) := v_BoardID(23 downto 16);
--		  elsif i = c_BOARD_ID_p3 then
--		     v_CR_space(i) := v_BoardID(15 downto 8);
--		  elsif i = c_BOARD_ID_p4 then
--		     v_CR_space(i) := v_BoardID(7 downto 0);	  
--		  elsif i = c_Manuf_ID_p1 then
--		     v_CR_space(i) := v_ManufacturerID(23 downto 16);	  
--		  elsif i = c_Manuf_ID_p2 then
--		     v_CR_space(i) := v_ManufacturerID(15 downto 8);
--		  elsif i = c_Manuf_ID_p3 then
--		     v_CR_space(i) := v_ManufacturerID(7 downto 0);	  
--		  elsif i = c_Rev_ID_p1 then
--		     v_CR_space(i) := v_RevisionID(31 downto 24);
--		  elsif i = c_Rev_ID_p2 then
--		     v_CR_space(i) := v_RevisionID(23 downto 16);
--		  elsif i = c_Rev_ID_p3 then
--		     v_CR_space(i) := v_RevisionID(15 downto 8);	  
--		  elsif i = c_Rev_ID_p4 then
--		     v_CR_space(i) := v_RevisionID(7 downto 0);	 
--		  elsif i = c_Prog_ID_p then
--		     v_CR_space(i) := v_ProgramID(7 downto 0);	  
--		  else 
--	        v_CR_space(i) := cr_default(i);	  
--		  end if;
	  end loop;
	return(v_CR_space);
 end function f_set_CR_space;
  
  function f_latchDS(clk_period : integer) return integer is
  begin
    for I in 1 to 4 loop               
      if (clk_period * I >= 20) then   -- 20 is the max time between the assertion
		                                 -- of the DS lines.
        return(I);
      end if;
    end loop;
    return(4);                         -- works for up to 200 MHz
  end function f_latchDS;
  
end vme64x_pack;
