--______________________________________________________________________________|
--                             VME TO WB INTERFACE                              |
--                                                                              |
--                                CERN,BE/CO-HT                                 |
--______________________________________________________________________________|
-- File:                        VME_CRAM.vhd                                  |
--______________________________________________________________________________|
-- Description: RAM memory
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
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use work.xvme64x_pack.all;
--===========================================================================
-- Entity declaration
--===========================================================================
 entity VME_CRAM is
 generic (dl : integer; 		
 			 al : integer := f_log2_size(c_CRAM_SIZE) 		
			 );    
 port (clk  : in std_logic; 			
 	we   : in std_logic; 				
 	aw    : in std_logic_vector(al - 1 downto 0); 	 
 	di   : in std_logic_vector(dl - 1 downto 0); 
 	dw  : out std_logic_vector(dl - 1 downto 0)
	);   	 
 end VME_CRAM; 
 												
--===========================================================================
-- Architecture declaration
--===========================================================================
 architecture syn of VME_CRAM is 
 
 type ram_type is array (2**al - 1 downto 0) of std_logic_vector (dl - 1 downto 0); 
 signal CRAM : ram_type; 

--===========================================================================
-- Architecture begin
--===========================================================================
begin 

 process (clk) 
 begin 
 	if (clk'event and clk = '1') then  
 		if (we = '1') then 
 			CRAM(conv_integer(aw)) <= di; 
 		end if; 
		dw <= CRAM(conv_integer(aw));
 	end if; 
 end process; 
 
 end syn;
--===========================================================================
-- Architecture end
--===========================================================================
 
