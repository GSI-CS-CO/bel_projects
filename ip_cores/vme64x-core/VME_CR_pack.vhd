--_______________________________________________________________________________________
--                             VME TO WB INTERFACE
--
--                                CERN,BE/CO-HT 
--______________________________________________________________________________________
-- File:                          VME_CR_pack.vhd
--______________________________________________________________________________________
-- Description: ROM memory (CR space)
--______________________________________________________________________________________
-- Authors:                                       
--               Pablo Alvarez Sanchez (Pablo.Alvarez.Sanchez@cern.ch)                             
--               Davide Pedretti       (Davide.Pedretti@cern.ch)  
-- Date         06/2012                                                                           
-- Version      v0.02  
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
---------------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.numeric_std.all;

use work.xvme64x_pack.all;
package VME_CR_pack is
 --  type t_cr_array is array (natural range <>) of std_logic_vector(7 downto 0);
   constant c_amcap : std_logic_vector(63 downto 0) :=
   "1111111100000000001100100000000000000000000100001111111100001011";
   constant c_amcap0 : std_logic_vector(63 downto 0) :=
   "0000000000000000000000000000000000000000000000001011101100000000";   	 --A32

  --	"1011101100000000001000100000000100000000000000001011101100000000";
   constant c_amcapMBLT : std_logic_vector(63 downto 0) :=
   "0000000000000000000000000000000000000000000000000000000100000000";	
   constant c_amcap1 : std_logic_vector(63 downto 0) :=
   "1011101100000000000000000000000000000000000000000000000000000000";	--A24
   constant c_amcap2 : std_logic_vector(63 downto 0) :=
   "0000000000000000001000100000000000000000000000000000000000000000";	--A16
   constant c_amcapA64 : std_logic_vector(63 downto 0) :=
   "0000000000000000000000000000000000000000000000000000000000001011"; --for modalities A64, A64_BLT, A64_MBLT
   constant c_amcap2e : std_logic_vector(63 downto 0) :=
   "0000000000000000000000000000000100000000000000000000000000000000"; -- for modalities TWO_edge 

   constant c_xamcap0 : std_logic_vector(255 downto 0) :=
   (others => '0');   	 


   constant c_xamcap2 : std_logic_vector(255 downto 0) :=
   x"0000000000000000000000000000000000000000000000000000000000060006";   

   constant c_amb : t_cr_array(0 to 7) :=(
   c_amcap(7 downto 0),   c_amcap(15 downto 8),
   c_amcap(23 downto 16), c_amcap(31 downto 24),
   c_amcap(39 downto 32), c_amcap(47 downto 40),
   c_amcap(55 downto 48), c_amcap(63 downto 56));    

   constant c_amb0 : t_cr_array(0 to 7) :=(                
   c_amcap0(7 downto 0),   c_amcap0(15 downto 8),
   c_amcap0(23 downto 16), c_amcap0(31 downto 24),
   c_amcap0(39 downto 32), c_amcap0(47 downto 40),
   c_amcap0(55 downto 48), c_amcap0(63 downto 56));	

   constant c_amb1 : t_cr_array(0 to 7) :=(                
   c_amcap1(7 downto 0),   c_amcap1(15 downto 8),
   c_amcap1(23 downto 16), c_amcap1(31 downto 24),
   c_amcap1(39 downto 32), c_amcap1(47 downto 40),
   c_amcap1(55 downto 48), c_amcap1(63 downto 56));			

   constant c_amb2 : t_cr_array(0 to 7) :=(              
   c_amcap2(7 downto 0),   c_amcap2(15 downto 8),
   c_amcap2(23 downto 16), c_amcap2(31 downto 24),
   c_amcap2(39 downto 32), c_amcap2(47 downto 40),
   c_amcap2(55 downto 48), c_amcap2(63 downto 56));			

   constant c_amb2e : t_cr_array(0 to 7) :=(              
   c_amcap2e(7 downto 0),   c_amcap2e(15 downto 8),
   c_amcap2e(23 downto 16), c_amcap2e(31 downto 24),
   c_amcap2e(39 downto 32), c_amcap2e(47 downto 40),
   c_amcap2e(55 downto 48), c_amcap2e(63 downto 56));	
   constant c_amb64 : t_cr_array(0 to 7) :=(              
   c_amcapA64(7 downto 0),   c_amcapA64(15 downto 8),
   c_amcapA64(23 downto 16), c_amcapA64(31 downto 24),
  c_amcapA64(39 downto 32),  c_amcapA64(47 downto 40),
   c_amcapA64(55 downto 48), c_amcapA64(63 downto 56));	

   constant c_xam0 : t_cr_array(0 to 31) :=(               
   c_xamcap0(7 downto 0),     c_xamcap0(15 downto 8),    c_xamcap0(23 downto 16), 
	c_xamcap0(31 downto 24),   c_xamcap0(39 downto 32),   c_xamcap0(47 downto 40), 
	c_xamcap0(55 downto 48),   c_xamcap0(63 downto 56),   c_xamcap0(71 downto 64), 
	c_xamcap0(79 downto 72),   c_xamcap0(87 downto 80),   c_xamcap0(95 downto 88), 
	c_xamcap0(103 downto 96),  c_xamcap0(111 downto 104), c_xamcap0(119 downto 112), 
	c_xamcap0(127 downto 120), c_xamcap0(135 downto 128), c_xamcap0(143 downto 136), 
	c_xamcap0(151 downto 144), c_xamcap0(159 downto 152), c_xamcap0(167 downto 160), 
	c_xamcap0(175 downto 168), c_xamcap0(183 downto 176), c_xamcap0(191 downto 184), 
	c_xamcap0(199 downto 192), c_xamcap0(207 downto 200), c_xamcap0(215 downto 208), 
	c_xamcap0(223 downto 216), c_xamcap0(231 downto 224), c_xamcap0(239 downto 232), 
	c_xamcap0(247 downto 240), c_xamcap0(255 downto 248));


   constant c_xam2 : t_cr_array(0 to 31) :=(             
   c_xamcap2(7 downto 0),     c_xamcap2(15 downto 8),    c_xamcap2(23 downto 16), 
	c_xamcap2(31 downto 24),   c_xamcap2(39 downto 32),   c_xamcap2(47 downto 40), 
	c_xamcap2(55 downto 48),   c_xamcap2(63 downto 56),   c_xamcap2(71 downto 64), 
	c_xamcap2(79 downto 72),   c_xamcap2(87 downto 80),   c_xamcap2(95 downto 88),
   c_xamcap2(103 downto 96),  c_xamcap2(111 downto 104), c_xamcap2(119 downto 112),
   c_xamcap2(127 downto 120), c_xamcap2(135 downto 128), c_xamcap2(143 downto 136), 
   c_xamcap2(151 downto 144), c_xamcap2(159 downto 152), c_xamcap2(167 downto 160), 
   c_xamcap2(175 downto 168), c_xamcap2(183 downto 176), c_xamcap2(191 downto 184),
   c_xamcap2(199 downto 192), c_xamcap2(207 downto 200), c_xamcap2(215 downto 208), 
   c_xamcap2(223 downto 216), c_xamcap2(231 downto 224), c_xamcap2(239 downto 232), 
   c_xamcap2(247 downto 240), c_xamcap2(255 downto 248));
   
   constant c_cr_array : 	t_cr_array(2**12 downto 0) :=
   (
   16#00#  => (others => '0'),
      -- Length of ROM
   16#01#  => x"00",
   16#02#  => x"10",
   16#03#  => x"00",
      --Configuration ROM data acces width
   16#04#  => x"84",  --D32, D16, D08
      --CSR data acces width
   16#05#  => x"84",  --D32, D16, D08
      --CR/CSR Space Specification ID
   16#06#  => x"01", 		
      --Ascii "C"
   16#07#  => x"43", 
      --Ascii "R"
   16#08#  => x"52",
      --Manufacturer's ID   -- for CERN: 0x080030
   16#09#  => x"08",
   16#0A#  => x"00",
   16#0B#  => x"30",
      --board id            -- eg: SVEC ID = 0x000198
   16#0C#  => x"03",
   16#0D#  => x"04",
   16#0E#  => x"04",
   16#0F#  => x"03",
      --Rev id
   16#10#  => x"00",
   16#11#  => x"00",
   16#12#  => x"00",
   16#13#  => x"02",
  --Point to ascii null terminatied
   16#14#  => x"00",
   16#15#  => x"00",
   16#16#  => x"00",  
  --Program Id code
   16#1F#  => x"5a",
   --Offset to BEG_USER_CR    
   16#20#  => x"00",
   16#21#  => x"00",
   16#22#  => x"00",
   --Offset to END_USER_CR   
   16#23#  => x"00",
   16#24#  => x"00",
   16#25#  => x"00",
      --Offset to BEG_CRAM    
   16#26#  => x"00",
   16#27#  => x"10",    
   16#28#  => x"00",    
      --Offset to END_CRAM       
   16#29#  => x"00",   
   16#2A#  => x"13",   
   16#2B#  => x"ff",   
      --Offset to BEG_USER_CSR  
   16#2C#  => x"00",
   16#2D#  => x"00",
   16#2E#  => x"00",    -- 0x7fbf0 and NOT 0x7fbf3 because is possible access with D32 mode
      --Offset to END_USER_CSR   
   16#2F#  => x"00",
   16#30#  => x"00",
   16#31#  => x"00",
      --CRAM_ACCESS_WIDTH
   16#3f#  => x"84", --D32, D16, D08
      --Function data access width
   16#40#  => x"86", -- Fun 0 accepts D64, D32, D16, D08(EO) cycles
   16#41#  => x"86", -- Fun 1 
   16#42#  => x"86", -- Fun 2 
   16#43#  => x"86", -- Fun 3

   16#44#  => x"86", -- Fun 4
   16#45#  => x"86", -- Fun 5
   16#46#  => x"86", -- Fun 6
   16#47#  => x"86", -- Fun 7


      --Function AM code Mask
   16#48#  => c_amb0(7), -- Fun 0    for A32 S, A32 BLT, A32 MBLT
   16#49#  => c_amb0(6), -- Fun 0 
   16#4A#  => c_amb0(5), -- Fun 0 
   16#4B#  => c_amb0(4), -- Fun 0  
   16#4C#  => c_amb0(3), -- Fun 0 
   16#4D#  => c_amb0(2), -- Fun 0 
   16#4E#  => c_amb0(1), -- Fun 0  
   16#4F#  => c_amb0(0), -- Fun 0

   16#50#  => c_amb1(7), -- Fun 1    for A24 S, A24 BLT, A24 MBLT
   16#51#  => c_amb1(6), -- Fun 1 
   16#52#  => c_amb1(5), -- Fun 1 
   16#53#  => c_amb1(4), -- Fun 1 
   16#54#  => c_amb1(3), -- Fun 1 
   16#55#  => c_amb1(2), -- Fun 1 
   16#56#  => c_amb1(1), -- Fun 1  
   16#57#  => c_amb1(0), -- Fun 1


   16#58#  => c_amb2(7), -- Fun 2    for A16 
   16#59#  => c_amb2(6), -- Fun 2  
   16#5A#  => c_amb2(5), -- Fun 2
   16#5B#  => c_amb2(4), -- Fun 2 
   16#5C#  => c_amb2(3), -- Fun 2 
   16#5D#  => c_amb2(2), -- Fun 2 
   16#5E#  => c_amb2(1), -- Fun 2 
   16#5F#  => c_amb2(0), -- Fun 2


   16#60#  => c_amb64(7), -- Fun 3   -- for A64 S, A64 BLT, A64 MBLT
   16#61#  => c_amb64(6), -- Fun 3 
   16#62#  => c_amb64(5), -- Fun 3 
   16#63#  => c_amb64(4), -- Fun 3 
   16#64#  => c_amb64(3), -- Fun 3 
   16#65#  => c_amb64(2), -- Fun 3 
   16#66#  => c_amb64(1), -- Fun 3 
   16#67#  => c_amb64(0), -- Fun 3


   16#68#  => x"00", -- Fun 3_b   -- These are not used because the FUNC 3 decode 
   16#69#  => x"00", -- Fun 3_b   -- the access mode: A64 --> 2 ADER, 2 ADEM
   16#6A#  => x"00", -- Fun 3_b 
   16#6B#  => x"00", -- Fun 3_b 
   16#6C#  => x"00", -- Fun 3_b 
   16#6D#  => x"00", -- Fun 3_b 
   16#6E#  => x"00", -- Fun 3_b 
   16#6F#  => x"00", -- Fun 3_b

   16#70#  => c_amb2e(7), -- Fun 4 
   16#71#  => c_amb2e(6), -- Fun 4 
   16#72#  => c_amb2e(5), -- Fun 4 
   16#73#  => c_amb2e(4), -- Fun 4
   16#74#  => c_amb2e(3), -- Fun 4 
   16#75#  => c_amb2e(2), -- Fun 4 
   16#76#  => c_amb2e(1), -- Fun 4 
   16#77#  => c_amb2e(0), -- Fun 4

   16#78#  => x"00", -- Fun 4_b 
   16#79#  => x"00", -- Fun 4_b 
   16#7A#  => x"00", -- Fun 4_b 
   16#7B#  => x"00", -- Fun 4_b
   16#7C#  => x"00", -- Fun 4_b 
   16#7D#  => x"00", -- Fun 4_b 
   16#7E#  => x"00", -- Fun 4_b 
   16#7F#  => x"00", -- Fun 4_b


      --Xamcap
   16#88#  => c_xam0(31), -- Fun 0  XAMCAP MSB
   16#89#  => c_xam0(30),
   16#8A#  => c_xam0(29),
   16#8B#  => c_xam0(28),
   16#8C#  => c_xam0(27),
   16#8D#  => c_xam0(26),
   16#8E#  => c_xam0(25),
   16#8F#  => c_xam0(24),
   16#90#  => c_xam0(23),
   16#91#  => c_xam0(22),
   16#92#  => c_xam0(21),
   16#93#  => c_xam0(20),
   16#94#  => c_xam0(19),
   16#95#  => c_xam0(18),
   16#96#  => c_xam0(17),
   16#97#  => c_xam0(16),
   16#98#  => c_xam0(15),
   16#99#  => c_xam0(14),
   16#9A#  => c_xam0(13),
   16#9B#  => c_xam0(12),
   16#9C#  => c_xam0(11),
   16#9D#  => c_xam0(10),
   16#9E#  => c_xam0(9),
   16#9F#  => c_xam0(8),
   16#A0#  => c_xam0(7),
   16#A1#  => c_xam0(6),
   16#A2#  => c_xam0(5),
   16#A3#  => c_xam0(4),
   16#A4#  => c_xam0(3),
   16#A5#  => c_xam0(2),
   16#A6#  => c_xam0(1),
   16#A7#  => c_xam0(0),


   16#A8#  => c_xam0(31),         -- Fun 1  XAMCAP MSB
   16#A9#  => c_xam0(30),
   16#AA#  => c_xam0(29),
   16#AB#  => c_xam0(28),
   16#AC#  => c_xam0(27),
   16#AD#  => c_xam0(26),
   16#AE#  => c_xam0(25),
   16#AF#  => c_xam0(24),
   16#B0#  => c_xam0(23),
   16#B1#  => c_xam0(22),
   16#B2#  => c_xam0(21),
   16#B3#  => c_xam0(20),
   16#B4#  => c_xam0(19),
   16#B5#  => c_xam0(18),
   16#B6#  => c_xam0(17),
   16#B7#  => c_xam0(16),
   16#B8#  => c_xam0(15),
   16#B9#  => c_xam0(14),
   16#BA#  => c_xam0(13),
   16#BB#  => c_xam0(12),
   16#BC#  => c_xam0(11),
   16#BD#  => c_xam0(10),
   16#BE#  => c_xam0(9),
   16#BF#  => c_xam0(8),
   16#C0#  => c_xam0(7),
   16#C1#  => c_xam0(6),
   16#C2#  => c_xam0(5),
   16#C3#  => c_xam0(4),
   16#C4#  => c_xam0(3),
   16#C5#  => c_xam0(2),
   16#C6#  => c_xam0(1),
   16#C7#  => c_xam0(0),

   16#C8#  => c_xam0(31),         -- Fun 2  XAMCAP MSB
   16#C9#  => c_xam0(30),
   16#CA#  => c_xam0(29),
   16#CB#  => c_xam0(28),
   16#CC#  => c_xam0(27),
   16#CD#  => c_xam0(26),
   16#CE#  => c_xam0(25),
   16#CF#  => c_xam0(24),
   16#D0#  => c_xam0(23),
   16#D1#  => c_xam0(22),
   16#D2#  => c_xam0(21),
   16#D3#  => c_xam0(20),
   16#D4#  => c_xam0(19),
   16#D5#  => c_xam0(18),
   16#D6#  => c_xam0(17),
   16#D7#  => c_xam0(16),
   16#D8#  => c_xam0(15),
   16#D9#  => c_xam0(14),
   16#DA#  => c_xam0(13),
   16#DB#  => c_xam0(12),
   16#DC#  => c_xam0(11),
   16#DD#  => c_xam0(10),
   16#DE#  => c_xam0(9),
   16#DF#  => c_xam0(8),
   16#E0#  => c_xam0(7),
   16#E1#  => c_xam0(6),
   16#E2#  => c_xam0(5),
   16#E3#  => c_xam0(4),
   16#E4#  => c_xam0(3),
   16#E5#  => c_xam0(2),
   16#E6#  => c_xam0(1),
   16#E7#  => c_xam0(0),

   16#E8#  => c_xam0(31),         -- Fun 3  XAMCAP MSB
   16#E9#  => c_xam0(30),
   16#EA#  => c_xam0(29),
   16#EB#  => c_xam0(28),
   16#EC#  => c_xam0(27),
   16#ED#  => c_xam0(26),
   16#EE#  => c_xam0(25),
   16#EF#  => c_xam0(24),
   16#F0#  => c_xam0(23),
   16#F1#  => c_xam0(22),
   16#F2#  => c_xam0(21),
   16#F3#  => c_xam0(20),
   16#F4#  => c_xam0(19),
   16#F5#  => c_xam0(18),
   16#F6#  => c_xam0(17),
   16#F7#  => c_xam0(16),
   16#F8#  => c_xam0(15),
   16#F9#  => c_xam0(14),
   16#FA#  => c_xam0(13),
   16#FB#  => c_xam0(12),
   16#FC#  => c_xam0(11),
   16#FD#  => c_xam0(10),
   16#FE#  => c_xam0(9),
   16#FF#  => c_xam0(8),
   16#100#  => c_xam0(7),
   16#101#  => c_xam0(6),
   16#102#  => c_xam0(5),
   16#103#  => c_xam0(4),
   16#104#  => c_xam0(3),
   16#105#  => c_xam0(2),
   16#106#  => c_xam0(1),
   16#107#  => c_xam0(0),

   16#108#  => c_xam0(31),         -- Fun 3_b  XAMCAP MSB
   16#109#  => c_xam0(30),
   16#10A#  => c_xam0(29),
   16#10B#  => c_xam0(28),
   16#10C#  => c_xam0(27),
   16#10D#  => c_xam0(26),
   16#10E#  => c_xam0(25),
   16#10F#  => c_xam0(24),
   16#110#  => c_xam0(23),
   16#111#  => c_xam0(22),
   16#112#  => c_xam0(21),
   16#113#  => c_xam0(20),
   16#114#  => c_xam0(19),
   16#115#  => c_xam0(18),
   16#116#  => c_xam0(17),
   16#117#  => c_xam0(16),
   16#118#  => c_xam0(15),
   16#119#  => c_xam0(14),
   16#11A#  => c_xam0(13),
   16#11B#  => c_xam0(12),
   16#11C#  => c_xam0(11),
   16#11D#  => c_xam0(10),
   16#11E#  => c_xam0(9),
   16#11F#  => c_xam0(8),
   16#120#  => c_xam0(7),
   16#121#  => c_xam0(6),
   16#122#  => c_xam0(5),
   16#123#  => c_xam0(4),
   16#124#  => c_xam0(3),
   16#125#  => c_xam0(2),
   16#126#  => c_xam0(1),
   16#127#  => c_xam0(0),

   16#128#  => c_xam2(31),         -- Fun 4  XAMCAP MSB
   16#129#  => c_xam2(30),
   16#12A#  => c_xam2(29),
   16#12B#  => c_xam2(28),
   16#12C#  => c_xam2(27),
   16#12D#  => c_xam2(26),
   16#12E#  => c_xam2(25),
   16#12F#  => c_xam2(24),
   16#130#  => c_xam2(23),
   16#131#  => c_xam2(22),
   16#132#  => c_xam2(21),
   16#133#  => c_xam2(20),
   16#134#  => c_xam2(19),
   16#135#  => c_xam2(18),
   16#136#  => c_xam2(17),
   16#137#  => c_xam2(16),
   16#138#  => c_xam2(15),
   16#139#  => c_xam2(14),
   16#13A#  => c_xam2(13),
   16#13B#  => c_xam2(12),
   16#13C#  => c_xam2(11),
   16#13D#  => c_xam2(10),
   16#13E#  => c_xam2(9),
   16#13F#  => c_xam2(8),
   16#140#  => c_xam2(7),
   16#141#  => c_xam2(6),
   16#142#  => c_xam2(5),
   16#143#  => c_xam2(4),
   16#144#  => c_xam2(3),
   16#145#  => c_xam2(2),
   16#146#  => c_xam2(1),
   16#147#  => c_xam2(0),

   16#148#  => c_xam0(31),         -- Fun 4_b  XAMCAP MSB
   16#149#  => c_xam0(30),
   16#14A#  => c_xam0(29),
   16#14B#  => c_xam0(28),
   16#14C#  => c_xam0(27),
   16#14D#  => c_xam0(26),
   16#14E#  => c_xam0(25),
   16#14F#  => c_xam0(24),
   16#150#  => c_xam0(23),
   16#151#  => c_xam0(22),
   16#152#  => c_xam0(21),
   16#153#  => c_xam0(20),
   16#154#  => c_xam0(19),
   16#155#  => c_xam0(18),
   16#156#  => c_xam0(17),
   16#157#  => c_xam0(16),
   16#158#  => c_xam0(15),
   16#159#  => c_xam0(14),
   16#15A#  => c_xam0(13),
   16#15B#  => c_xam0(12),
   16#15C#  => c_xam0(11),
   16#15D#  => c_xam0(10),
   16#15E#  => c_xam0(9),
   16#15F#  => c_xam0(8),
   16#160#  => c_xam0(7),
   16#161#  => c_xam0(6),
   16#162#  => c_xam0(5),
   16#163#  => c_xam0(4),
   16#164#  => c_xam0(3),
   16#165#  => c_xam0(2),
   16#166#  => c_xam0(1),
   16#167#  => c_xam0(0),

   16#168#  => c_xam0(31),         -- Fun 5  XAMCAP MSB
   16#169#  => c_xam0(30),
   16#16A#  => c_xam0(29),
   16#16B#  => c_xam0(28),
   16#16C#  => c_xam0(27),
   16#16D#  => c_xam0(26),
   16#16E#  => c_xam0(25),
   16#16F#  => c_xam0(24),
   16#170#  => c_xam0(23),
   16#171#  => c_xam0(22),
   16#172#  => c_xam0(21),
   16#173#  => c_xam0(20),
   16#174#  => c_xam0(19),
   16#175#  => c_xam0(18),
   16#176#  => c_xam0(17),
   16#177#  => c_xam0(16),
   16#178#  => c_xam0(15),
   16#179#  => c_xam0(14),
   16#17A#  => c_xam0(13),
   16#17B#  => c_xam0(12),
   16#17C#  => c_xam0(11),
   16#17D#  => c_xam0(10),
   16#17E#  => c_xam0(9),
   16#17F#  => c_xam0(8),
   16#180#  => c_xam0(7),
   16#181#  => c_xam0(6),
   16#182#  => c_xam0(5),
   16#183#  => c_xam0(4),
   16#184#  => c_xam0(3),
   16#185#  => c_xam0(2),
   16#186#  => c_xam0(1),
   16#187#  => c_xam0(0),

      --...

      --16#C6#  => x"00", -- Fun 0  XAMCAP LSB
      --16#C7#  => x"01", -- Fun 0  XAMCAP LSB
      --......

      -- Address Decoder Mask ADEM
   16#188#  => x"f0", -- Fun 0 
   16#189#  => x"00", -- Fun 0 
   16#18A#  => x"00", -- Fun 0 
   16#18B#  => x"00", -- Fun 0 --DFS = '0'

   16#18c#  => x"00", -- Fun 1 
   16#18d#  => x"f0", -- Fun 1 
   16#18e#  => x"00", -- Fun 1 
   16#18f#  => x"00", -- Fun 1 --DFS = '0'

   16#190#  => x"00", -- Fun 2 
   16#191#  => x"00", -- Fun 2
   16#192#  => x"f0", -- Fun 2 
   16#193#  => x"00", -- Fun 2 --DFS = '0'

   16#194#  => x"00", -- Fun 3 
   16#195#  => x"00", -- Fun 3 
   16#196#  => x"00", -- Fun 3 
   16#197#  => x"01", -- Fun 3

   16#198#  => x"ff", -- Fun 4 (used for decoding FUNC3) 
   16#199#  => x"00", -- Fun 4 (used for decoding FUNC3)
   16#19a#  => x"00", -- Fun 4 (used for decoding FUNC3)
   16#19b#  => x"00", -- Fun 4 (used for decoding FUNC3)


   16#19c#  => x"ff", -- Fun 5 
   16#19d#  => x"00", -- Fun 5 
   16#19e#  => x"00", -- Fun 5 
   16#19f#  => x"01", -- Fun 5

   16#1a0#  => x"00", -- Fun 6 
   16#1a1#  => x"00", -- Fun 6 
   16#1a2#  => x"00", -- Fun 6 
   16#1a3#  => x"00", -- Fun 6


   others => (others => '0'));

end VME_CR_pack;                                                                




















