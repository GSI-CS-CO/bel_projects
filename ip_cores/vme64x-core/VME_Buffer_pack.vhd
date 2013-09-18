--_______________________________________________________________________________________
--                             VME TO WB INTERFACE
--
--                                CERN,BE/CO-HT 
--______________________________________________________________________________________
-- File:                          VME_BUFFER_pack.vhd
--______________________________________________________________________________________
-- Description: Buffer functionality 
--______________________________________________________________________________________
-- Authors:                                       
--              Cesar Prados <c.prados@gsi.de>
-- Date         02/2013           
-- Version      v0.01 
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

package VME_Buffer_pack is

   function buffer_function ( fsm          : t_mainFSMstates;
                              is_d64       : std_logic;
                              vme_write    : std_logic)
                            return t_VME_BUFFER;
                            
   type bus_mode is (  LATCHED,
                       CLOCKED);
							  
   component VME_Buffer_ctrl  is
		generic(
			g_bus_mode        : bus_mode  := LATCHED); -- CLOCK 
		port( 
         clk_i            : in  std_logic;
         rst_i            : in  std_logic;
         buffer_stat_i    : in  t_VME_BUFFER;
         buffer_clk_o     : out std_logic;

         data_buff_v2f_o  : out std_logic;
         data_buff_f2v_o  : out std_logic;

         addr_buff_v2f_o  : out std_logic;
         addr_buff_f2v_o  : out std_logic;

         latch_buff_o     : out std_logic
      );
   end component VME_Buffer_ctrl;
	
	constant VME2FPGA		: std_logic :=	'0';
	constant FPGA2VME		: std_logic :=	'1';
	constant WRITE_C		: std_logic :=	'0';
	constant READ_C		: std_logic := '1'; 

end VME_Buffer_pack;

package body VME_Buffer_pack is

-- Buffer direction
-- VME to FPGA -> _v2f_ in schematics ab, negative logic e.g READ Cycle  in Data 
-- FPGA to VME -> _f2v_ in schematics ba, negative logic e.g WRITE Cycle in Data

   function buffer_function ( fsm          : t_mainFSMstates;
                              is_d64       : std_logic;
                              vme_write    : std_logic)
                            return t_VME_BUFFER is

        variable vme_buff       : t_VME_BUFFER  := c_buffer_default;

    begin

        case fsm is

            when IDLE => 

               vme_buff.s_addrDir	:= VME2FPGA;
					vme_buff.s_dataDir 	:= VME2FPGA;
               vme_buff.s_buffer_eo := ADDR_BUFF;
               vme_buff.s_clk 		:= '1';

            when DECODE_ACCESS  =>
 
               vme_buff.s_addrDir	:= VME2FPGA;
					vme_buff.s_dataDir 	:= VME2FPGA;
               vme_buff.s_buffer_eo := ADDR_BUFF;
					vme_buff.s_clk       := '0';                   
				
				when	WAIT_FOR_DS | LATCH_DS1 | LATCH_DS2
				     | LATCH_DS3 | LATCH_DS4 =>
					
					if('1' = is_d64) then
						vme_buff.s_buffer_eo    := DATA_ADDR_BUFF;
					else
						vme_buff.s_buffer_eo    := DATA_BUFF;
					end if;
					
					if	vme_write = WRITE_C then 		  
						vme_buff.s_addrDir 	:= VME2FPGA;
						vme_buff.s_dataDir 	:= VME2FPGA;
					else -- READ_C
						vme_buff.s_addrDir 	:= FPGA2VME;
						vme_buff.s_dataDir 	:= FPGA2VME;  
					end if;
						
					vme_buff.s_clk          := '0';
					
				when CHECK_TRANSFER_TYPE | MEMORY_REQ =>
						
					if('1' = is_d64) then
						vme_buff.s_buffer_eo    := DATA_ADDR_BUFF;
					else
						vme_buff.s_buffer_eo    := DATA_BUFF;
					end if;
					
					if	vme_write = WRITE_C then 		  
						vme_buff.s_addrDir 	:= VME2FPGA;
						vme_buff.s_dataDir 	:= VME2FPGA;
					else -- READ_C
						vme_buff.s_addrDir 	:= FPGA2VME;
						vme_buff.s_dataDir 	:= FPGA2VME;  
					end if;
						
					vme_buff.s_clk          := '0';	

            when DATA_TO_BUS | DTACK_LOW | DECIDE_NEXT_CYCLE =>

					if('1' = is_d64) then
						vme_buff.s_buffer_eo    := DATA_ADDR_BUFF;
					else
						vme_buff.s_buffer_eo    := DATA_BUFF;
					end if;
					
					if	vme_write = WRITE_C then 		  
						vme_buff.s_addrDir 	:= VME2FPGA;
						vme_buff.s_dataDir 	:= VME2FPGA;
					else -- READ_C
						vme_buff.s_addrDir 	:= FPGA2VME;
						vme_buff.s_dataDir 	:= FPGA2VME;  
					end if;
						
					vme_buff.s_clk          := '1';

				when INCREMENT_ADDR | SET_DATA_PHASE  =>
				
					if('1' = is_d64) then
						vme_buff.s_buffer_eo    := DATA_ADDR_BUFF;
					else
						vme_buff.s_buffer_eo    := DATA_BUFF;
					end if;
					
					if	vme_write = WRITE_C then 		  
						vme_buff.s_addrDir 	:= VME2FPGA;
						vme_buff.s_dataDir 	:= VME2FPGA;
					else -- READ_C
						vme_buff.s_addrDir 	:= FPGA2VME;
						vme_buff.s_dataDir 	:= FPGA2VME;  
					end if;
						
					vme_buff.s_clk          := '0';
					
				when others =>		
					vme_buff.s_addrDir 	:= VME2FPGA;
					vme_buff.s_dataDir 	:= VME2FPGA;
               vme_buff.s_buffer_eo := ADDR_BUFF;
					vme_buff.s_clk       := '0'; 
					
			end case;

      return vme_buff;

   end buffer_function;

end VME_Buffer_pack;

