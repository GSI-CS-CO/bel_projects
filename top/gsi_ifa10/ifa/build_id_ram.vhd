---------------------------------------------------------------------------------
-- filename: build_id_ram.vhd
-- desc: build id wrapper for eb-info
-- creation date: 20.12.2016
-- last modified: 13.02.2017
-- author: Stefan Rauch <s.rauch@gsi.de>
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

library altera_mf;
use altera_mf.altera_mf_components.all;

entity build_id_ram is
  port (
    clk:      in std_logic;
    sclr:     in std_logic;
    str:      in std_logic;

    build_id_out: out std_logic_vector(15 downto 0)
  );
end entity;

architecture arch of build_id_ram is
  signal rom_addr:        unsigned(8 downto 0);
  signal str_reg:         std_logic_vector(1 downto 0);
  signal str_edge:        std_logic;
  signal s_build_id_out:  std_logic_vector(31 downto 0);

-- vK
--component BuildID_ROM
--	PORT
--	(
--		address		: IN STD_LOGIC_VECTOR (2 DOWNTO 0);
--		clock		: IN STD_LOGIC  := '1';
--		q		: OUT STD_LOGIC_VECTOR (31 DOWNTO 0)
--	);
--end component;



begin
  str_pulse: process(clk,str,str_reg)
  begin
    if rising_edge(clk) then
      str_reg(0) <= str;
      str_reg(1) <= str_reg(0);
    end if;
  end process;

  str_edge <= str_reg(0) and not str_reg(1);

  ram_cnt: process(clk,sclr,str_edge,rom_addr)
  begin
    if sclr = '1' then
      rom_addr <= (others => '0');
    elsif rising_edge(clk) then
      if str_edge = '1' then
        rom_addr <= rom_addr + 1;
        end if;
    end if;
  end process;

--  build_id_rom: altsyncram
--  generic map (
--    operation_mode  => "ROM",
--    width_a         => 32,
--    widthad_a       => 8,
--    init_file       => "build_id.mif")
--  port map (
--    clock0          => clk,
--    address_a       => std_logic_vector(rom_addr(8 downto 1)),
--    q_a             => s_build_id_out,
--    q_b             => open);

--build_id_rom: BuildID_ROM
--port map (
--    clock          => clk,
--    address       => std_logic_vector(rom_addr(3 downto 1)),
--    q             => s_build_id_out);

 --Vk workaround -- internal RamConfig beu dual-FLASH nicht möglich
  out_reg: process(clk, sclr,s_build_id_out,str_edge,rom_addr)
  begin
    if rising_edge(clk) then
      if str_edge = '1' then
        if rom_addr(0) = '0' then
	   build_id_out <= x"2020";--s_build_id_out(31 downto 16);
	  --build_id_out <= s_build_id_out(31 downto 16);
        else
          build_id_out <= x"4944";--s_build_id_out(15 downto 0);
--	  build_id_out <= s_build_id_out(15 downto 0);
        end if;
      end if;
    end if;
  end process;

end architecture;
