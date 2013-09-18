--_________________________________________________________________________________________
--                             VME TO WB INTERFACE
--
--                                CERN,BE/CO-HT 
--_________________________________________________________________________________________
-- File:                      VME_Funct_Match.vhd
--_________________________________________________________________________________________
-- Description: this component compares the Address with the ADER using the mask bits and 
-- if the base address match it asserts the corresponding bit in the FunctMatch vector and it
-- latches the base address that will be subtracted to the Address before accessing the WB bus. 
-- FunctMatch  /= 0 is necessary but not sufficient to select one function and to access the board, 
-- indeed also the AM has to be checked (VME_AM_Match.vhd component).
-- For better understanding how this component works here is one example:
--  base address = 0xc0
--  access mode: A32_S  --> AM = 0x09
--  The Master writes the ADERi = 0xc0000024
--  ADEMi = 0xffffff04 --> DFS = '1' --> all the mask bits are '1'!!
-- The Master wants to access the location 0x08: Address= 0xc0000008
--  For i = 0 to 7 check:
--  Check if the ADEMi is compatible with the AM selected: ADEMi[31:8] /= 0
--  Address[31:8] and ADEMi[31:8]              ADERi[31:8] and ADEMi[31:8] 
--                |                                        |
--             0xc00000                                 0xc00000
--                |                 _______                |
--                |________________|  = ?  |_______________|
--                                 |_______| 
--                               No |     |yes
--         FunctMatch(i) <= '0'_____|     |______FunctMatch(i) <= '1'   
--  Now with the same ADEMi the master accesses with A16 mode:
--  base address = 0xc0
--  access mode: A16_S  --> AM = 0x29
--  The Master writes the ADERi = 0x0000c0a4
-- The Master wants to access the location 0x08: Address= 0x0000c008
--  For i = 0 to 7 check:
--  Check if the ADEMi is compatible with the AM selected: ADEMi[15:8] /= 0
--  Address[31:8] and ADEMi[31:8]              ADERi[31:8] and ADEMi[31:8] 
--                |                                        |
--             0x0000c0                                 0x0000c0
--                |                 _______                |
--                |________________|  = ?  |_______________|
--                                 |_______| 
--                               No |     |yes
--         FunctMatch(i) <= '0'_____|     |______FunctMatch(i) <= '1'   
--
--  DFS = '1' --> 1 function --> multiple access modes
-- The Master accesses with different modes only changing the ADER registers if the 
-- DFS bit is asserted but:
-- It is easy to see that if DFS = '1'  we can only address 256 bytes, indeed eg:
--  base address = 0xc0
--  access mode: A32_S  --> AM = 0x09
--  The Master write the ADERi = 0xc0000024
-- The Master wants to access the location 0x4008: Address= 0xc0004008
--  For i = 0 to 7 check:
--  Check if the ADEMi is compatible with the AM selected: ADEMi[31:8] /= 0
--  Address[31:8] and ADEMi[31:8]              ADERi[31:8] and ADEMi[31:8] 
--                |                                        |
--             0xc00040                                 0xc00000
--                |                 _______                |
--                |________________|  = ?  |_______________|
--                                 |_______| 
--                               No |     |yes
--         FunctMatch(i) <= '0'_____|     |______FunctMatch(i) <= '1'   
-- The Master can't access!!
-- Without DFS asserted:
--  base address = 0xc0
--  access mode: A32_S  --> AM = 0x09
--  The Master write the ADERi = 0xc0000024
--  ADEMi = 0xff000000 --> DFS = '0'
--  The Master wants to access the location 0x4008: Address= 0xc0004008
--  For i = 0 to 7 check:
--  Check if the ADEMi is compatible with the AM selected: ADEM[31:8] /= 0
--  Address[31:8] and ADEMi[31:8]              ADERi[31:8] and ADEMi[31:8] 
--                |                                        |
--             0xc00000                                 0xc00000
--                |                 _______                |
--                |________________|  = ?  |_______________|
--                                 |_______| 
--                               No |     |yes
--         FunctMatch(i) <= '0'_____|     |______FunctMatch(i) <= '1'   
--  The Master can access!
--  base address = 0xc0
--  access mode: A16_S  --> AM = 0x29
--  The Master writes the ADERi = 0x0000c0a4
--  ADEMi = 0xff000000 --> DFS = '0'  -- The Master can't change the CR space!!
--  The Master wants to access the location 0x08: Address= 0x0000c008
--  For i = 0 to 7 check:
--  Check if the ADEMi is compatible with the AM selected: 
--  ADEM[15:8] = 0 --> FunctMatch(i) <= '0'
--  The Master can't access! this mask is not compatible with A16
--  
--  DFS = '0' --> 1 function --> only the access modes with the same
--  address width !!
--  Is it possible initialize all the ADER to 0 ?
--  Yes, it is. Indeed now suppose that we are in this situation:
--  ADERi = 0x00000000
--  ADEMi = 0x0000ff00 --> DFS = '0'
--  A VME Master takes the ownership of the VMEbus for accessing another board:
--  base address = 0xc0
--  access mode: A32_S  --> AM = 0x09
--  The Master wants to access the location 0x0008: Address= 0xc0000008
--  For i = 0 to 7 check:
--  Check if the ADEMi is compatible with the AM selected: ADEMi[31:8] /= 0
--   Address[31:8] and ADEMi[31:8]              ADERi[31:8] and ADEMi[31:8] 
--                |                                        |
--             0x000000                                 0x000000
--                |                 _______                |
--                |________________|  = ?  |_______________|
--                                 |_______| 
--                               No |     |yes
--         FunctMatch(i) <= '0'_____|     |______FunctMatch(i) <= '1' 
-- FunctMatch(i) is asserted but our Slave will not be the responding Slave, indeed 
-- the AmMatch(i) is zero becouse the Master is accessing with A32_S and if DFS is 0 
-- the AMCAPi register has only the A16 or A16_SUP bits asserted!
-- If DFS is '1' AmMatch(i) is zero becouse ADER[7:2] is 0 (see VME_Am_Match.vhd) and
-- also FunctMatch(i) is 0 because ADEMi should has all the mask bits '1'.
--
-- An example about A64 access mode:
--  base address = 0xc0
--  access mode: A64_S  --> AM = 0x01
-- ADEM(i) = 0x00000001 --> EFM = '1' and DFS = '0' 
-- ADEM(i+1) = 0xff000000       
-- ADEM64(i) =  ADEM(i+1) &  ADEM(i)                  
-- AMCAP(i) = "0000000000000000000000000000000000000000000000000000000000000010";
-- AMCAP(i+1) <= (others => '0')
-- ADER(i) = 0x00000004
-- ADER(i+1) = 0xc0000000
-- ADER64(i) = ADER(i+1) & ADER(i)
-- s_isprev_func64(i+1) --> '1' --> don't check if the function i + 1 is selected 
-- because the next ADER and ADEM are used to decode the function i.
-- The Master accesses the location 0x0008: Address= 0xc000000000000008
-- Check if the ADEM64i is compatible with the AM selected: ADEM64(i)[63:10] /= 0
-- Address[63:10] and ADEM64(i)[63:10]        ADER64(i)[63:10] and ADEM64(i)[63:10] 
--                |                                        |
--            0xc0000000000000                         0xc0000000000000
--                |                 _______                |
--                |________________|  = ?  |_______________|
--                                 |_______| 
--                               No |     |yes
--         FunctMatch(i) <= '0'_____|     |______FunctMatch(i) <= '1' 
--
-- For the 2e modes it is the same, it changes only the ADER(i)'s XAM bit that must 
-- be '1'.
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
use IEEE.NUMERIC_STD.ALL;
use work.xvme64x_pack.all;

--===========================================================================
-- Entity declaration
--===========================================================================

entity VME_Funct_Match is
   Port ( clk_i          : in  std_logic;
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
end VME_Funct_Match;
--===========================================================================
-- Architecture declaration
--===========================================================================
architecture Behavioral of VME_Funct_Match is
   signal s_FUNC_ADER, s_FUNC_ADEM      : t_FUNC_32b_array;
   signal s_FUNC_ADER_64, s_FUNC_ADEM_64: t_FUNC_64b_array;
   signal s_isprev_func64               : std_logic_vector(7 downto 0);
   signal s_locAddr                     : unsigned(63 downto 0);
--===========================================================================
-- Architecture begin
--===========================================================================
begin
   s_locAddr <= unsigned(Addr);

   p_functMatch : process(clk_i)
   begin
      if rising_edge(clk_i) then
         if mainFSMreset = '1' or reset = '1' then
            FunctMatch <= (others => '0');
            Nx_Base_Addr <= (others => '0');
         elsif decode = '1' then	 
            for i in FunctMatch'range loop

               case AddrWidth is
                  when "11" => 
                     if (s_FUNC_ADEM(i)(0) = '1')  and (s_isprev_func64(i) = '0') and 
                        (s_FUNC_ADEM_64(i)(63 downto 10) /= 0) then  

                        if (s_FUNC_ADER_64(i)(63 downto 10) and s_FUNC_ADEM_64(i)(63 downto 10)) = 
                           ((s_locAddr(63 downto 10)) and s_FUNC_ADEM_64(i)(63 downto 10)) then            
                           FunctMatch(i)              <= '1';
                           Nx_Base_Addr(63 downto 10) <= std_logic_vector(s_FUNC_ADER_64(i)(63 downto 10));
                           Nx_Base_Addr(9 downto 0)   <= (others => '0');
                        end if;
                     end if;

                  when "10" =>
                     if (s_FUNC_ADEM(i)(31 downto 8) /=0)  and (s_isprev_func64(i) = '0') then

                        if (s_FUNC_ADER(i)(31 downto 8) and s_FUNC_ADEM(i)(31 downto 8)) = 
								   ((s_locAddr(31 downto 8)) and s_FUNC_ADEM(i)(31 downto 8)) then
                           FunctMatch(i)              <= '1';
                           Nx_Base_Addr(31 downto 8)  <= std_logic_vector(s_FUNC_ADER(i)(31 downto 8));
                           Nx_Base_Addr(63 downto 32) <= (others => '0');
                           Nx_Base_Addr(7 downto 0)   <= (others => '0');                
                        end if;
                     end if;

                  when "01" =>
                     if (s_FUNC_ADEM(i)(23 downto 8) /=0)  and (s_isprev_func64(i) = '0') then
                        if (s_FUNC_ADER(i)(23 downto 8) and s_FUNC_ADEM(i)(23 downto 8)) = 
								   ((s_locAddr(23 downto 8)) and s_FUNC_ADEM(i)(23 downto 8)) then
                           FunctMatch(i)              <= '1';
                           Nx_Base_Addr(23 downto 8)  <= std_logic_vector(s_FUNC_ADER(i)(23 downto 8));
                           Nx_Base_Addr(63 downto 24) <= (others => '0');
                           Nx_Base_Addr(7 downto 0)   <= (others => '0');                   
                        end if;
                     end if;

                  when "00" =>
                     if (s_FUNC_ADEM(i)(15 downto 8) /=0)  and (s_isprev_func64(i) = '0') then
                        if (s_FUNC_ADER(i)(15 downto 8) and s_FUNC_ADEM(i)(15 downto 8)) = 
								   ((s_locAddr(15 downto 8)) and s_FUNC_ADEM(i)(15 downto 8)) then
                           FunctMatch(i)              <= '1';
                           Nx_Base_Addr(15 downto 8)  <= std_logic_vector(s_FUNC_ADER(i)(15 downto 8));
                           Nx_Base_Addr(63 downto 16) <= (others => '0');
                           Nx_Base_Addr(7 downto 0)   <= (others => '0');                  
                        end if;
                     end if;

                  when others =>
               end case;
            end loop;

         end if;
      end if;	  
   end process;

------------------------------------------------------
   s_FUNC_ADER(0) <= unsigned(Ader0);
   s_FUNC_ADER(1) <= unsigned(Ader1);
   s_FUNC_ADER(2) <= unsigned(Ader2);
   s_FUNC_ADER(3) <= unsigned(Ader3);
   s_FUNC_ADER(4) <= unsigned(Ader4);
   s_FUNC_ADER(5) <= unsigned(Ader5);
   s_FUNC_ADER(6) <= unsigned(Ader6);
   s_FUNC_ADER(7) <= unsigned(Ader7);
   s_FUNC_ADEM(0) <= unsigned(Adem0);
   s_FUNC_ADEM(1) <= unsigned(Adem1);
   s_FUNC_ADEM(2) <= unsigned(Adem2);
   s_FUNC_ADEM(3) <= unsigned(Adem3);
   s_FUNC_ADEM(4) <= unsigned(Adem4);
   s_FUNC_ADEM(5) <= unsigned(Adem5);
   s_FUNC_ADEM(6) <= unsigned(Adem6);
   s_FUNC_ADEM(7) <= unsigned(Adem7);

   GDFS : for i in 0 to 7 generate
      DFS_o(i) <= s_FUNC_ADEM(i)(DFS);
   end generate GDFS;

   GADER_64 : for i in 0 to 6 generate
      s_FUNC_ADER_64(i) <= s_FUNC_ADER(i+1)&s_FUNC_ADER(i);
   end generate GADER_64;

   s_FUNC_ADER_64(7) <= (others => '0');

   GADEM_64 : for i in 0 to 6 generate
      s_FUNC_ADEM_64(i) <= s_FUNC_ADEM(i+1)&s_FUNC_ADEM(i);
      s_isprev_func64(i+1) <= s_FUNC_ADEM(i)(0);    
   end generate GADEM_64;

   s_isprev_func64(0) <= '0';
   s_FUNC_ADEM_64(7) <= (others => '0');
end Behavioral;
--===========================================================================
-- Architecture end
--===========================================================================
