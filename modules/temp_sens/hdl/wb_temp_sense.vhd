-------------------------------------------------------------------------------
-- Title      : Altera Temperature Sensor IP Core
-------------------------------------------------------------------------------
-- File       : wb_temp_sense.vhd
-- Author     : Anjan Suresh, Enkhbold Ochirsuren
-- Company    : GSI
-- Created    : 2016-04-22
-- Last update: 2019-01-18
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
-- Date        Version  Author      Description
-- 2016-04-22  1.0      Suresh      FPGA Device temperature reading
-- 2019-01-18  1.1      Ochirsuren  Supports temperature reading in degree
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;
use work.temp_sensor_pkg.all;

entity wb_temp_sense is
  generic (
    g_address_size  : natural := 32;  -- in bit(s)
    g_data_size     : natural := 32;  -- in bit(s)
    g_spi_data_size : natural := 8;   -- in bit(s)
    g_addr_width    : natural := 32;  -- wb addr bus width
    g_data_width    : natural := 32;  -- wb data bus width
    g_ts_data_width : natural := 8;   -- temperature sensor data width
    g_ts_clk_div    : natural := 80;  -- temperature sensor clock divider
    g_ts_clr_cycles : natural := 3;   -- adcclk cycles for clr, min=1
    g_ts_adc_cycles : natural := 12   -- adcclk cycles for a/d conversion, nom=10
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

--constants
    constant c_address_tx_data  : std_logic_vector (1 downto 0):= "00";  -- sensor data
    constant c_address_temp     : std_logic_vector (1 downto 0):= "01";  -- temperature in degree
    constant c_temp_coef        : signed(g_ts_data_width downto 0):= "110000000"; -- -128
    constant c_ts_clr_cnt       : integer := integer(ceil(real(g_ts_clk_div) * real(g_ts_clr_cycles)));
    constant c_ts_clr_max       : integer := 2**(integer(ceil(log2(real(c_ts_clr_cnt)))));
    constant c_ts_adc_cnt       : integer := integer(ceil(real(g_ts_clk_div) * real(g_ts_clr_cycles + g_ts_adc_cycles)));
    constant c_ts_adc_max       : integer := 2**(integer(ceil(log2(real(c_ts_adc_cnt)))));
    constant c_ts_cnt_width     : integer := integer(ceil(log2(real(c_ts_adc_max + 1))));

--wishbone signals
    signal s_wb_cyc		: std_logic ;
    signal s_wb_stb		: std_logic ;
    signal s_wb_we		: std_logic ;
    signal s_wb_sel		: std_logic_vector(3 downto 0);
    signal s_wb_ack 	        : std_logic := '0';
    signal s_wb_stall	        : std_logic := '0';
    signal s_wb_adr   : std_logic_vector(g_addr_width -1 downto 0);
    signal s_wb_dat   : std_logic_vector(g_data_width -1 downto 0) := (others => '0');
    signal s_wb_dat_i : std_logic_vector(g_data_width -1 downto 0) := (others => '0');

--temperature sensor signals
    signal s_ce			: std_logic := '1';
    signal s_clr		: std_logic := '0';
    signal s_tsdcaldone	        : std_logic := '0';
    signal s_tsdcalo    : std_logic_vector(g_ts_data_width -1 downto 0);

--internal signals
    signal s_clr_o  	        : std_logic;
    signal s_count		: unsigned(c_ts_cnt_width -1 downto 0);
    signal s_ts_data  : std_logic_vector(g_ts_data_width downto 0);  -- latest sensor data
    signal s_temp     : signed(g_ts_data_width downto 0);  -- latest temperature value

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
--Reading from the temperature sensor diode is done
--periodically. When tsd signals the completion of
--ADC conversion, then its sensor data is acquired and
--next read cycle is started. If ADC takes far longer
--than expected period, then fault bit is set.
--
-----------------------------------------------------

  p_clk_temp_clr: process(clk_sys_i)

  begin
    if rising_edge (clk_sys_i) then
      if (rst_n_i='0') then
        s_count 	<= (others => '0');
        s_clr_o 	<= '1';
        s_ts_data <= (g_ts_data_width => '0', others =>'1');
     else
        s_count <= s_count + 1;

        if (s_count < c_ts_clr_cnt) then
          s_clr_o <= '1';
        elsif (s_count = c_ts_clr_cnt) then
          s_clr_o <= '0';
        elsif  (s_count > c_ts_adc_max) then -- conversion failure/timeout
          s_ts_data <= '1' & s_tsdcalo;      -- set the fault bit
          s_count   <= (others => '0');
        elsif (s_tsdcaldone = '1') then
          s_ts_data <= '0' & s_tsdcalo;
          s_temp    <= signed('0' & s_tsdcalo) + c_temp_coef;
          s_count   <= (others => '0');
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
--to the wishbone slave. The MSB bit indicates the
--fault/timeout in sensor data conversion:
--if ADC is failed, then MSB is set to '1'.
--
--The temperature value in degree is read at the
--next offset of the base address. In case of ADC
--fault it returns a value of 384.
--
---------------------------------------------------

  p_clk_wb_read: process (clk_sys_i)

  begin
    if rising_edge(clk_sys_i) then
      if (rst_n_i='0') then
        s_wb_stall <= '0';
        s_wb_ack	<= '0';

      else
        s_wb_ack <= s_wb_cyc and s_wb_stb and not s_wb_stall;

        if (s_wb_cyc='1' and s_wb_stb='1' and s_wb_stall='0') then -- wb cycle

          if (s_wb_we = '0') then -- read access

            case s_wb_adr(3 downto 2) is
              when c_address_tx_data =>  -- latest sensor raw data with the fault flag
                s_wb_dat(g_ts_data_width -1 downto 0)            <= s_ts_data(g_ts_data_width -1 downto 0);
                s_wb_dat(g_data_width -2 downto g_ts_data_width) <= (others => '0');
                s_wb_dat(g_data_width -1)                        <= s_ts_data(g_ts_data_width);
              when c_address_temp    =>  -- temperature in degree
                if (s_ts_data(g_ts_data_width) = '1') then -- temperature=384, if conversion fails
                  s_wb_dat(g_ts_data_width downto 0)                  <= std_logic_vector(c_temp_coef);
                  s_wb_dat(g_data_width -1 downto g_ts_data_width +1) <= (others => '0');
                else
                  s_wb_dat(g_ts_data_width downto 0)                  <= std_logic_vector(s_temp);
                  s_wb_dat(g_data_width -1 downto g_ts_data_width +1) <= (others => std_logic(s_temp(g_ts_data_width)));
                end if;
              when others =>  -- wrong address
                s_wb_dat <= x"DEADC0DE";
              end case;

          end if;--sensor reading complete
        end if;--cycle and strobe check
      end if;--reset check
    end if;
  end process;
end rtl;
