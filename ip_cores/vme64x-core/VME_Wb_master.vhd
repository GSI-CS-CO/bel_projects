--___________________________________________________________________________________
--                              VME TO WB INTERFACE
--
--                                CERN,BE/CO-HT 
--___________________________________________________________________________________
-- File:                           VME_Wb_master.vhd
--___________________________________________________________________________________
-- Description:
-- This component implements the WB master side in the vme64x core.
-- Work mode:
--            PIPELINED 
--            SINGLE READ/WRITE
--
-- The WB bus can be 64 bit wide or 32 bit wide and the data organization is BIG ENDIAN 
-- --> the most significant byte is carried in the lower position of the bus.
-- Eg:
--   _______________________________________________________________________
--  | Byte(0)| Byte(1)| Byte(2)| Byte(3)| Byte(4)| Byte(5)| Byte(6)| Byte(7)|
--  |________|________|________|________|________|________|________|________|
--   D[63:56] D[55:48] D[47:40] D[39:32] D[31:24] D[23:16] D[15:8]  D[7:0]
--
-- eg of timing diagram with synchronous WB Slave:
--             
--       Clk   _____       _____       _____       _____       _____       _____       _____
--       _____|     |_____|     |_____|     |_____|     |_____|     |_____|     |_____|     
--      cyc_o  ____________________________________________________________
--       _____|                                                            |________________
--      stb_o  ________________________________________________
--       _____|                                                |____________________________
--       __________________________________________
--      stall_i                                    |________________________________________
--      ack_i                                                   ___________
--       ______________________________________________________|           |________________       
--
-- The ack_i can be asserted with some Tclk of delay, not immediately.
-- This component implements the correct shift of the data in input/output from/to WB bus
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
-- This source file is free software; you can redistribute it and/or modify it 
-- under the terms of the GNU Lesser General Public License as published by the 
-- Free Software Foundation; either version 2.1 of the License, or (at your option) 
-- any later version. This source is distributed in the hope that it will be useful, 
-- but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
-- FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for 
-- more details. You should have received a copy of the GNU Lesser General Public 
-- License along with this source; if not, download it from 
-- http://www.gnu.org/licenses/lgpl-2.1.html                     
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.numeric_std.all;
use work.xvme64x_pack.all;

--===========================================================================
-- Entity declaration
--===========================================================================
entity VME_Wb_master is
   generic(g_wb_data_width : integer := c_width;
	        g_wb_addr_width : integer := c_addr_width
	);
   Port ( memReq_i        : in   std_logic;
          clk_i           : in   std_logic;
          cardSel_i       : in   std_logic;
          reset_i         : in   std_logic;
          BERRcondition_i : in   std_logic;
          sel_i           : in   std_logic_vector(7 downto 0);
          locDataInSwap_i : in   std_logic_vector(63 downto 0);
          locDataOut_o    : out  std_logic_vector(63 downto 0);
          rel_locAddr_i   : in   std_logic_vector(63 downto 0);
          memAckWb_o      : out  std_logic;
          err_o           : out  std_logic;
          rty_o           : out  std_logic;
          RW_i            : in   std_logic;
          stall_i         : in   std_logic;
          rty_i           : in   std_logic;
          err_i           : in   std_logic;
          cyc_o           : out  std_logic;
          memReq_o        : out  std_logic;
          WBdata_o        : out  std_logic_vector(g_wb_data_width - 1 downto 0);
          wbData_i        : in   std_logic_vector(g_wb_data_width - 1 downto 0);
          locAddr_o       : out  std_logic_vector(g_wb_addr_width - 1 downto 0);
          memAckWB_i      : in   std_logic;
          WbSel_o         : out  std_logic_vector(f_div8(g_wb_data_width) - 1 downto 0);
          RW_o            : out  std_logic);
end VME_Wb_master;

--===========================================================================
-- Architecture declaration
--===========================================================================
architecture Behavioral of VME_Wb_master is
signal s_shift_dx     :   std_logic;
signal s_cyc          :   std_logic;
signal s_AckWithError :   std_logic;
signal s_wbData_i     :   std_logic_vector(63 downto 0);
signal s_select       :   std_logic_vector(8 downto 0);
signal s_DATi_sample  :   std_logic_vector(g_wb_data_width - 1 downto 0);
--===========================================================================
-- Architecture begin
--===========================================================================
begin

s_select <= cardSel_i & sel_i;

s_wbData_i <= std_logic_vector(resize(unsigned(s_DATi_sample),s_wbData_i'length));
-- stb handler
  process(clk_i)
   begin
      if rising_edge(clk_i) then
         if reset_i = '1' or (stall_i = '0' and s_cyc = '1') then
            memReq_o <= '0';
         elsif memReq_i = '1' and cardSel_i = '1' and BERRcondition_i = '0' then	 
            memReq_o <= '1';
         end if;
      end if;
   end process;
	
-- cyc_o handler
   process(clk_i)
   begin
      if rising_edge(clk_i) then
         if reset_i = '1' or memAckWB_i = '1' then
            s_cyc <= '0';
         elsif memReq_i = '1' and cardSel_i = '1' and BERRcondition_i = '0' then	 
            s_cyc <= '1';
         end if;
      end if;
   end process;
	cyc_o  <= s_cyc;
	
  process(clk_i)
  begin
      if rising_edge(clk_i) then
         RW_o           <= RW_i;
         s_AckWithError <=(memReq_i and cardSel_i and BERRcondition_i);        
      end if;
  end process;
	
-- shift data and address for WB data bus 64 bits 
  gen64: if (g_wb_data_width = 64) generate

         process(clk_i)
         begin
	         if rising_edge(clk_i) then
              locAddr_o <= std_logic_vector(resize(unsigned(rel_locAddr_i) srl 3,g_wb_addr_width));
	         end if;
	      end process;

         process(clk_i)
         begin
            if rising_edge(clk_i) then
				   case sel_i is                                          
                 when "10000000" => WBdata_o <= std_logic_vector(unsigned(locDataInSwap_i) sll 56); 
                 when "01000000" => WBdata_o <= std_logic_vector(unsigned(locDataInSwap_i) sll 48); 
                 when "00100000" => WBdata_o <= std_logic_vector(unsigned(locDataInSwap_i) sll 40); 
					  when "00010000" => WBdata_o <= std_logic_vector(unsigned(locDataInSwap_i) sll 32); 
                 when "00001000" => WBdata_o <= std_logic_vector(unsigned(locDataInSwap_i) sll 24);                                  
                 when "00000100" => WBdata_o <= std_logic_vector(unsigned(locDataInSwap_i) sll 16);                  
                 when "00000010" => WBdata_o <= std_logic_vector(unsigned(locDataInSwap_i) sll 8);                    
                 when "11000000" => WBdata_o <= std_logic_vector(unsigned(locDataInSwap_i) sll 48);                           
                 when "00110000" => WBdata_o <= std_logic_vector(unsigned(locDataInSwap_i) sll 32);                 
                 when "00001100" => WBdata_o <= std_logic_vector(unsigned(locDataInSwap_i) sll 16);                      
                 when "11110000" => WBdata_o <= std_logic_vector(unsigned(locDataInSwap_i) sll 32);
                 when "00001111" => WBdata_o <= locDataInSwap_i;                                          
                 when "00000001" => WBdata_o <= locDataInSwap_i;
                 when "00000011" => WBdata_o <= locDataInSwap_i;
					  when "11111111" => WBdata_o <= locDataInSwap_i;
					  when others => null;
               end case;                                 
			      WbSel_o  <= std_logic_vector(sel_i);
		      end if;
         end process;		
         
			process (s_select,s_wbData_i)
         begin
           case s_select is
               when "100000010" => locDataOut_o <= std_logic_vector(		
					     resize(unsigned(s_wbData_i(15 downto 0)) srl 8, locDataOut_o'length));
               when "100000100" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(23 downto 0)) srl 16,locDataOut_o'length));
               when "100001000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(31 downto 0)) srl 24,locDataOut_o'length));
               when "100010000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(39 downto 0)) srl 32,locDataOut_o'length));
               when "100100000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(47 downto 0)) srl 40,locDataOut_o'length));
					when "101000000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(55 downto 0)) srl 48,locDataOut_o'length));
					when "110000000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i) srl 56,locDataOut_o'length));
					when "100001100" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(31 downto 0)) srl 16,locDataOut_o'length));
					when "100110000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(47 downto 0)) srl 32,locDataOut_o'length));
					when "111000000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i) srl 48,locDataOut_o'length));
					when "100000001" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(7 downto 0)), locDataOut_o'length));
					when "100000011" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(15 downto 0)), locDataOut_o'length));
					when "100001111" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(31 downto 0)), locDataOut_o'length));
					when "111110000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i) srl 32, locDataOut_o'length));
					when "111111111" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i),locDataOut_o'length));
					when others => locDataOut_o <= (others => '0');
           end case;
         end process;

  end generate gen64; 
		
	-- shift data and address for WB data bus 32 bits 	
		
  gen32: if (g_wb_data_width = 32) generate

			process(clk_i)
         begin
	         if rising_edge(clk_i) then
               locAddr_o <= std_logic_vector(resize(unsigned(rel_locAddr_i) srl 2,g_wb_addr_width));
	         end if;
	      end process;
			
			process(sel_i)
         begin
             if sel_i = "10000000" or  sel_i = "01000000" or sel_i = "00100000" or sel_i = "00010000" 
			       or sel_i = "11000000" or sel_i = "00110000" or sel_i = "11110000" then
                s_shift_dx <= '1';
             else	 
                s_shift_dx <= '0';
             end if;
         end process;	
			
		   process(clk_i)
         begin
           if rising_edge(clk_i) then
			     case sel_i is                                          
                 when "10000000" => WBdata_o <= std_logic_vector(resize(unsigned(locDataInSwap_i) sll 24,g_wb_data_width)); 
                 when "01000000" => WBdata_o <= std_logic_vector(resize(unsigned(locDataInSwap_i) sll 16,g_wb_data_width)); 
                 when "00100000" => WBdata_o <= std_logic_vector(resize(unsigned(locDataInSwap_i) sll 8,g_wb_data_width)); 
					  when "00010000" => WBdata_o <= std_logic_vector(resize(unsigned(locDataInSwap_i),g_wb_data_width)); 
                 when "00001000" => WBdata_o <= std_logic_vector(resize(unsigned(locDataInSwap_i) sll 24,g_wb_data_width));                                  
                 when "00000100" => WBdata_o <= std_logic_vector(resize(unsigned(locDataInSwap_i) sll 16,g_wb_data_width));                  
                 when "00000010" => WBdata_o <= std_logic_vector(resize(unsigned(locDataInSwap_i) sll 8,g_wb_data_width));                    
                 when "11000000" => WBdata_o <= std_logic_vector(resize(unsigned(locDataInSwap_i) sll 16,g_wb_data_width));                           
                 when "00110000" => WBdata_o <= std_logic_vector(resize(unsigned(locDataInSwap_i),g_wb_data_width));                 
                 when "00001100" => WBdata_o <= std_logic_vector(resize(unsigned(locDataInSwap_i) sll 16,g_wb_data_width));                      
                 when "11110000" => WBdata_o <= std_logic_vector(resize(unsigned(locDataInSwap_i),g_wb_data_width));
                 when "00001111" => WBdata_o <= std_logic_vector(resize(unsigned(locDataInSwap_i),g_wb_data_width));                                          
                 when "00000001" => WBdata_o <= std_logic_vector(resize(unsigned(locDataInSwap_i),g_wb_data_width));
                 when "00000011" => WBdata_o <= std_logic_vector(resize(unsigned(locDataInSwap_i),g_wb_data_width));
					  when "11111111" => WBdata_o <= std_logic_vector(resize(unsigned(locDataInSwap_i),g_wb_data_width));
					  when others => null;
               end case;                        
			  
			     if s_shift_dx = '1' then
			        WbSel_o  <= sel_i(7 downto 4);  
			     else
			        WbSel_o  <= sel_i(3 downto 0);
              end if;			  		  
           end if;	
         end process;
		
		   process (s_select,s_wbData_i)
         begin
           case s_select is
               when "100000010" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(15 downto 0)) srl 8, locDataOut_o'length));
               when "100000100" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(23 downto 0)) srl 16,locDataOut_o'length));
               when "100001000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(31 downto 0)) srl 24,locDataOut_o'length));
               when "100010000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(7 downto 0)),locDataOut_o'length));
               when "100100000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(15 downto 0)) srl 8,locDataOut_o'length));
					when "101000000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(23 downto 0)) srl 16,locDataOut_o'length));
					when "110000000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(31 downto 0)) srl 24,locDataOut_o'length));
					when "100001100" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(31 downto 0)) srl 16,locDataOut_o'length));
					when "100110000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(15 downto 0)),locDataOut_o'length));
					when "111000000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(31 downto 0)) srl 16,locDataOut_o'length));
					when "100000001" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(7 downto 0)), locDataOut_o'length));
					when "100000011" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(15 downto 0)), locDataOut_o'length));
					when "100001111" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(31 downto 0)), locDataOut_o'length));
					when "111110000" => locDataOut_o <= std_logic_vector(
					     resize(unsigned(s_wbData_i(31 downto 0)), locDataOut_o'length));
					when others => locDataOut_o <= (others => '0');
           end case;
         end process;
			
  end generate gen32;			               
			  
   err_o <= err_i;
   rty_o <= rty_i; 
   memAckWb_o <= memAckWB_i or s_AckWithError or rty_i;
------------------------------------------------------------------------
    -- This process registers the WB data input; this is a warranty that this
    -- data will be stable during all the time the VME_bus component needs to 
    -- transfers its to the VME bus.
    process(clk_i)
    begin
      if rising_edge(clk_i) then
        if memAckWB_i = '1' then 
           s_DATi_sample <= wbData_i;
        end if;
      end if;
    end process; 

------------------------------------------------------------------------
end Behavioral;
--===========================================================================
-- Architecture end
--===========================================================================
