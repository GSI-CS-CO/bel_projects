-------------------------------------------------------------------------------
-- Title      : Test slave
-- Project    : all Arria platforms
-------------------------------------------------------------------------------
-- File       : en_err_counter.vhd
-- Author     : Lucas Herfurth
-- Company    : GSI
-- Created    : 2024-07-09
-- Last update: 2024-07-09
-- Platform   : Altera
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: Wishbone slave  that counts encoding errors from the white 
-- rabbit core in the reference clock domain. These error counts then get 
-- transferred into the system clock domain by an asynchronous FIFO. The 
-- counter size needs to be a power of 2.
--
--
-------------------------------------------------------------------------------
--
-- Copyright (c) 2024 GSI / Lucas Herfurth
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

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all; --not used???

library work;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.genram_pkg.all;

entity enc_err_counter is
	--possible generics like clock speeds?
	port (
		clk_sys_i     : in std_logic;
		clk_ref_i	  : in std_logic;
		rstn_sys_i    : in std_logic;

		slave_o       : out t_wishbone_slave_out;
		slave_i       : in  t_wishbone_slave_in;
		
		enc_err_i	  : in std_logic
	);
end entity;

architecture enc_err_counter_arc of enc_err_counter is
	
	constant c_counter_bits : integer := 32;

	subtype t_counter is std_logic_vector(c_counter_bits-1 downto 0);

	-- bin: binary counter
	-- bin_next: bin + 1
	-- bin_x: cross-clock domain version of bin
	-- gray: gray code of bin
	-- gray_next: gray code of bin_next
	-- gray_x: gray code of bin_x
	--
	-- We use gray codes for safe cross-clock domain crossing of counters. Thus,
	-- a binary counter is converted to gray before crossing, and then it is
	-- converted back to binary after crossing.
	type t_counter_block is record
		bin, bin_next   : t_counter;
		gray, gray_next : t_counter;
		bin_x, gray_x   : t_counter;
	end record;
	
	signal cnt	: t_counter_block := (others =>(others => '0'));
	
	signal overflow_reg : std_logic_vector(31 downto 0) := x"00000000";
	
	-- wishbone controller
	signal ack_flag		 : std_logic := '0';
	
begin------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	-- wishbone controller
	slave_o.err   <= '0';
	slave_o.stall <= '0';
	
	wb_read_write: process(clk_sys_i)
	begin
		if rising_edge(clk_sys_i) then
			if (rstn_sys_i = '0') then
				--TBD reset init
			else
			
				--limits the ack signal to one pulse, so the slave guarantees that it is finished
				if ack_flag = '0' and slave_i.cyc = '1' and slave_i.stb = '1' then --add possible check if action is done -> otherwise delay the ack
					slave_o.ack <= '1';
					ack_flag <= '1';
					if slave_i.we = '1' then --read or write
						--reg_mem <= slave_i.dat;
					else
					
						if slave_i.adr(7 downto 0) = x"00" then
							slave_o.dat <= cnt.bin_x;
						elsif slave_i.adr(7 downto 0) = x"04" then -- 0x04
							-- second register													
						else 
							slave_o.dat <= overflow_reg;
						end if;
					end if;
				else
					slave_o.ack <='0';
					ack_flag <= '0';
				end if; --ack				
			
			end if; --sync reset
		end if; --rising edge	
	end process;
	
	-- error counter
	cnt.bin_next  <= std_logic_vector(unsigned(cnt.bin) + 1);
  	cnt.gray_next <= f_gray_encode(cnt.bin_next);
  	
  	U_Sync : gc_sync_register
    generic map (
      g_width => c_counter_bits)
    port map (
      clk_i     => clk_sys_i,
      rst_n_a_i => rstn_sys_i,
      d_i       => cnt.gray,
      q_o       => cnt.gray_x);
  	
  	cnt.bin_x <= f_gray_decode(cnt.gray_x, 1);
	
	-- reference clock domain
	process (clk_ref_i) begin
		if rstn_sys_i = '1' then
			if rising_edge(clk_ref_i) and enc_err_i = '1' then
				cnt.bin <= cnt.bin_next;
				cnt.gray <= cnt.gray_next;

				if cnt.bin_next = x"FFFFFFFF" then
					overflow_reg <= x"00000001";					
				end if;
			end if;
		else 
			cnt.bin  <= (others => '0');
		end if;
	end process;
	
end architecture;



