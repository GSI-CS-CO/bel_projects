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
use work.genram_pkg.all;
use work.eca_tap_auto_pkg.all;



entity eca_tap is generic
(
  g_build_tap : boolean := true
);
port (
  clk_sys_i       : in  std_logic;  -- system clock 
  rst_sys_n_i     : in  std_logic;  -- reset, active low
  clk_ref_i       : in  std_logic;  -- system clock 
  rst_ref_n_i     : in  std_logic;  -- reset, active low 
  time_ref_i      : in  std_logic_vector(63 downto 0) := (others => '0');
    
  ctrl_o      : out t_wishbone_slave_out;
  ctrl_i      : in  t_wishbone_slave_in;

  tap_out_o   : out t_wishbone_master_out; 
  tap_out_i   : in  t_wishbone_master_in := ('0', '0', '0', '0', '0', x"00000000");
  tap_in_o    : out t_wishbone_slave_out;
  tap_in_i    : in  t_wishbone_slave_in

);
end eca_tap;

architecture rtl of eca_tap is

  signal r_dl_hi, r_dl_lo : unsigned(31 downto 0) := (others => '0'); 
  signal r_min, r_max, s_dl, r_diff_acc, r_diff, s_diff : signed(63 downto 0) := (others => '0'); --check again - do we need it that wide?
  signal r_cnt_word : unsigned(3 downto 0) := (others => '0'); 
  signal r_cnt_msg  : unsigned(63 downto 0) := (others => '0');
  signal r_cnt_late  : unsigned(31 downto 0) := (others => '0');
  signal s_diff_neg, s_diff_gt_max_offs, s_diff_lo_lt_offs : std_logic := '0';
  signal r_is_late : std_logic_vector(0 downto 0) := (others => '0');
  signal s_en, r_en0, r_en1, s_push, s_new_min, s_new_max, s_inc_msg : std_logic := '0';
  signal s_valid : std_logic_vector(0 downto 0) := (others => '0');

  signal s_empty, s_full, r_push, s_pop : std_logic;
  signal s_time_sys : std_logic_vector(63 downto 0);

  constant c_DIFF_MIN   : natural := 0;
  constant c_DIFF_MAX   : natural := 1;
  constant c_DIFF_MEAN  : natural := 2;
  constant c_CNT_LATE   : natural := 3;

  signal s_ctrl_error_i       : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Error control
  signal s_ctrl_stall_i       : std_logic_vector(1-1 downto 0)  := (others => '0'); -- flow control
  signal s_ctrl_reset_o       : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Resets ECA-Tap
  signal s_ctrl_clear_o       : std_logic_vector(4-1 downto 0)  := (others => '0'); -- b2: clear count/accu, b1: clear max, b0: clear min
  signal s_ctrl_capture_o     : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Enable/Disable Capture
  signal s_ctrl_cnt_msg_i     : std_logic_vector(64-1 downto 0) := (others => '0'); -- Message Count
  signal s_ctrl_diff_acc_i    : std_logic_vector(64-1 downto 0) := (others => '0'); -- Accumulated differences (dl - ts)
  signal s_ctrl_diff_min_i    : std_logic_vector(64-1 downto 0) := (others => '0'); -- Minimum difference
  signal s_ctrl_diff_max_i    : std_logic_vector(64-1 downto 0) := (others => '0'); -- Maximum difference
  signal s_ctrl_cnt_late_i    : std_logic_vector(32-1 downto 0) := (others => '0'); -- Late Message Count
  signal s_ctrl_offset_late_o  : std_logic_vector(32-1 downto 0) := (others => '0'); -- Offset on difference. Controls condition for Late Message Counter increment


begin

  tap_out_o <= tap_in_i;
  tap_in_o  <= tap_out_i;

instOn: if g_build_tap = TRUE generate

   gaf : generic_async_fifo
   generic map (
     g_data_width => time_ref_i'length,
     g_size       => 8,
     g_show_ahead => true,

     g_with_rd_empty        => true,
     g_with_rd_full         => false,
     g_with_rd_almost_empty => false,
     g_with_rd_almost_full  => false,
     g_with_rd_count        => false,

     g_with_wr_empty        => false,
     g_with_wr_full         => true,
     g_with_wr_almost_empty => false,
     g_with_wr_almost_full  => false,
     g_with_wr_count        => false
     )
   port map (
     rst_n_i           => rst_ref_n_i,
     clk_wr_i          => clk_ref_i,
     d_i               => time_ref_i,
     we_i              => r_push,
     wr_full_o         => s_full,
     clk_rd_i          => clk_sys_i,
     q_o               => s_time_sys,
     rd_i              => s_pop,
     rd_empty_o        => s_empty);
 

  s_pop <= not s_empty;

  tsin : process(clk_ref_i)
  begin
    if rising_edge(clk_ref_i) then
        if(rst_ref_n_i = '0') then
          r_push <= '0';
        else
          r_push <= not r_push and not s_full; -- ref clk / 2
        end if;
    end if;
  end process;





--WB IF
  INST_eca_tap_auto : eca_tap_auto
  port map (
    clk_sys_i     => clk_sys_i,
    rst_sys_n_i   => rst_sys_n_i,
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

    cnt_late_i    => s_ctrl_cnt_late_i, 
    cnt_late_V_i  => s_valid,
    offset_late_o => s_ctrl_offset_late_o,
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

  s_diff    <= s_dl - signed(s_time_sys);                         -- deadline diff

  --no protection against invalid timing messages, but that can come later
  s_inc_msg   <= r_cnt_word(r_cnt_word'left);                         -- word count underflow
  s_en        <= s_inc_msg;                                           -- enable signal

  word_cnt : process(clk_sys_i)
  begin
    if rising_edge(clk_sys_i) then
      if(rst_sys_n_i = '0' or s_en = '1') then -- reset also on underflow
        r_cnt_word <= to_unsigned(7, r_cnt_word'length);      
      else
        if s_push = '1' then
          r_cnt_word <= r_cnt_word - resize(unsigned(s_ctrl_capture_o), r_cnt_word'length);      
        end if; 
      end if;   
    end if;
  end process;

  -- Late Counter begin
  s_diff_neg         <= r_diff(63);                 -- if difference is negative (bit63 set), msg is definitely late.
  s_diff_gt_max_offs <= '1' when unsigned(r_diff(63 downto 32)) /= 0   -- if difference is greater 32 bits its greater than the offset, msg cannot be late.
                   else '0';     
  s_diff_lo_lt_offs  <= '1' when unsigned(r_diff(31 downto 0)) < unsigned(s_ctrl_offset_late_o) -- if difference low word is less than the offset, msg is possibly late
                   else '0';

  late_cnt : process(clk_sys_i)
  begin
    if rising_edge(clk_sys_i) then
      if(rst_sys_n_i = '0' or s_ctrl_reset_o = "1" or s_ctrl_clear_o(c_CNT_LATE) = '1' ) then -- reset also on underflow
        r_cnt_late  <= (others => '0');
        r_is_late   <= "0";
      else
        r_en1 <= r_en0; 
        if r_en0 = '1' then -- r_diff is valid on r_en0
          r_is_late(0) <= s_diff_neg or (not s_diff_gt_max_offs and s_diff_lo_lt_offs); -- if diff is negative or fits in 32b and is less than the offset, the message was late.
        end if;
        if r_en1 = '1' then -- r_is_late is valid on r_en1
          r_cnt_late <= r_cnt_late + resize(unsigned(r_is_late), r_cnt_late'length); -- add r_is_late to the late counter
        end if;
        
      end if;   
    end if;
  end process;

  -- Late Counter end

  main : process(clk_sys_i)
  begin
    if rising_edge(clk_sys_i) then
      
      r_dl_hi <= r_dl_lo; 
      
     if(rst_sys_n_i = '0' or s_ctrl_reset_o = "1") then
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




