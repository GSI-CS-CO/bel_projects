--! @file        eca_tap.vhd
--  DesignUnit   eca_tap
--! @author      M. Kreider <m.kreider@gsi.de>
--! @date        17/02/2017
--! @version     0.0.1
--! @copyright   2017 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!
--! @brief *** Wire Tap for ECA to determine leftover delay budget of messages (min, max, arith. mean)***
--!
--------------------------------------------------------------------------------
--! This library is free software; you can redistribute it and/or
--! modify it under the terms of the GNU Lesser General Public
--! License as published by the Free Software Foundation; either
--! version 3 of the License, or (at your option) any later version.
--!
--! This library is distributed in the hope that it will be useful,
--! but WITHOUT ANY WARRANTY; without even the implied warranty of
--! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
--! Lesser General Public License for more details.
--!
--! You should have received a copy of the GNU Lesser General Public
--! License along with this library. If not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.wbgenplus_pkg.all;
--use work.genram_pkg.all;
use work.eca_tap_auto_pkg.all;



entity eca_tap is generic
(
  g_build_tap : boolean := true
);
port (
  clk_i        : in  std_logic;  -- system clock 
  rst_n_i      : in  std_logic;  -- reset, active low 
  tm_tai8ns_i  : in  std_logic_vector(63 downto 0) := (others => '0');
    
  ctrl_o       : out t_wishbone_slave_out;
  ctrl_i       : in  t_wishbone_slave_in;

  tap_out_o    : out t_wishbone_master_out; 
  tap_out_i    : in  t_wishbone_master_in := ('0', '0', '0', '0', '0', x"00000000");
  tap_in_o     : out t_wishbone_slave_out;
  tap_in_i     : in  t_wishbone_slave_in

);
end eca_tap;

architecture rtl of eca_tap is

  signal r_dl_hi, r_dl_lo : unsigned(31 downto 0) := (others => '0'); 
  signal r_min, r_max, s_dl, r_diff_acc, r_diff, s_diff : signed(63 downto 0) := (others => '0'); --check again - do we need it that wide?
  signal r_cnt_word : unsigned(3 downto 0) := (others => '0'); 
  signal r_cnt_msg  : unsigned(63 downto 0) := (others => '0');
  signal s_en, r_en0, s_push, s_new_min, s_new_max, s_inc_msg : std_logic := '0';
  signal s_valid : std_logic_vector(0 downto 0) := (others => '0');

  constant c_DIFF_MIN   : natural := 0;
  constant c_DIFF_MAX   : natural := 1;
  constant c_DIFF_MEAN  : natural := 2;

  signal s_ctrl_error_i       : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Error control
  signal s_ctrl_stall_i       : std_logic_vector(1-1 downto 0)  := (others => '0'); -- flow control
  signal s_ctrl_reset_o       : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Resets ECA-Tap
  signal s_ctrl_clear_o       : std_logic_vector(3-1 downto 0)  := (others => '0'); -- b2: clear count/accu, b1: clear max, b0: clear min
  signal s_ctrl_capture_o     : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Enable/Disable Capture
  signal s_ctrl_cnt_msg_i     : std_logic_vector(64-1 downto 0) := (others => '0'); -- Message Count
  signal s_ctrl_diff_acc_i    : std_logic_vector(64-1 downto 0) := (others => '0'); -- Accumulated differences (dl - ts)
  signal s_ctrl_diff_min_i    : std_logic_vector(64-1 downto 0) := (others => '0'); -- Minimum difference
  signal s_ctrl_diff_max_i    : std_logic_vector(64-1 downto 0) := (others => '0'); -- Maximum difference
  


begin

  tap_out_o <= tap_in_i;
  tap_in_o  <= tap_out_i;

instOn: if g_build_tap = TRUE generate

--WB IF
  INST_eca_tap_auto : eca_tap_auto
  port map (
    clk_sys_i     => clk_i,
    rst_sys_n_i   => rst_n_i,
    error_i       => "0",
    stall_i       => "0",
    reset_o       => s_ctrl_reset_o,
    clear_o       => s_ctrl_clear_o,
    capture_o     => s_ctrl_capture_o,
    cnt_msg_V_i   => s_valid,
    cnt_msg_i     => s_ctrl_cnt_msg_i,
    diff_acc_V_i  => s_valid,
    diff_acc_i    => s_ctrl_diff_acc_i,
    diff_min_V_i  => s_valid,
    diff_min_i    => s_ctrl_diff_min_i,
    diff_max_V_i  => s_valid,
    diff_max_i    => s_ctrl_diff_max_i,
    ctrl_i        => ctrl_i,
    ctrl_o        => ctrl_o  );


  s_valid(0)        <= not (s_en or r_en0);
  s_ctrl_cnt_msg_i  <= std_logic_vector(r_cnt_msg);
  s_ctrl_diff_acc_i <= std_logic_vector(r_diff_acc);
  s_ctrl_diff_min_i <= std_logic_vector(r_min);
  s_ctrl_diff_max_i <= std_logic_vector(r_max);

  s_push    <= tap_in_i.cyc AND tap_in_i.stb AND NOT tap_out_i.stall; -- incoming word enable

  s_new_min <= '1' when r_diff < r_min                               -- found new minimum?
          else '0';

  s_new_max <= '1' when r_diff > r_max                               -- found new minimum?  
          else '0';

  s_dl      <= signed(r_dl_hi & r_dl_lo);                            -- deadline ts registers

  s_diff    <= s_dl - signed(tm_tai8ns_i);                         -- deadline diff

  --no protection against invalid timing messages, but that can come later
  s_inc_msg   <= r_cnt_word(r_cnt_word'left);                         -- word count underflow
  s_en        <= s_inc_msg;                                           -- enable signal

  word_cnt : process(clk_i)
  begin
    if rising_edge(clk_i) then
      if(rst_n_i = '0' or s_en = '1') then -- reset also on underflow
        r_cnt_word <= to_unsigned(7, r_cnt_word'length);      
      else
        if s_push = '1' then
          r_cnt_word <= r_cnt_word - resize(unsigned(s_ctrl_capture_o), r_cnt_word'length);      
        end if; 
      end if;   
    end if;
  end process;

  main : process(clk_i)
  begin
    if rising_edge(clk_i) then
      
      r_dl_hi <= r_dl_lo; 
      
     if(rst_n_i = '0' or s_ctrl_reset_o = "1") then
        r_dl_lo             <= (others => '0');
        r_min               <= (others => '1'); -- set rmin to maximum positive value 
        r_min(r_min'left)   <= '0';
        r_max               <= (others => '1'); -- set rmax to maximum negative value 
        r_diff_acc          <= (others => '0');
        r_cnt_msg           <= (others => '0');
        r_en0               <= '0';
      else
        r_en0  <= s_en;  -- on r_en = '1', s_dl contains timestamp
        r_diff <= s_diff;

        if s_push = '1' then  -- save word on push
          r_dl_lo <= unsigned(tap_in_i.dat);
        end if;

        -- Diff Min
        if s_ctrl_clear_o(c_DIFF_MIN) = '1' then
          r_min               <= (others => '1'); -- set rmin to maximum positive value 
          r_min(r_min'left)   <= '0';
        elsif (s_new_min and r_en0) = '1' then 
          r_min <= r_diff;  -- save minimum diff
        end if;

        -- Diff Max
        if s_ctrl_clear_o(c_DIFF_MAX) = '1' then
          r_max <= (others => '1'); -- set rmax to maximum negative value 
        elsif (s_new_max and r_en0) = '1' then 
          r_max <= r_diff;  -- save maximum diff
        end if;

        -- Diff Mean (Acc & Cnt)
        if s_ctrl_clear_o(c_DIFF_MEAN) = '1' then
          r_diff_acc          <= (others => '0');
          r_cnt_msg           <= (others => '0');
        elsif r_en0 = '1' then
          r_diff_acc <= r_diff_acc + r_diff;  -- add to diff accumulator
          r_cnt_msg  <= r_cnt_msg  + 1;       -- inc msg counter
        end if;        

      end if;   
    end if;
  end process;

end generate;

end architecture;




