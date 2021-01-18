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
-- 2016-01-7  1.2      srauch 	- added register for hw version number
--					                    - read from address offset 0x8
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;


library work;
use work.wishbone_pkg.all;
use work.wb_arria_reset_pkg.all;
use work.aux_functions_pkg.all;

entity wb_arria_reset is
  generic (
    arria_family : string := "Arria II";
    rst_channels : integer range 1 to 32 := 2;
    clk_in_hz    : integer;
    en_wd_tmr    : boolean
  );
  port (
    clk_sys_i  : in std_logic;
    rstn_sys_i : in std_logic;
    clk_upd_i  : in std_logic;
    rstn_upd_i : in std_logic;

    hw_version : in std_logic_vector(31 downto 0);

    slave_o    : out t_wishbone_slave_out;
    slave_i    : in t_wishbone_slave_in;

    rstn_o     : out std_logic_vector(rst_channels-1 downto 0)
  );
end entity;


architecture wb_arria_reset_arch of wb_arria_reset is
  signal reset_reg        : std_logic_vector(31 downto 0);
  signal reset            : std_logic;
  signal en_1ms           : std_logic;
  signal trigger_reconfig : std_logic;
  signal halt_wd          : std_logic;
  constant cnt_value      : integer := 1000 * 60 * 10; -- 10 min with 1ms granularity
  constant cnt_width      : integer := integer(ceil(log2(real(cnt_value)))) + 1;
  signal reset_or_trigger : std_logic;
begin

  reset <= not rstn_upd_i;

  reset_or_trigger <= reset_reg(0) or trigger_reconfig;
  ruc_gen_a2 : if arria_family = "Arria II" generate
    arria_reset_inst : arria_reset PORT MAP (
      clock       => clk_upd_i,
      param       => "000",
      read_param  => '0',
      reconfig    => reset_or_trigger,
      reset       => reset,
      reset_timer => '0',
      busy        => open,
      data_out    => open
    );
  end generate;

  ruc_gen_a5 : if arria_family = "Arria V" generate
    arria5_reset_inst : arria5_reset PORT MAP (
      clock       => clk_upd_i,
      param       => "000",
      read_param  => '0',
      reconfig    => reset_or_trigger,
      reset       => reset,
      reset_timer => '0',
      busy        => open,
      data_out    => open
    );
  end generate;

  gen_wd: if en_wd_tmr = true generate
    wd_div : div_n generic map (
      n => (clk_in_hz / 1000) + 2 -- 1ms
    )
    port map (
      res => reset,
      clk => clk_sys_i,
      ena => '1',
      div_o => en_1ms
    );

    wd_cnt : process(clk_sys_i)
      variable cnt : unsigned(cnt_width-1 downto 0) := to_unsigned(cnt_value, cnt_width);
    begin
      if rising_edge(clk_sys_i) then
        if en_1ms = '1' and halt_wd = '0' then
          cnt := cnt - 1;
        end if;
        if cnt(cnt'high) = '1' then
          trigger_reconfig <= '1';
        end if;
      end if;
    end process;
  end generate;

  rst_out_gen: for i in 0 to rst_channels-1 generate
    rstn_o(i) <= not reset_reg(i+1);
  end generate;

  gen_wd_off: if en_wd_tmr = false generate
    trigger_reconfig <= '0';
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
              when 0 =>
                if(slave_i.dat = x"DEADBEEF") then
                  reset_reg(0) <= '1';
                end if;

              when 1 =>
                -- disable the watchdog
                if(slave_i.dat = x"CAFEBABE") then
                  halt_wd <= '1';
                end if;
              when 2 => reset_reg(reset_reg'left downto 1) <= reset_reg(reset_reg'left downto 1) OR slave_i.dat(reset_reg'left-1 downto 0);
              when 3 => reset_reg(reset_reg'left downto 1) <= reset_reg(reset_reg'left downto 1) AND NOT slave_i.dat(reset_reg'left-1 downto 0);
              when others => null;
            end case;
          else -- read
            case to_integer(unsigned(slave_i.adr(3 downto 2))) is
              when 1 => slave_o.dat <= '0' & reset_reg(reset_reg'left downto 1);
              when 2 => slave_o.dat <= hw_version;
              when others => null;
            end case;
          end if;
        end if;

      end if; -- of sync reset
    end if; -- of rising_edge
  end process;
end architecture;
