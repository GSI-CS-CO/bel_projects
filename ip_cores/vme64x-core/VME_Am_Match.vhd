--_______________________________________________________________________________________
--                             VME TO WB INTERFACE
--
--                                CERN,BE/CO-HT 
--______________________________________________________________________________________
-- File:                       VME_Am_Match.vhd
--______________________________________________________________________________________
-- Description: this component checks if the AM match. 
-- If it is the correspondent AmMatch's bit is asserted. This condition is necessary but 
-- not sufficient to select the function and access the board.
-- If DFS = '0' the function supports only access modes with the same address width; 
-- 1 function --> only 1 address width;
-- with address width I mean A16, A24, A32 or A64.
-- is sufficient check the AMCAP; AmMatch(i) <= s_FUNC_AMCAP(i)(to_integer(unsigned(Am))).
-- If DFS = '1' the function supports access modes with different address widths so AmMatch(i)
-- is asserted only if ADER[7:2] = AM and s_FUNC_AMCAP(i)(to_integer(unsigned(Am)))='1'.
-- If ADER(i)'s XAM bit is asserted than AmMatch(i) is asserted only if AM = 0x20 and if the 
-- XAMCAP(i)(to_integer(unsigned(XAm))) = '1' and if DFS = '1' also ADER[9:2] must be equal 
-- to XAM[7:0] lines.
--______________________________________________________________________________________
-- Authors:                                   
--               Pablo Alvarez Sanchez (Pablo.Alvarez.Sanchez@cern.ch)                             
--               Davide Pedretti       (Davide.Pedretti@cern.ch)  
-- Date         11/2012                                                                           
-- Version      v0.03 
--______________________________________________________________________________________
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
----------------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use work.xvme64x_pack.all;

--===========================================================================
-- Entity declaration
--===========================================================================

entity VME_Am_Match is
   Port ( clk_i          : in  std_logic;
          reset          : in  std_logic;
          mainFSMreset   : in  std_logic;
          Ader0          : in  std_logic_vector (31 downto 0);
          Ader1          : in  std_logic_vector (31 downto 0);
          Ader2          : in  std_logic_vector (31 downto 0);
          Ader3          : in  std_logic_vector (31 downto 0);
          Ader4          : in  std_logic_vector (31 downto 0);
          Ader5          : in  std_logic_vector (31 downto 0);
          Ader6          : in  std_logic_vector (31 downto 0);			  
          Ader7          : in  std_logic_vector (31 downto 0);	
          AmCap0         : in  std_logic_vector (63 downto 0);
          AmCap1         : in  std_logic_vector (63 downto 0);
          AmCap2         : in  std_logic_vector (63 downto 0);
          AmCap3         : in  std_logic_vector (63 downto 0);
          AmCap4         : in  std_logic_vector (63 downto 0);
          AmCap5         : in  std_logic_vector (63 downto 0);
          AmCap6         : in  std_logic_vector (63 downto 0);
          AmCap7         : in  std_logic_vector (63 downto 0);
          XAmCap0        : in  std_logic_vector (255 downto 0);
          XAmCap1        : in  std_logic_vector (255 downto 0);
          XAmCap2        : in  std_logic_vector (255 downto 0);
          XAmCap3        : in  std_logic_vector (255 downto 0);
          XAmCap4        : in  std_logic_vector (255 downto 0);
          XAmCap5        : in  std_logic_vector (255 downto 0);
          XAmCap6        : in  std_logic_vector (255 downto 0);
          XAmCap7        : in  std_logic_vector (255 downto 0);
          Am             : in  std_logic_vector (5 downto 0);
          XAm            : in  std_logic_vector (7 downto 0);  
          DFS_i          : in  std_logic_vector (7 downto 0);
          decode         : in  std_logic;
          AmMatch        : out std_logic_vector (7 downto 0));
end VME_Am_Match;
--===========================================================================
-- Architecture declaration
--===========================================================================
architecture Behavioral of VME_Am_Match is
   signal s_FUNC_ADER    : t_FUNC_32b_array;
   signal s_FUNC_AMCAP   : t_FUNC_64b_array;
   signal s_FUNC_XAMCAP  : t_FUNC_256b_array;
   signal s_amcap_match  : std_logic_vector(7 downto 0);
   signal s_xamcap_match : std_logic_vector(7 downto 0);
--===========================================================================
-- Architecture begin
--===========================================================================
begin

   s_FUNC_ADER(0)   <= unsigned(Ader0);
   s_FUNC_ADER(1)   <= unsigned(Ader1);
   s_FUNC_ADER(2)   <= unsigned(Ader2);
   s_FUNC_ADER(3)   <= unsigned(Ader3);
   s_FUNC_ADER(4)   <= unsigned(Ader4);
   s_FUNC_ADER(5)   <= unsigned(Ader5);
   s_FUNC_ADER(6)   <= unsigned(Ader6);
   s_FUNC_ADER(7)   <= unsigned(Ader7);

   s_FUNC_AMCAP(0)  <= unsigned(AmCap0);
   s_FUNC_AMCAP(1)  <= unsigned(AmCap1);
   s_FUNC_AMCAP(2)  <= unsigned(AmCap2);
   s_FUNC_AMCAP(3)  <= unsigned(AmCap3);
   s_FUNC_AMCAP(4)  <= unsigned(AmCap4);
   s_FUNC_AMCAP(5)  <= unsigned(AmCap5);
   s_FUNC_AMCAP(6)  <= unsigned(AmCap6);
   s_FUNC_AMCAP(7)  <= unsigned(AmCap7);

   s_FUNC_XAMCAP(0) <= unsigned(XAmCap0);
   s_FUNC_XAMCAP(1) <= unsigned(XAmCap1);
   s_FUNC_XAMCAP(2) <= unsigned(XAmCap2);
   s_FUNC_XAMCAP(3) <= unsigned(XAmCap3);
   s_FUNC_XAMCAP(4) <= unsigned(XAmCap4);
   s_FUNC_XAMCAP(5) <= unsigned(XAmCap5);
   s_FUNC_XAMCAP(6) <= unsigned(XAmCap6);
   s_FUNC_XAMCAP(7) <= unsigned(XAmCap7);

   p_AMmatch : process(clk_i)
   begin
      if rising_edge(clk_i) then  
         if mainFSMreset = '1' or reset = '1' then
            AmMatch <= (others => '0');
         elsif decode = '1' then	  
            for i in AmMatch'range loop
               if DFS_i(i) = '1' then
                  if s_FUNC_ADER(i)(XAM_MODE) = '0' then
                     if unsigned(s_FUNC_ADER(i)(7 downto 2)) = unsigned(Am) then
                        AmMatch(i) <= s_amcap_match(i);                   
                     else
                        AmMatch(i) <= '0';
                     end if;
                  else
                     if (unsigned(XAm) = unsigned(s_FUNC_ADER(i)(9 downto 2))) then
                        AmMatch(i) <= s_xamcap_match(i) and s_amcap_match(i);                 
                     else
                        AmMatch(i) <= '0';
                     end if;
                  end if;
               else  
                  if s_FUNC_ADER(i)(XAM_MODE) = '1' then
                     AmMatch(i) <= s_xamcap_match(i) and s_amcap_match(i);                        				
                  else
                     AmMatch(i) <= s_amcap_match(i);              
                  end if;
               end if;		
            end loop;
         end if;	
      end if;
   end process;
------------------------------------------------------
-- Check if the AM is in the AMCAP register
   process(s_FUNC_AMCAP, Am)
   begin
      s_amcap_match <= (others => '0');
      for i in 0 to 7 loop
         s_amcap_match(i) <= s_FUNC_AMCAP(i)(to_integer(unsigned(Am)));  
      end loop;
   end process;
-------------------------------------------------------
-- Check if the XAM is in the XAMCAP register    
   process(s_FUNC_XAMCAP, XAm)
   begin
      s_xamcap_match <= (others => '0');
      for i in 0 to 7 loop    
         s_xamcap_match(i) <= s_FUNC_XAMCAP(i)(to_integer(unsigned(XAm)));
      end loop;
   end process;
------------------------------------------------------
end Behavioral;
--===========================================================================
-- Architecture end
--===========================================================================
