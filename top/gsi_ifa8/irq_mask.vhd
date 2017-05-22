---------------------------------------------------------------------------------
-- filename: irq_mask.vhd
-- desc: masking registers for interlock, data ready, data request and powerup interrupt requests
-- creation date: 19.05.2017
-- last modified: 19.05.2017
-- author: Stefan Rauch <s.rauch@gsi.de>
-- based on inrm-c.bdf from R.Hartmann
--
-- Copyright (C) 2017 GSI Helmholtz Centre for Heavy Ion Research GmbH 
--
---------------------------------------------------------------------------------
-- This library is free software; you can redistribute it and/or
-- modify it under the terms of the GNU Lesser General Public
-- License as published by the Free Software Foundation; either
-- version 3 of the License, or (at your option) any later version.
--
-- This library is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- Lesser General Public License for more details.
--  
-- You should have received a copy of the GNU Lesser General Public
-- License along with this library. If not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------- 

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity irq_mask is
  port (
    intl_d       : in std_logic;
    drdy_d       : in std_logic;
    drq_d        : in std_logic;
    pures_d      : in std_logic;
  
    n_intl_si    : in std_logic;
    n_drdy_si    : in std_logic;
    n_drq_si     : in std_logic;

    clk          : in std_logic;
    wr_irm       : in std_logic;
    sclr         : in std_logic;
    
    intl_q       : buffer std_logic;
    drdy_q       : buffer std_logic;
    drq_q        : buffer std_logic;
    n_intl_out   : out std_logic;
    n_drdy_out   : out std_logic;
    n_drq_out    : out std_logic;
    powerup_flag : out std_logic;
    n_opt_inl    : out std_logic;
    n_opt_drdy   : out std_logic;
    n_opt_drq    : out std_logic);
end entity irq_mask;

architecture arch of irq_mask is
	signal s_n_power_up  : std_logic;
	signal s_intl_mask_q : std_logic;
	signal s_drdy_mask_q : std_logic;
	signal s_drq_mask_q  : std_logic;
begin
  irq_reg: process(clk, sclr)
  begin
    if sclr = '1' then
      intl_q <= '1';
      drq_q  <= '0';
      drdy_q <= '0';
    elsif rising_edge(clk) then
      if wr_irm = '1' then
        intl_q <= intl_d;
        drq_q  <= drq_d;
        drdy_q <= drdy_d;
      end if;
    end if;
  end process;
  
  pur_reg: process(clk, sclr)
  begin
    if sclr = '1' then
      s_n_power_up <= '0';
    elsif rising_edge(clk) then
      if wr_irm = '1' and pures_d = '1' then
        s_n_power_up <= '1';
      end if;
    end if;
  end process;

  mask_reg: process(clk, sclr)
  begin
    if rising_edge(clk) then
      s_intl_mask_q <= not s_n_power_up or not n_intl_si;
      s_drdy_mask_q <= not n_drdy_si;
      s_drq_mask_q  <= not n_drq_si;
    end if;
  end process;

n_opt_inl    <= intl_q nand s_intl_mask_q;
n_opt_drdy   <= drdy_q nand s_drdy_mask_q;
n_opt_drq    <= drq_q nand s_drq_mask_q;

powerup_flag <= not s_n_power_up;
n_intl_out   <= n_intl_si;
n_drdy_out   <= n_drdy_si;
n_drq_out    <= n_drq_si;

end architecture arch;
