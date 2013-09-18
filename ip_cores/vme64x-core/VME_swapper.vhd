--______________________________________________________________________________
--                             VME TO WB INTERFACE
--
--                                CERN,BE/CO-HT 
--______________________________________________________________________________
-- File:                           VME_swapper.vhd
--______________________________________________________________________________
-- Description: 
--sel= 00 --> No swap
--sel= 01 --> Swap Byte            eg: 01234567 became 10325476
--sel= 10 --> Swap Word            eg: 01234567 became 23016745
--sel= 11 --> Swap Word+ Swap Byte eg: 01234567 became 32107654
--______________________________________________________________________________
-- Authors:                                                                    
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
--===========================================================================
-- Entity declaration
--===========================================================================
entity VME_swapper is
    Port ( d_i : in  STD_LOGIC_VECTOR (63 downto 0);
	        sel : in  STD_LOGIC_VECTOR (2 downto 0);
           d_o : out  STD_LOGIC_VECTOR (63 downto 0));
end VME_swapper;
--===========================================================================
-- Architecture declaration
--===========================================================================
architecture Behavioral of VME_swapper is
signal Byte0_i : std_logic_vector(7 downto 0);
signal Byte1_i : std_logic_vector(7 downto 0);
signal Byte2_i : std_logic_vector(7 downto 0);
signal Byte3_i : std_logic_vector(7 downto 0);
signal Byte4_i : std_logic_vector(7 downto 0);
signal Byte5_i : std_logic_vector(7 downto 0);
signal Byte6_i : std_logic_vector(7 downto 0);
signal Byte7_i : std_logic_vector(7 downto 0);
signal Byte0_o : std_logic_vector(7 downto 0);
signal Byte1_o : std_logic_vector(7 downto 0);
signal Byte2_o : std_logic_vector(7 downto 0);
signal Byte3_o : std_logic_vector(7 downto 0);
signal Byte4_o : std_logic_vector(7 downto 0);
signal Byte5_o : std_logic_vector(7 downto 0);
signal Byte6_o : std_logic_vector(7 downto 0);
signal Byte7_o : std_logic_vector(7 downto 0);
--===========================================================================
-- Architecture begin
--===========================================================================
begin

process (sel,Byte0_i,Byte1_i,Byte2_i,Byte3_i,Byte7_i)
begin
   case sel is
      when "000"  => Byte0_o <= Byte0_i;
      when "001"  => Byte0_o <= Byte1_i;
      when "010"  => Byte0_o <= Byte2_i;
      when "011"  => Byte0_o <= Byte3_i;
		when "100"  => Byte0_o <= Byte7_i;
      when others => Byte0_o <= Byte0_i;
   end case;
end process;

process (sel,Byte0_i,Byte1_i,Byte2_i,Byte3_i,Byte6_i)
begin
   case sel is
      when "000"  => Byte1_o <= Byte1_i;
      when "001"  => Byte1_o <= Byte0_i;
      when "010"  => Byte1_o <= Byte3_i;
      when "011"  => Byte1_o <= Byte2_i;
		when "100"  => Byte1_o <= Byte6_i;
      when others => Byte1_o <= Byte1_i;
   end case;
end process;

process (sel,Byte0_i,Byte1_i,Byte2_i,Byte3_i,Byte5_i)
begin
   case sel is
      when "000"   => Byte2_o   <= Byte2_i;
      when "001"   => Byte2_o   <= Byte3_i;
      when "010"   => Byte2_o   <= Byte0_i;
      when "011"   => Byte2_o   <= Byte1_i;
		when "100"   => Byte2_o   <= Byte5_i;
      when others  => Byte2_o   <= Byte2_i;
   end case;
end process;

process (sel,Byte0_i,Byte1_i,Byte2_i,Byte3_i,Byte4_i)
begin
   case sel is
      when "000"  => Byte3_o <= Byte3_i;
      when "001"  => Byte3_o <= Byte2_i;
      when "010"  => Byte3_o <= Byte1_i;
      when "011"  => Byte3_o <= Byte0_i;
		when "100"  => Byte3_o <= Byte4_i;
      when others => Byte3_o <= Byte3_i;
   end case;
end process;

process (sel,Byte4_i,Byte5_i,Byte6_i,Byte7_i,Byte3_i)
begin
   case sel is
      when "000"  => Byte4_o <= Byte4_i;
      when "001"  => Byte4_o <= Byte5_i;
      when "010"  => Byte4_o <= Byte6_i;
      when "011"  => Byte4_o <= Byte7_i;
		when "100"  => Byte4_o <= Byte3_i;
      when others => Byte4_o <= Byte4_i;
   end case;
end process;

process (sel,Byte4_i,Byte5_i,Byte6_i,Byte7_i,Byte2_i)
begin
   case sel is
      when "000"  => Byte5_o <= Byte5_i;
      when "001"  => Byte5_o <= Byte4_i;
      when "010"  => Byte5_o <= Byte7_i;
      when "011"  => Byte5_o <= Byte6_i;
		when "100"  => Byte5_o <= Byte2_i;
      when others => Byte5_o <= Byte5_i;
   end case;
end process;

process (sel,Byte4_i,Byte5_i,Byte6_i,Byte7_i,Byte1_i)
begin
   case sel is
      when "000"  => Byte6_o <= Byte6_i;
      when "001"  => Byte6_o <= Byte7_i;
      when "010"  => Byte6_o <= Byte4_i;
      when "011"  => Byte6_o <= Byte5_i;
		when "100"  => Byte6_o <= Byte1_i;
      when others => Byte6_o <= Byte6_i;
   end case;
end process;

process (sel,Byte4_i,Byte5_i,Byte6_i,Byte7_i,Byte0_i)
begin
   case sel is
      when "000"  => Byte7_o <= Byte7_i;
      when "001"  => Byte7_o <= Byte6_i;
      when "010"  => Byte7_o <= Byte5_i;
      when "011"  => Byte7_o <= Byte4_i;
		when "100"  => Byte7_o <= Byte0_i;
      when others => Byte7_o <= Byte7_i;
   end case;
end process;

Byte0_i           <= d_i(7 downto 0);
Byte1_i           <= d_i(15 downto 8);
Byte2_i           <= d_i(23 downto 16);
Byte3_i           <= d_i(31 downto 24);
Byte4_i           <= d_i(39 downto 32);
Byte5_i           <= d_i(47 downto 40);
Byte6_i           <= d_i(55 downto 48);
Byte7_i           <= d_i(63 downto 56);
d_o(7 downto 0)   <= Byte0_o;
d_o(15 downto 8)  <= Byte1_o;
d_o(23 downto 16) <= Byte2_o;
d_o(31 downto 24) <= Byte3_o;
d_o(39 downto 32) <= Byte4_o;
d_o(47 downto 40) <= Byte5_o;
d_o(55 downto 48) <= Byte6_o;
d_o(63 downto 56) <= Byte7_o;

end Behavioral;
--===========================================================================
-- Architecture end
--===========================================================================
