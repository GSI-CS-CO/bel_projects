--__________________________________________________________________________________                  
--                             VME TO WB INTERFACE
--
--                                CERN,BE/CO-HT 
--_________________________________________________________________________________
-- File:                      VME_Access_Decode.vhd
--_________________________________________________________________________________
-- Description: This component checks if the board is addressed and if it is, allows 
-- the access to CR/CSR space by asserting the Confaccess signal, or allows the access 
-- to WB bus by asserting the CardSel signal.
--
-- The access to CR/CSR space is possible if:
--   1) Addr[23:19] = BAR[7:3],  (BAR[7:3] = not VME_GA_i), (VME_GA_i = not Slot number)
--   2) AM = 0x2f
--   3) The initialization is finished (wait about 8800 ns after power-up or software reset)
--
-- To Access the Wb bus we have 7 functions; only one at time can be selected. If one of 
-- these functions is selected the CardSel signal is asserted (this is the responding Slave).
-- To access the Wb bus we need to decode the AM and the address lines; so as shown in 
-- the block diagram the main components are two: VME_Funct_Match, VME_Am_Match.

--                                ___________________________________________
--                               |            VME_Access_Decode.vhd          |
--                               |                                           |
--                               |    ____________           ____________    |
--                               |   |            |         |            |   |
--                               |   |  FUNCTION  |         |     AM     |   |
--                               |   |            |         |            |   |
--                               |   |   MATCH    |         |    MATCH   |   |
--                               |   |            |         |            |   |
--                               |   |            |         |            |   |
--                               |   |            |         |            |   |
--                               |   |            |         |            |   |
--                               |   |            |         |            |   |
--                               |   |____________|         |____________|   |
--                               |                                           |
--                               |___________________________________________|

-- Each function has one ADER, one ADEM, one AMCAP and one XAMCAP register.   
-- The ADEM, AMCAP, XAMCAP are in the CR memory; the Master can't write these registers 
-- The ADER registers are located in the CSR space so the VME master has to write 
-- these registers properly after the initialization.
-- How to access:
--          ADER[31:0]        
--         [31:8]  --> compare bits (put here the base address) 
--         [7:2]   --> AM
--         [1]     --> '0'
--         [0]     --> XAM bit: '0'; '1' only for 2e access mode
--         If XAM is '1' it will be:
--         [31:10] --> compare bits (put here the base address) 
--         [9:2]   --> XAM
--         [1]     --> '0'
--         [0]     --> '1'
--          ADEM[31:0]
--         [31:8]  --> mask bits
--         [7:4]   --> "0000"
--         [3]     --> '0' --> The ADER is programmable   
--         [2]     --> DFS
--         [1]     --> '0'
--         [0]     --> EFM : '0'
--         EFM = Extra Function Mask: if '1' the next ADEM (and so the next AMCAP, XAMCAP 
--               and ADER) provides the upper bit's mask for a 64 bit decoder.
--               This bit is '1' during A64 and 2e access.
--         DFS = Dynamic Function Decoder: a '1' here means this function can be used 
--               to decode different address length (eg. A16 or A24 or A32) so the mask bits 
--               should be all '1'.
--
--          AMCAP[63:0]
--         6 AM lines --> 2**6 = 64 different configurations
--         This register is 64 bits wide and each bit rappresents one AM configuration. 
--         If the bit is '1' it means that the corresponding AM is supported by this function.
--         If the corresponding ADEM's DFS is 0, only the AMCAP's bits with the same address 
--         width must be '1'.
--         If the corresponding ADEM's DFS is 1, one or more AMCAP's bits can be '1'  
--         eg: "1011101100000000001000100000000100000000000000001011101100000000" this 
--            function supports the following access mode:
--            A24_S, A24_BLT, A24_MBLT, A16_S, A32_S, A32_BLT, A32_MBLT supervisor and user 
--         access
--
--          XAMCAP[255:0]
--         8 XAM lines --> 2**8 = 256 different configurations
--         This register is 256 bits wide and each bit rappresents one XAM configuration. 
--         If the bit is '1' it means that the corresponding XAM is supported
--         by this function.
--         This register is used during the decode phase if the XAM bit is asserted (1).
-- Before accessing the board the VME Master must write the ADER registers. Of course for 
-- writing properly the ADER the VME Master needs to know the corresponding ADEM and check if 
-- EFM or DFS bits are asserted. The VME Master can read also 
-- the AMCAP and XAMCAP and check the access mode supported by each function.
-- 
-- eg.1 let's imagine that we want to access different storage device; we can assign 
-- one base address and one function at each storage.
-- Now the VME Master has to write the base address of each storage in the corresponding 
-- ADER's compare bits and after this operation each function decodes the access to 
-- the corresponding storage.
-- eg.2 this example is relative to our application; the vme64x interface has to transfer 
-- data from the VMEbus to WB bus and in this core we have only one WB master. We 
-- can use the same base address for all the functions because we will access always 
-- the same WB master, and use the different functions to access with different mode eg:
--       function0 --> A32_S, A32_BLT, A32_MBLT modes
--       function1 --> A24_S, A24_BLT, A24_MBLT modes
--       function2 --> A16 mode
--       function3 and function4 --> A64, A64_BLT, A64_MBLT
--       function5 and function6 --> 2eVME and 2eSST modes
-- Note that if the address is 64 bits wide we need of two ADER and two ADEM to decode the 
-- address so we need two functions. (see also EFM bit definition)
-- Of course you can mix these two example and set up one system with more storage devices 
-- each with its base address and to assign each storage more than one function to access it 
-- with all the access modes.
-- It is also possible extend the number of the functions defining other ADEM, AMCAP, XAMCAP 
-- and ADER in the User CR Space and User CSR Space (see the VME_CR_CSR_Space.vhd component) 
-- respectively.
-- In the VME_Funct_Match.vhd and VME_Am_Match.vhd components you can find more details 
-- about the decode process.
--
-- To access the board both the FunctMatch(i) and AmMatch(i) must be equal to one.
--________________________________________________________________________________________
-- Authors:                                     
--               Pablo Alvarez Sanchez (Pablo.Alvarez.Sanchez@cern.ch)                             
--               Davide Pedretti       (Davide.Pedretti@cern.ch)  
-- Date         11/2012                                                                           
-- Version      v0.03 
--________________________________________________________________________________________
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
-----------------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

use IEEE.NUMERIC_STD.ALL;
use work.xvme64x_pack.all;

--===========================================================================
-- Entity declaration
--===========================================================================

entity VME_Access_Decode is
    Port (clk_i           : in  STD_LOGIC;
          reset           : in  STD_LOGIC;
          mainFSMreset    : in  STD_LOGIC;
          decode          : in  STD_LOGIC;
          ModuleEnable    : in  STD_LOGIC;
          InitInProgress  : in  STD_LOGIC;
          Addr            : in  STD_LOGIC_VECTOR (63 downto 0);
          Ader0           : in  STD_LOGIC_VECTOR (31 downto 0);
          Ader1           : in  STD_LOGIC_VECTOR (31 downto 0);
          Ader2           : in  STD_LOGIC_VECTOR (31 downto 0);
          Ader3           : in  STD_LOGIC_VECTOR (31 downto 0);
          Ader4           : in  STD_LOGIC_VECTOR (31 downto 0);
          Ader5           : in  STD_LOGIC_VECTOR (31 downto 0);
          Ader6           : in  STD_LOGIC_VECTOR (31 downto 0);
          Ader7           : in  STD_LOGIC_VECTOR (31 downto 0);
          Adem0           : in  STD_LOGIC_VECTOR (31 downto 0);
          Adem1           : in  STD_LOGIC_VECTOR (31 downto 0);
          Adem2           : in  STD_LOGIC_VECTOR (31 downto 0);
          Adem3           : in  STD_LOGIC_VECTOR (31 downto 0);
          Adem4           : in  STD_LOGIC_VECTOR (31 downto 0);
          Adem5           : in  STD_LOGIC_VECTOR (31 downto 0);
          Adem6           : in  STD_LOGIC_VECTOR (31 downto 0);
          Adem7           : in  STD_LOGIC_VECTOR (31 downto 0);
          AmCap0          : in  STD_LOGIC_VECTOR (63 downto 0);
          AmCap1          : in  STD_LOGIC_VECTOR (63 downto 0);
          AmCap2          : in  STD_LOGIC_VECTOR (63 downto 0);
          AmCap3          : in  STD_LOGIC_VECTOR (63 downto 0);
          AmCap4          : in  STD_LOGIC_VECTOR (63 downto 0);
          AmCap5          : in  STD_LOGIC_VECTOR (63 downto 0);
          AmCap6          : in  STD_LOGIC_VECTOR (63 downto 0);
          AmCap7          : in  STD_LOGIC_VECTOR (63 downto 0);
          XAmCap0         : in  STD_LOGIC_VECTOR (255 downto 0);
          XAmCap1         : in  STD_LOGIC_VECTOR (255 downto 0);
          XAmCap2         : in  STD_LOGIC_VECTOR (255 downto 0);
          XAmCap3         : in  STD_LOGIC_VECTOR (255 downto 0);
          XAmCap4         : in  STD_LOGIC_VECTOR (255 downto 0);
          XAmCap5         : in  STD_LOGIC_VECTOR (255 downto 0);
          XAmCap6         : in  STD_LOGIC_VECTOR (255 downto 0);
          XAmCap7         : in  STD_LOGIC_VECTOR (255 downto 0);
          Am              : in  STD_LOGIC_VECTOR (5 downto 0);
          XAm             : in  STD_LOGIC_VECTOR (7 downto 0);
          BAR_i           : in  STD_LOGIC_VECTOR (4 downto 0);
          AddrWidth       : in  STD_LOGIC_VECTOR (1 downto 0);
          Funct_Sel       : out  STD_LOGIC_VECTOR (7 downto 0);
          Base_Addr       : out  STD_LOGIC_VECTOR (63 downto 0);
          Confaccess      : out  std_logic;
          CardSel         : out  std_logic
       );

end VME_Access_Decode;
--===========================================================================
-- Architecture declaration
--===========================================================================
architecture Behavioral of VME_Access_Decode is
   signal s_Func_Match    : std_logic_vector(7 downto 0);
   signal s_Am_Match      : std_logic_vector(7 downto 0);
   signal s_nx_base_addr  : std_logic_vector(63 downto 0);
   signal s_func_sel      : std_logic_vector(7 downto 0);
   signal s_DFS           : std_logic_vector(7 downto 0);
--===========================================================================
-- Architecture begin
--===========================================================================	
begin

   Funct_Sel <= s_func_sel;

   Inst_Funct_Match: VME_Funct_Match port map(
                                                clk_i          => clk_i,
                                                reset          => reset,
                                                decode         => decode,
                                                mainFSMreset   => mainFSMreset,
                                                Addr           => Addr,
                                                AddrWidth      => AddrWidth,
                                                Ader0          => Ader0,
                                                Ader1          => Ader1,
                                                Ader2          => Ader2,
                                                Ader3          => Ader3,
                                                Ader4          => Ader4,
                                                Ader5          => Ader5,
                                                Ader6          => Ader6,
                                                Ader7          => Ader7,
                                                Adem0          => Adem0,
                                                Adem1          => Adem1,
                                                Adem2          => Adem2,
                                                Adem3          => Adem3,
                                                Adem4          => Adem4,
                                                Adem5          => Adem5,
                                                Adem6          => Adem6,
                                                Adem7          => Adem7,
                                                FunctMatch     => s_Func_Match,
                                                DFS_o          => s_DFS,
                                                Nx_Base_Addr   => s_nx_base_addr
                                             );

   Inst_Am_Match: VME_Am_Match port map(
                                                clk_i          => clk_i,
                                                reset          => reset,
                                                mainFSMreset   => mainFSMreset,
                                                Ader0          => Ader0,
                                                Ader1          => Ader1,
                                                Ader2          => Ader2,
                                                Ader3          => Ader3,
                                                Ader4          => Ader4,
                                                Ader5          => Ader5,
                                                Ader6          => Ader6,
                                                Ader7          => Ader7,
                                                AmCap0         => AmCap0,
                                                AmCap1         => AmCap1,
                                                AmCap2         => AmCap2,
                                                AmCap3         => AmCap3,
                                                AmCap4         => AmCap4,
                                                AmCap5         => AmCap5,
                                                AmCap6         => AmCap6,
                                                AmCap7         => AmCap7,
                                                XAmCap0        => XAmCap0,
                                                XAmCap1        => XAmCap1,
                                                XAmCap2        => XAmCap2,
                                                XAmCap3        => XAmCap3,
                                                XAmCap4        => XAmCap4,
                                                XAmCap5        => XAmCap5,
                                                XAmCap6        => XAmCap6,
                                                XAmCap7        => XAmCap7,
                                                Am             => Am,
                                                XAm            => XAm,
                                                DFS_i          => s_DFS,
                                                decode         => decode,
                                                AmMatch        => s_Am_Match
                                       );

-- Check if the WB application is addressed
   process(clk_i)
   begin
      if rising_edge(clk_i) then
         CardSel <= '0';
         Base_Addr <= (others => '0');
         if ModuleEnable = '1' and InitInProgress = '0' then
            for I in 0 to 7 loop
               if s_func_sel(i) = '1' then
                  CardSel <= '1';
                  Base_Addr <= s_nx_base_addr;
                  -- exit; in this case the exit statement is useless
               end if;
            end loop;
         end if;
      end if;	 
   end process;

   s_func_sel <=  s_Func_Match and s_Am_Match; 
	
-- Check if the CR/CSR space is addressed
   Confaccess <= '1' when unsigned(BAR_i) = unsigned(Addr(23 downto 19)) and 
                  Am = c_CR_CSR and InitInProgress = '0' else '0';  


end Behavioral;
--===========================================================================
-- Architecture end
--===========================================================================
