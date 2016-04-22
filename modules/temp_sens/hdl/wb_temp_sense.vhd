-------------------------------------------------------------------------------
-- Title      : Altera Temperature Sensor IP Core
-------------------------------------------------------------------------------
-- File       : wb_temp_sense.vhd
-- Author     : Anjan Suresh
-- Company    : GSI
-- Created    : 2016-04-22
-- Last update: 2016-04-22
-- Platform   : Altera
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: Read the temperature value of Arria5 devices
-------------------------------------------------------------------------------
--
-- Copyright (c) 2016 GSI / Anjan Suresh
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
-- Date        Version  Author    Description
-- 2016-04-22  1.0      Suresh    FPGA Device temperature reading
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;
use work.temp_sensor_pkg.all;

entity wb_temp_sense is
  generic (
    g_address_size  : natural := 32;  -- in bit(s)
    g_data_size     : natural := 32;  -- in bit(s)
    g_spi_data_size : natural := 8    -- in bit(s)
          );
  port (
    -- generic system interface
    clk_sys_i  : in  std_logic;
    rst_n_i    : in  std_logic;
    -- wishbone slave interface
    slave_i    : in  t_wishbone_slave_in;
    slave_o    : out t_wishbone_slave_out;
    clr_o      : out std_logic
       );
end wb_temp_sense;

--architecture

architecture rtl of wb_temp_sense is

--wishbone signals
    signal s_wb_cyc		: std_logic ;
    signal s_wb_stb		: std_logic ;
    signal s_wb_we		: std_logic ;
    signal s_wb_adr		: std_logic_vector(31 downto 0);
    signal s_wb_sel		: std_logic_vector(3 downto 0);
    signal s_wb_ack 	        : std_logic := '0';
    signal s_wb_stall	        : std_logic := '0';
    signal s_wb_dat		: std_logic_vector(31 downto 0) := (others => '0');
    signal s_wb_dat_i           : std_logic_vector(31 downto 0) := (others => '0');

--temperature sensor signals
    signal s_ce			: std_logic := '1';
    signal s_clr		: std_logic := '0';
    signal s_tsdcaldone	        : std_logic := '0';
    signal s_tsdcalo		: std_logic_vector(7 downto 0);

--internal signals
    signal s_clr_o  	        : std_logic;
    signal s_count		: unsigned(11 downto 0);
    signal s_temp_val		: std_logic_vector(7 downto 0);
--constants
    constant c_address_tx_data  : std_logic_vector (1 downto 0):= "00";

begin
    s_wb_cyc	  <= slave_i.cyc;
    s_wb_stb	  <= slave_i.stb;
    s_wb_we	  <= slave_i.we;
    s_wb_adr	  <= slave_i.adr;
--    s_wb_dat_i	  <= slave_i.dat; ## Can be used to write data in future if necessary
    s_wb_sel	  <= slave_i.sel;
    slave_o.ack	  <= s_wb_ack;
    slave_o.stall <= s_wb_stall;
    slave_o.dat	  <= s_wb_dat;
    slave_o.err	  <= '0';
    slave_o.int	  <= '0';
    slave_o.rty	  <= '0';
    clr_o	  <= s_clr_o;

-----------------------------------------------------
--Process description
--
--Assert the clear signal to clear the data for
--next cycle to read the temperature value
--
-----------------------------------------------------

  p_clk_temp_clr: process(clk_sys_i)

  begin
    if rising_edge (clk_sys_i) then
      if (rst_n_i='0') then
        s_count 	<= (others => '0');
	s_clr_o 	<= '1';
	s_temp_val	<= (others => '0');
      else

      if (s_count <= x"120") then
        s_count   	<= s_count + 1;
	s_clr_o 	<= '1';
      elsif (s_count > x"120") then
	s_clr_o 	<= '0';
	s_count   	<= s_count+1;
      elsif  (s_count > x"450") then
	s_count 	<= (others => '0');
      end if; 

      end if;
    end if;
  end process;


temperature_sensor : temp_sens

  port map (
    ce		=> s_ce,
    clk		=> clk_sys_i,
    clr		=> s_clr_o,
    tsdcaldone	=> s_tsdcaldone,
    tsdcalo		=> s_tsdcalo
           );

---------------------------------------------------
--Process Description
--
--Read the temperature sensor data from the device
--to the wishbone slave
--
---------------------------------------------------

  p_clk_wb_read: process (clk_sys_i)

  begin
    if rising_edge(clk_sys_i) then
      if (rst_n_i='0') then
        s_wb_ack	<= '0';
      else
	s_wb_ack        <= s_wb_cyc and s_wb_stb;
      if (s_wb_cyc='1' and s_wb_stb='1') then
	case s_wb_adr(3 downto 2) is
          when c_address_tx_data =>
	    if (s_wb_we='0'and s_clr_o='0' and s_tsdcaldone='1') then
	      s_wb_dat(7 downto 0) 	<= s_tsdcalo;
	      s_wb_dat(31 downto 8)	<= (others => '0');
	    else
	      s_wb_dat <= x"DEADC0DE";
	    end if;
	  when others =>
	    s_wb_dat <= x"DEADC0DE";
	end case;		
      end if;--sensor reading complete
      end if;--cycle and strobe check
    end if;--reset check
  end process;
end rtl;
