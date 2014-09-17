-------------------------------------------------------------------------------
-- Title      : FPGA reset for Arria 
-- Project    : all Arria platforms
-------------------------------------------------------------------------------
-- File       : altera_reset.vhd
-- Author     : Stefan Rauch
-- Company    : GSI
-- Created    : 2013-12-12
-- Last update: 2014-09-16
-- Platform   : Altera
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: resets FPGA with internal logic using alt remote update 
-- n: number of user LM32 cores in system
--
-- Bit 0 => reload FPGA configuration (active high)
-- Bit 1..n => reset_out(1 .. n)
-------------------------------------------------------------------------------
--
-- Copyright (c) 2013 GSI / Stefan Rauch
--
-- This source file is free software; you can redistribute it   
-- and/or modify it under the terms of the GNU Lesser General   
-- Public License as published by the Free Software Foundation; 
-- either version 2.1 of the License, or (at your option) any   
-- later version.                                               
--
-- This source is distributed in the hope that it will be       
-- useful, but WITHOUT ANY WARRANTY; without even the implied   
-- warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      
-- PURPOSE.  See the GNU Lesser General Public License for more 
-- details.                                                     
--
-- You should have received a copy of the GNU Lesser General    
-- Public License along with this source; if not, download it   
-- from http://www.gnu.org/licenses/lgpl-2.1.html
-- 
--
-------------------------------------------------------------------------------
-- Revisions  :
-- Date        Version  Author      Description
-- 2013-09-13  1.0      stefanrauch first version
-------------------------------------------------------------------------------
-- 2014-09-16  1.1      mkreider 	- FPGA reset needs DEADBEEF as magic
--					word at address 0x0
--					- 0x4 - 0xC are now GET, SET, CLR for
--                                        individual LM32 reset lines  		
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wb_arria_reset_pkg.all;

entity wb_arria_reset is
  generic (
            arria_family: string := "Arria II";
            rst_channels: integer range 1 to 32 := 2
          );
  port (
          clk_sys_i:  in std_logic;
          rstn_sys_i: in std_logic;
          clk_upd_i:  in std_logic;
          rstn_upd_i: in std_logic;
          
          slave_o:    out t_wishbone_slave_out;
          slave_i:    in t_wishbone_slave_in;
           
          rstn_o:     out std_logic_vector(rst_channels-1 downto 0)
      );
end entity;


architecture wb_arria_reset_arch of wb_arria_reset is
  signal reset_reg: std_logic_vector(31 downto 0);
  signal reset : std_logic;
begin
  
  reset <= not rstn_upd_i;
  
  ruc_gen_a2 : if arria_family = "Arria II" generate
    arria_reset_inst : arria_reset PORT MAP (
      clock	      => clk_upd_i,
      param	      => "000",
      read_param	=> '0',
      reconfig	  => reset_reg(0),
      reset	      => reset,
      reset_timer	=> '0',
      busy	      => open,
      data_out	  => open
    );
  end generate;
  
  ruc_gen_a5 : if arria_family = "Arria V" generate
    arria5_reset_inst : arria5_reset PORT MAP (
      clock	      => clk_upd_i,
      param	      => "000",
      read_param	=> '0',
      reconfig	  => reset_reg(0),
      reset	      => reset,
      reset_timer	=> '0',
      busy	      => open,
      data_out	  => open
    );
  end generate;
  
  rst_out_gen: for i in 0 to rst_channels-1 generate
    rstn_o(i) <= not reset_reg(i+1);
  end generate;
  
  slave_o.err <= '0';
  slave_o.stall <= '0';
  
  wb_reg: process(clk_sys_i)
  begin
    if rising_edge(clk_sys_i) then
      slave_o.ack <= slave_i.cyc and slave_i.stb;
      slave_o.dat <= (others => '0');
  
      if rstn_sys_i = '0' then
        reset_reg <= (others => '0');
      else
        -- Detect a write to the register byte
        if slave_i.cyc = '1' and slave_i.stb = '1' and slave_i.sel(0) = '1' then
          if(slave_i.we = '1') then
				 case to_integer(unsigned(slave_i.adr(3 downto 2))) is
					when 0 => if(slave_i.dat = x"DEADBEEF") then
									reset_reg(0) <= '1';
								 end if;
					
					when 2 => reset_reg(reset_reg'left downto 1) <= reset_reg(reset_reg'left downto 1) OR slave_i.dat(reset_reg'left-1 downto 0);
					when 3 => reset_reg(reset_reg'left downto 1) <= reset_reg(reset_reg'left downto 1) AND NOT slave_i.dat(reset_reg'left-1 downto 0);
					
					when others => null;
				 end case;
			 else
				 case to_integer(unsigned(slave_i.adr(3 downto 2))) is
					when 1 => slave_o.dat <= '0' & reset_reg(reset_reg'left downto 1); 
					when others => null;
				 end case;
			 end if;
        end if;
		  
		  
      end if;
    end if;
  end process;
end architecture;
