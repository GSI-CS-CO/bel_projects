--________________________________________________________________________________________________
--                             VME TO WB INTERFACE
--
--                                CERN,BE/CO-HT 
--________________________________________________________________________________________________
-- File:                           VME_CR_CSR_Space.vhd
--________________________________________________________________________________________________
-- Description:
-- Please note that only every fourth location in the CR/CSR space is used so it is possible write 
-- the CSR/CRAM selecting the data transfer mode D08_Byte3, D16_Byte23, D32. If other data transfer 
-- modes are selected the write operation will not be successful.
-- If the Master access the board for a reading operation with data transfer type different than 
-- D08_Byte3, D16_Byte23, D32 the data that will be read is 0.
--                                        width = 1 byte
--                           /---------------------------------/
--                           _________________________________
--                          |                                 |0x7ffff
--                          |                                 |
--                          |     Defined and Reserved CSR    |
--                          |                                 |
--                          |   Table 10-13 "Defined Control  |
--                          |   Status register Assignments"  |
--                          |       ANSI/VITA 1.1-1997        |
--                          |        VME64 Extensions         |
--                          |_________________________________|0x7fc00
--                          |                                 |0x013ff
--                          |                                 |
--                          |                                 |
--                          |              CRAM               |
--                          |                                 |
--                          |                                 |
--                          |                                 |
--                          |                                 |
--                          |_________________________________|0x1000
--                          |                                 |0xfff
--                          |                                 |
--                          |     Defined and reserved CR     |
--                          |                                 |
--                          |     Table 10-12 "Defined        |
--                          |  Configuration ROM Assignments" |
--                          |       ANSI/VITA 1.1-1997        |
--                          |        VME64 Extensions         |
--                          |                                 |
--                          |_________________________________| 0x00
--
-- If the size of the register is bigger than 1 byte, (eg: ADER is 4 bytes) these bytes are 
-- stored in the BIG_ENDIAN ORDER.
-- User CR and User CSR are not implemented.
-- In addition to the registers of the table 10-13 in the CSR space you can find:
--                        _
-- IRQ_Vector --> 0x7FF5F  |--> for the Interrupter 
-- IRQ_level  --> 0x7FF5B _|
--                       
-- Endian --> 0x7FF53  --> for the swapper
--
-- WB32bits  --> 0x7FF33 --> if the bit 0 is '1' it means that the WB data bus is 32 bit
--                       _
-- TIME0_ns  --> 0x7FF4f  |
-- TIME1_ns  --> 0x7FF4b  |
-- TIME2_ns  --> 0x7FF47  |
-- TIME3_ns  --> 0x7FF43  | --> to calculate the transfer rate
-- TIME4_ns  --> 0x7FF3f  |
-- BYTES0    --> 0x7FF3b  |
-- BYTES1    --> 0x7FF37 _|
-- 
-- CRAM memory Added. How to use the CRAM:  (1KB)
--        1) The Master read the CRAM_OWNER Register location 0x7fff3; if 0 the CRAM is free
--        2) The Master write his ID in the CRAM_OWNER Register location 0x7fff3
--        3) If the Master can read his ID in the CRAM_OWNER Register it means that it 
--           is the owner of the CRAM.
--           If other Masters write their ID in the CRAM_OWNER Register when it contains a non-zero 
--           value, the write operation will not be successful --> this allows the first 
--           Master that writes a non-zero value to acquire ownership.
--        4) When a Master has the ownership of the CRAM the Bit Set Register's bit 2, 
--           location 0x7fffb, should be setted.
--        5) The Master can release the ownership by writing '1' in the bit 2 to the Bit Set 
--           Register location 0x7fffb.
-- Other flags:
--   Module Enable --> Bit Set Register's bit 4 location 0x7fffb
--                     If this bit is '0' the slave module's address decoder is not enable and 
--                     the Wb bus can't be accessed.
--   Error flag    --> Bit Set Register's bit 3 location 0x7fffb
--                     When the Slave asserts the BERR* line should asserts also this bit.
--   CRAM_OWNER flag --> Bit Set Register's bit 2  location 0x7fffb               
-- The Master can clear these flags by writing '1' in the corresponding bits to the Bit Clr Register 
-- location 0x7fff7.
--
-- Software reset  --> Bit Set Register's bit 7 location 0x7fffb
--                     This bit acts as software reset, indeed if the Master writes '1' here, 
--                     the module will be resetted and reinitializated. 
--                     The reset condition is temporary because during the initialization the default
--                     configuration is uploaded again, so the Master don't need to remove the 
--                     module from reset mode by writing '1' in the bit 7 to the Bit Clr Register.
--                      
--______________________________________________________________________________
-- Authors:                                       
--               Pablo Alvarez Sanchez (Pablo.Alvarez.Sanchez@cern.ch)                             
--               Davide Pedretti       (Davide.Pedretti@cern.ch)  
-- Date         11/2012                                                                           
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
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.numeric_std.all;

use work.xvme64x_pack.all;
use work.VME_CR_pack.all;
use work.VME_CSR_pack.all;
--===========================================================================
-- Entity declaration
--===========================================================================
entity VME_CR_CSR_Space is
   generic(
				   g_cram_size      : integer    := c_CRAM_SIZE;
					g_wb_data_width  : integer    := c_width;
					g_CRspace        : t_cr_array := c_cr_array;
					g_BoardID        : integer    := c_SVEC_ID;
					g_ManufacturerID : integer    := c_CERN_ID;       -- 0x00080030
				   g_RevisionID     : integer    := c_RevisionID;    -- 0x00000001
				   g_ProgramID      : integer    := 96;              -- 0x00000060
					g_base_addr      : base_addr	:= GEOGRAPHICAL_ADDR	
              );
   Port ( -- VMEbus.vhd signals
           clk_i              : in   std_logic;
           reset              : in   std_logic;
           CR_addr            : in   std_logic_vector (11 downto 0);  
           CR_data            : out  std_logic_vector (7 downto 0);
           CRAM_addr          : in   std_logic_vector (f_log2_size(g_cram_size)-1 downto 0);
           CRAM_data_o        : out  std_logic_vector (7 downto 0);
           CRAM_data_i        : in   std_logic_vector (7 downto 0);
           CRAM_Wen           : in   std_logic;
           en_wr_CSR          : in   std_logic;
           CrCsrOffsetAddr    : in   std_logic_vector (18 downto 0);
           VME_GA_oversampled : in   std_logic_vector (5 downto 0);
           locDataIn          : in   std_logic_vector (7 downto 0);
           err_flag           : in   std_logic;
           reset_flag         : out  std_logic;
           CSRdata            : out  std_logic_vector(7 downto 0);
           numBytes           : in   std_logic_vector(12 downto 0);
           transfTime         : in   std_logic_vector(39 downto 0);
         -- VMEbus.vhd DECODER signals
           Ader0              : out  std_logic_vector(31 downto 0);
           Ader1              : out  std_logic_vector(31 downto 0);
           Ader2              : out  std_logic_vector(31 downto 0);
           Ader3              : out  std_logic_vector(31 downto 0);
           Ader4              : out  std_logic_vector(31 downto 0);
           Ader5              : out  std_logic_vector(31 downto 0);
           Ader6              : out  std_logic_vector(31 downto 0);
           Ader7              : out  std_logic_vector(31 downto 0);
           ModuleEnable       : out  std_logic;
           Sw_Reset           : out  std_logic;
           Endian_o           : out  std_logic_vector(2 downto 0);
           BAR_o              : out  std_logic_vector(4 downto 0);
         -- IRQ_controller signals
           INT_Level          : out  std_logic_vector(7 downto 0);
           INT_Vector         : out  std_logic_vector(7 downto 0)
        );
end VME_CR_CSR_Space;
--===========================================================================
-- Architecture declaration
--===========================================================================
architecture Behavioral of VME_CR_CSR_Space is
   signal s_CSRarray             : t_CSRarray;   -- Array of CSR registers
   signal s_bar_written          : std_logic;
   signal s_CSRdata              : unsigned(7 downto 0);
   signal s_FUNC_ADER            : t_FUNC_32b_array;
	signal s_CR_Space             : t_cr_array(2**12 downto 0);
   signal s_CrCsrOffsetAddr      : unsigned(18 downto 0);
   signal s_locDataIn            : unsigned(7 downto 0);
   signal s_CrCsrOffsetAderIndex : unsigned(18 downto 0);
   signal s_odd_parity           : std_logic;
	signal s_BARerror             : std_logic;
	signal s_BAR_o                : std_logic_vector(4 downto 0);
	signal s_VME_GA_pre				: std_logic_vector(5 downto 0);
--===========================================================================
-- Architecture begin
--===========================================================================
begin

-- check the parity:
GA_Addr: if (g_base_addr = GEOGRAPHICAL_ADDR) generate

s_odd_parity   <=  VME_GA_oversampled(5) xor VME_GA_oversampled(4) xor 
                   VME_GA_oversampled(3) xor VME_GA_oversampled(2) xor 
					    VME_GA_oversampled(1) xor VME_GA_oversampled(0);
end generate GA_Addr;

MECH_Addr: if (g_base_addr = MECHANICALLY) generate
s_odd_parity	<= '1';
  process(clk_i)
   begin
      if rising_edge(clk_i) then
			s_VME_GA_pre	<=	VME_GA_oversampled;
      end if;
   end process;
end generate MECH_Addr;


-- If the crate is not driving the GA lines or the parity is even the BAR register
-- is set to 0x00 and the following flag is asserted; the board will not answer if the 
-- master accesses its  CR/CSR space and we can see a time out error in the VME bus.  
s_BARerror <= not(s_BAR_o(4) or s_BAR_o(3)or s_BAR_o(2) or s_BAR_o(1) or s_BAR_o(0));		
--------------------------------------------------------------------------------
s_CR_Space <= f_set_CR_space(g_BoardID, g_CRspace, g_ManufacturerID, g_RevisionID, g_ProgramID);
-- CR
   process(clk_i)
   begin
      if rising_edge(clk_i) then
         CR_data <= s_CR_Space(to_integer(unsigned(CR_addr)));  -- c_cr_array(to_integer(unsigned(CR_addr)));
      end if;
   end process;
--------------------------------------------------------------------------------
-- CSR Write
   s_locDataIn <= unsigned(locDataIn);
   s_CrCsrOffsetAderIndex  <= s_CrCsrOffsetAddr - 
                              (c_FUNC0_ADER_3_addr(18 downto 0) srl 2) + FUNC0_ADER_3;
   p_CSR_Write : process(clk_i)
   begin
      if rising_edge(clk_i) then
         if reset = '1'  then
            s_CSRarray(BAR) <= (others => '0');
            s_bar_written   <= '0';
            for i in 254 downto WB32bits loop        -- Initialization of the CSR memory
               s_CSRarray(i) <= c_csr_array(i);
            end loop;
         elsif s_bar_written = '0' and s_odd_parity = '1' then  
            -- initialization of BAR reg to access the CR/CSR space
				if g_base_addr = GEOGRAPHICAL_ADDR then
					s_CSRarray(BAR)(7 downto 3) <= unsigned(not VME_GA_oversampled(4 downto 0));   
				else --MECHANICALLY
				   s_CSRarray(BAR)(7 downto 3) <= unsigned(VME_GA_oversampled(4 downto 0));   
				end if;
				s_CSRarray(BAR)(2 downto 0) <= "000";
            s_bar_written <= '1';
		   elsif (g_base_addr = MECHANICALLY and (s_VME_GA_pre /=	VME_GA_oversampled)) then 
			-- updates the based adddress if there was a change and the base addres is set with switches
					s_bar_written <= '0';
         elsif s_odd_parity = '0' then		
			   s_CSRarray(BAR) <= (others => '0');
         elsif (en_wr_CSR = '1') then  
            case to_integer(s_CrCsrOffsetAddr) is    
               when to_integer("00" & c_BAR_addr(18 downto 2)) =>       
                  s_CSRarray(BAR) <= s_locDataIn(7 downto 0);
                  s_bar_written   <= '1';
               when to_integer("00" & c_BIT_SET_REG_addr(18 downto 2)) =>
                  for i in 0 to 7 loop
                     s_CSRarray(BIT_SET_CLR_REG)(i) <= s_locDataIn(i);
                  end loop;

               when to_integer("00" & c_BIT_CLR_REG_addr(18 downto 2)) => 
                  for i in 0 to 7 loop
                     if s_locDataIn(i) = '1' and i = 2 then
                        s_CSRarray(BIT_SET_CLR_REG)(i) <= '0';
                        s_CSRarray(CRAM_OWNER)         <= x"00";
                     elsif  s_locDataIn(i) = '1' and i = 3 then
                        reset_flag <= '1';
                     else 
                        if s_locDataIn(i) = '1' then
                           s_CSRarray(BIT_SET_CLR_REG)(i) <= '0';
                        end if;
                     end if;
                  end loop;

               when to_integer("00" & c_CRAM_OWNER_addr(18 downto 2)) =>	
                  if s_CSRarray(CRAM_OWNER) = x"00" and s_locDataIn(7 downto 0) /= x"00" then
                     -- Write register give ownership only if register value is 0
                     s_CSRarray(CRAM_OWNER) <= s_locDataIn(7 downto 0);  
                     s_CSRarray(BIT_SET_CLR_REG)(2) <= '1';
                  end if;

               when to_integer("00" & c_USR_BIT_SET_REG_addr(18 downto 2)) =>
                  s_CSRarray(USR_BIT_SET_CLR_REG) <= s_locDataIn(7 downto 0);
               
               when to_integer("00" & c_USR_BIT_CLR_REG_addr(18 downto 2)) =>
                  for i in 0 to 7 loop
                     if s_locDataIn(i) = '1' then
                        s_CSRarray(USR_BIT_SET_CLR_REG)(i) <= '0';
                     end if;
                  end loop;
               
               when to_integer("00" & c_FUNC0_ADER_3_addr(18 downto 2)) to 
                    to_integer("00" & c_FUNC7_ADER_0_addr(18 downto 2)) => 
                  s_CSRarray(to_integer(s_CrCsrOffsetAderIndex)) <= s_locDataIn(7 downto 0);
               
               when to_integer("00" & c_IRQ_Vector_addr(18 downto 2)) =>
                  s_CSRarray(IRQ_Vector) <= s_locDataIn(7 downto 0);
               
               when to_integer("00" & c_IRQ_level_addr(18 downto 2)) =>
                  s_CSRarray(IRQ_level) <= s_locDataIn(7 downto 0);
               
               when to_integer("00" & c_Endian_addr(18 downto 2)) =>
                  s_CSRarray(Endian) <= s_locDataIn(7 downto 0);			
						
               when others => null;   
            end case;	

         else
			   if g_wb_data_width = 32 then
				   s_CSRarray(WB32bits) <= x"01";
				else
				   s_CSRarray(WB32bits) <= x"00";
				end if;	
            reset_flag           <= '0';
            s_CSRarray(BYTES0)   <= unsigned(numBytes(7 downto 0));
            s_CSRarray(BYTES1)   <= resize(unsigned(numBytes(12 downto 8)),8);
            s_CSRarray(TIME0_ns) <= unsigned(transfTime(7 downto 0));
            s_CSRarray(TIME1_ns) <= unsigned(transfTime(15 downto 8));
            s_CSRarray(TIME2_ns) <= unsigned(transfTime(23 downto 16));
            s_CSRarray(TIME3_ns) <= unsigned(transfTime(31 downto 24));
            s_CSRarray(TIME4_ns) <= unsigned(transfTime(39 downto 32));
				
         end if;
      end if;	
   end process;
  ------------------------------------------------------------------------------------------------------------------------------------
  --CSR Read
   process(s_CSRarray, s_CrCsrOffsetAddr,err_flag)
   begin
      s_CSRdata <= (others => '0');
      case (s_CrCsrOffsetAddr) is
         when "00" & c_BAR_addr(18 downto 2)             => s_CSRdata <= s_CSRarray(BAR); 
         when "00" & c_BIT_SET_REG_addr(18 downto 2)     => s_CSRdata <= s_CSRarray(
              BIT_SET_CLR_REG)(7 downto 4) & err_flag & s_CSRarray(BIT_SET_CLR_REG)(2 downto 0); 
         when "00" & c_BIT_CLR_REG_addr(18 downto 2)     => s_CSRdata <= s_CSRarray(
              BIT_SET_CLR_REG)(7 downto 4) & err_flag & s_CSRarray(BIT_SET_CLR_REG)(2 downto 0);
         when "00" & c_CRAM_OWNER_addr(18 downto 2)      => s_CSRdata <= s_CSRarray(CRAM_OWNER);
         when "00" & c_USR_BIT_SET_REG_addr(18 downto 2) => s_CSRdata <= s_CSRarray(
              USR_BIT_SET_CLR_REG);
         when "00" & c_USR_BIT_CLR_REG_addr(18 downto 2) => s_CSRdata <= s_CSRarray(
              USR_BIT_SET_CLR_REG);
         when "00" & c_FUNC7_ADER_0_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC7_ADER_0);
         when "00" & c_FUNC7_ADER_1_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC7_ADER_1);
         when "00" & c_FUNC7_ADER_2_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC7_ADER_2);
         when "00" & c_FUNC7_ADER_3_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC7_ADER_3);
         when "00" & c_FUNC6_ADER_0_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC6_ADER_0);
         when "00" & c_FUNC6_ADER_1_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC6_ADER_1);
         when "00" & c_FUNC6_ADER_2_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC6_ADER_2);
         when "00" & c_FUNC6_ADER_3_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC6_ADER_3);
         when "00" & c_FUNC5_ADER_0_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC5_ADER_0);
         when "00" & c_FUNC5_ADER_1_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC5_ADER_1);
         when "00" & c_FUNC5_ADER_2_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC5_ADER_2);
         when "00" & c_FUNC5_ADER_3_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC5_ADER_3);
         when "00" & c_FUNC4_ADER_0_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC4_ADER_0);
         when "00" & c_FUNC4_ADER_1_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC4_ADER_1);
         when "00" & c_FUNC4_ADER_2_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC4_ADER_2);
         when "00" & c_FUNC4_ADER_3_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC4_ADER_3);
         when "00" & c_FUNC3_ADER_0_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC3_ADER_0);
         when "00" & c_FUNC3_ADER_1_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC3_ADER_1);
         when "00" & c_FUNC3_ADER_3_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC3_ADER_3);
         when "00" & c_FUNC2_ADER_0_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC2_ADER_0);
         when "00" & c_FUNC2_ADER_1_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC2_ADER_1);
         when "00" & c_FUNC2_ADER_2_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC2_ADER_2);
         when "00" & c_FUNC2_ADER_3_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC2_ADER_3);
         when "00" & c_FUNC1_ADER_0_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC1_ADER_0);
         when "00" & c_FUNC1_ADER_1_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC1_ADER_1);
         when "00" & c_FUNC1_ADER_2_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC1_ADER_2);
         when "00" & c_FUNC1_ADER_3_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC1_ADER_3);
         when "00" & c_FUNC0_ADER_0_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC0_ADER_0);
         when "00" & c_FUNC0_ADER_1_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC0_ADER_1);
         when "00" & c_FUNC0_ADER_2_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC0_ADER_2);
         when "00" & c_FUNC0_ADER_3_addr(18 downto 2)    => s_CSRdata <= s_CSRarray(FUNC0_ADER_3);
         when "00" & c_IRQ_Vector_addr (18 downto 2)     => s_CSRdata <= s_CSRarray(IRQ_Vector);
         when "00" & c_IRQ_level_addr(18 downto 2)       => s_CSRdata <= s_CSRarray(IRQ_level);
         when "00" & c_Endian_addr(18 downto 2)          => s_CSRdata <= s_CSRarray(Endian);
         when "00" & c_TIME0_ns_addr(18 downto 2)        => s_CSRdata <= s_CSRarray(TIME0_ns);
         when "00" & c_TIME1_ns_addr(18 downto 2)        => s_CSRdata <= s_CSRarray(TIME1_ns);
         when "00" & c_TIME2_ns_addr(18 downto 2)        => s_CSRdata <= s_CSRarray(TIME2_ns);
         when "00" & c_TIME3_ns_addr(18 downto 2)        => s_CSRdata <= s_CSRarray(TIME3_ns);
         when "00" & c_TIME4_ns_addr(18 downto 2)        => s_CSRdata <= s_CSRarray(TIME4_ns);
         when "00" & c_BYTES0_addr(18 downto 2)          => s_CSRdata <= s_CSRarray(BYTES0);
         when "00" & c_BYTES1_addr(18 downto 2)          => s_CSRdata <= s_CSRarray(BYTES1);
			when "00" & c_WB32bits_addr(18 downto 2)        => s_CSRdata <= s_CSRarray(WB32bits);
         when others                                     => s_CSRdata <= (others => '0');
      end case;

   end process;

   INT_Level         <= std_logic_vector(s_CSRarray(IRQ_level));
   INT_Vector        <= std_logic_vector(s_CSRarray(IRQ_Vector));
   CSRdata           <= std_logic_vector(s_CSRdata);
   s_CrCsrOffsetAddr <= unsigned(CrCsrOffsetAddr);
-- Generate a vector of 8 array (unsigned 32 bits).
   GADER_1 : for i in 0 to 7 generate
      GADER_2 : for h in 0 to 3 generate
         s_FUNC_ADER(i)(8*(4-h)-1 downto 8*(3-h)) <= s_CSRarray(FUNC0_ADER_3+(h+i*4));
      end generate GADER_2;
   end generate GADER_1;
-- to the decoder
   Ader0         <= std_logic_vector(s_FUNC_ADER(0));
   Ader1         <= std_logic_vector(s_FUNC_ADER(1));
   Ader2         <= std_logic_vector(s_FUNC_ADER(2));
   Ader3         <= std_logic_vector(s_FUNC_ADER(3));
   Ader4         <= std_logic_vector(s_FUNC_ADER(4));
   Ader5         <= std_logic_vector(s_FUNC_ADER(5));
   Ader6         <= std_logic_vector(s_FUNC_ADER(6));
   Ader7         <= std_logic_vector(s_FUNC_ADER(7));
   ModuleEnable  <= s_CSRarray(BIT_SET_CLR_REG)(4);
   Endian_o      <= std_logic_vector(s_CSRarray(Endian)(2 downto 0));
   Sw_Reset      <= s_CSRarray(BIT_SET_CLR_REG)(7);
   BAR_o	        <= s_BAR_o;
   s_BAR_o       <= std_logic_vector(s_CSRarray(BAR)(7 downto 3));
---------------------------------------------------------------------------------------------------------------
-- CRAM:
   CRAM_1 : VME_CRAM
   generic map(dl => 8,                   
               al => f_log2_size(g_cram_size)
               )					
   port map(clk => clk_i,            
            we  => CRAM_Wen,            
            aw  => CRAM_addr,            
            di  => CRAM_data_i,         
            dw  => CRAM_data_o);          
end Behavioral;
--===========================================================================
-- Architecture end
--===========================================================================
