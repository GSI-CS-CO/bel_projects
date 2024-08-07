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
  generic (
    g_aux_phy_interface : boolean
  );

  port (
    clk_sys_i     : in std_logic;
    clk_ref_i     : in std_logic;
    rstn_sys_i    : in std_logic;
    rstn_ref_i    : in std_logic;

    slave_o       : out t_wishbone_slave_out;
    slave_i       : in  t_wishbone_slave_in;
    
    enc_err_i     : in std_logic;
    enc_err_aux_i : in std_logic
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
  
  signal cnt              : t_counter_block               := (others =>(others => '0'));
  signal overflow_reg     : std_logic_vector(31 downto 0) := x"00000000";
  signal rst_counter_sys  : std_logic                     := '0';
  signal rst_counter_ref  : std_logic                     := '0';
  signal rst_shift_reg    : unsigned(1 downto 0)          := "00";
  signal synched_enc_err  : std_logic                     := '0';
  signal reg_mem          : std_logic_vector(31 downto 0) := x"00000000";
  signal reg_overflow     : std_logic_vector(31 downto 0) := x"00000000";
  
  signal cnt_aux              : t_counter_block               := (others =>(others => '0'));
  signal rst_counter_sys_aux  : std_logic                     := '0';
  signal rst_counter_ref_aux  : std_logic                     := '0';  
  signal rst_shift_reg_aux    : unsigned(1 downto 0)          := "00";
  signal synched_enc_err_aux  : std_logic                     := '0';
  signal reg_mem_aux          : std_logic_vector(31 downto 0) := x"00000000";

  -- wishbone controller
  signal ack_flag : std_logic := '0';
  
begin------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  -- wishbone controller
  slave_o.err    <= '0';
  slave_o.stall  <= '0';
  slave_o.rty    <= '0';
  
  wb_read_write: process(clk_sys_i, rstn_sys_i)
  begin
    if (rstn_sys_i = '1') then
      if rising_edge(clk_sys_i) then
        
        --limits the ack signal to one pulse, so the slave guarantees that it is finished
        if ack_flag = '0' and slave_i.cyc = '1' and slave_i.stb = '1' then

          slave_o.ack <= '1';
          ack_flag <= '1';
          
          if slave_i.we = '1' then -- write enable and x"1" in data signifies reset for the counter at the given address
            if slave_i.adr(7 downto 0) = x"00" then
              rst_counter_sys <= slave_i.dat(0);
            elsif slave_i.adr(7 downto 0) = x"04" then
              rst_counter_sys_aux <= slave_i.dat(0);
            end if;
            
          else
            if slave_i.adr(7 downto 0) = x"00" then -- counter 1
              slave_o.dat <= reg_mem;
            elsif slave_i.adr(7 downto 0) = x"04" then -- auxiliary counter
              slave_o.dat <= reg_mem_aux;                        
            elsif slave_i.adr(7 downto 0) = x"08" then -- counter 1 overflow flag
              slave_o.dat <= (31 downto 1 => '0') & reg_overflow(0);
            elsif slave_i.adr(7 downto 0) = x"0C" then -- auxiliary counter overflow flag
              slave_o.dat <= (31 downto 1 => '0') & reg_overflow(1);
            elsif slave_i.adr(7 downto 0) = x"10" then -- second phy interface existance flag
              slave_o.dat <= (31 downto 1 => '0') & reg_overflow(2);
            end if;
          end if;
          
        else
          slave_o.ack         <= '0';
          ack_flag            <= '0';
          rst_counter_sys     <= '0';
          rst_counter_sys_aux <= '0';
          slave_o.dat         <= (others => '0');
        end if; --ack        
        
      end if; --rising edge  
    
    
    else --rstn_sys_i
      slave_o.ack         <= '0';
      ack_flag            <= '0';
      rst_counter_sys     <= '0';
      rst_counter_sys_aux <= '0';
    end if; --rstn_sys_i
  end process;
  
  aux_phy_interface_n : if not g_aux_phy_interface generate
    overflow_reg(2) <= '0';
  end generate;
  aux_phy_interface_y : if g_aux_phy_interface generate
    overflow_reg(2) <= '1';
  end generate;

  --shift register for edge registering for crossing clock domain from system clock to reference clock
  reset_counter_shift_register : process (clk_ref_i, rstn_ref_i)
  begin
    if rstn_ref_i = '1' then
        if rising_edge(clk_ref_i) then
        
        rst_shift_reg     <= shift_left(rst_shift_reg, 1);
        rst_shift_reg(0)  <= rst_counter_sys;
        
        if rst_shift_reg(1) = '0' and rst_shift_reg(0) = '1' then
          rst_counter_ref <= '1';
        else 
          rst_counter_ref <= '0';
        end if;
    
      end if; -- rising_edge(clk_ref_i)
    

    else -- rstn_ref_i
      rst_shift_reg   <= "00";
      rst_counter_ref <= '0';
    end if; -- rstn_ref_i
  end process;

  --shift register for edge registering for crossing clock domain from system clock to reference clock
  reset_counter_shift_register_aux : process (clk_ref_i, rstn_ref_i)
  begin
    if rstn_ref_i = '1' then
      if rising_edge(clk_ref_i) then
        
        rst_shift_reg_aux <= shift_left(rst_shift_reg_aux, 1);
        rst_shift_reg_aux (0) <= rst_counter_sys_aux;

        if rst_shift_reg_aux(1) = '0' and rst_shift_reg_aux(0) = '1' then
          rst_counter_ref_aux <= '1';
        else 
          rst_counter_ref_aux <= '0';
        end if;
      
      end if; -- rising_edge(clk_ref_i)
    
    
    else -- rstn_ref_i
      rst_shift_reg_aux   <= "00";
      rst_counter_ref_aux <= '0';
    end if; -- rstn_ref_i
  end process;
  
  -- error counter clock domain crossing
  cnt.bin_next  <= std_logic_vector(unsigned(cnt.bin) + 1);
  cnt.gray_next <= f_gray_encode(cnt.bin_next);
    
  cnt_aux.bin_next  <= std_logic_vector(unsigned(cnt_aux.bin) + 1);
  cnt_aux.gray_next <= f_gray_encode(cnt_aux.bin_next);
    
  U_Sync : gc_sync_register
  generic map (
    g_width => c_counter_bits)
  port map (
    clk_i     => clk_sys_i,
    rst_n_a_i => rstn_sys_i,
    d_i       => cnt.gray,
    q_o       => cnt.gray_x);
      
  U_Sync_aux : gc_sync_register
  generic map (
      g_width => c_counter_bits)
    port map (
      clk_i     => clk_sys_i,
      rst_n_a_i => rstn_sys_i,
      d_i       => cnt_aux.gray,
      q_o       => cnt_aux.gray_x);
    
  cnt.bin_x <= f_gray_decode(cnt.gray_x, 1);
    
  cnt_aux.bin_x <= f_gray_decode(cnt_aux.gray_x, 1);

  -- separating wishbone interface from reference clock domain
  p_synch_signals_sys: process (clk_sys_i, rstn_sys_i) begin
    if rstn_sys_i = '1' then
      if rising_edge(clk_sys_i) then
        reg_mem           <= cnt.bin_x;
        reg_mem_aux       <= cnt_aux.bin_x;
        reg_overflow      <= overflow_reg;
      end if;
    else
      reg_mem           <= (others => '0');
      reg_mem_aux       <= (others => '0');
      reg_overflow      <= (others => '0');
    end if;
  end process;

  -- synching enc_err_i to ensure synchronous and stable signal
  p_synch_signals_ref: process (clk_ref_i, rstn_ref_i) begin
    if rstn_ref_i = '1' then
      if rising_edge(clk_ref_i) then
        synched_enc_err     <= enc_err_i;
        synched_enc_err_aux <= enc_err_aux_i;
      end if;
    else
      synched_enc_err     <= '0';
      synched_enc_err_aux <= '0';
    end if;
  end process;

  -- reference clock domain
  process (clk_ref_i, rstn_ref_i) begin
    
    if rstn_ref_i = '1' then
      if rising_edge(clk_ref_i) then

        if rst_counter_ref = '0' then  -- counter and overflow flag reset
          if synched_enc_err = '1' then
            if cnt.bin = x"FFFFFFFF" then -- overflow
              overflow_reg(0) <= '1';
            end if;

            cnt.bin   <= cnt.bin_next;
            cnt.gray  <= cnt.gray_next;
          end if;
        else 
          cnt.bin       <= x"00000000";
          cnt.gray      <= f_gray_encode(x"00000000");
          overflow_reg(0) <= '0';
        end if; -- rst_counter_ref: counter and overflow flag reset
        
        if rst_counter_ref_aux = '0' then -- counter and overflow flag reset
          if synched_enc_err_aux = '1' then
            if cnt_aux.bin = x"FFFFFFFF" then -- overflow
              overflow_reg(1) <= '1';
            end if;

            cnt_aux.bin   <= cnt_aux.bin_next;
            cnt_aux.gray  <= cnt_aux.gray_next;
          end if;
        else 
          cnt_aux.bin       <= x"00000000";
          cnt_aux.gray      <= f_gray_encode(x"00000000");
          overflow_reg(1) <= '0';
        end if; -- rst_counter_ref_aux: counter and overflow flag reset

      end if; -- rising_edge(clk_ref_i)


    else --rstn_ref_i
      cnt.bin       <= (others => '0');
      cnt.gray      <= f_gray_encode(x"00000000");
      overflow_reg(1 downto 0) <= "00";
      
      cnt_aux.bin       <= x"00000000";
      cnt_aux.gray      <= f_gray_encode(x"00000000");
    end if; --rstn_ref_i

  end process;
  
end architecture;