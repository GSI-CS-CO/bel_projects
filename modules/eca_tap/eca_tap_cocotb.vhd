--! @file        eca_tap.xml
--  DesignUnit   eca_tap
--! @author      M. Kreider <m.kreider@gsi.de>
--! @date        17/02/2017
--! @version     0.0.1
--! @copyright   2017 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!

--! @brief Interface to Cocotb Framework for eca_tap
--!
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.wbgenplus_pkg.all;
use work.genram_pkg.all;
use work.eca_tap_auto_pkg.all;

entity eca_tap_cocotb is  port (
    clk        : in  std_logic;
    reset_n    : in  std_logic;
    clk2       : in  std_logic;
    reset_n2   : in  std_logic;
    wbs_cyc    : in  std_logic;
    wbs_stb    : in  std_logic;
    wbs_we     : in  std_logic;
    wbs_sel    : in  std_logic_vector(3 downto 0);
    wbs_adr    : in  std_logic_vector(31 downto 0);
    wbs_datrd  : out std_logic_vector(31 downto 0);
    wbs_datwr  : in  std_logic_vector(31 downto 0);
    wbs_stall  : out std_logic;
    wbs_ack    : out std_logic;
    wbs_err    : out std_logic
  );
end entity;
architecture rtl of eca_tap_cocotb is

  signal s_slave_in   : t_wishbone_slave_in;
  signal s_slave_out  : t_wishbone_slave_out;
  signal s_master_in  : t_wishbone_master_in;
  signal s_master_out : t_wishbone_master_out;
  signal s_ctrl_error_i       : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Error control
  signal s_ctrl_stall_i       : std_logic_vector(1-1 downto 0)  := (others => '0'); -- flow control
  signal s_ctrl_reset_o       : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Resets ECA-Tap
  signal s_ctrl_clear_o       : std_logic_vector(3-1 downto 0)  := (others => '0'); -- b2: clear count/accu, b1: clear max, b0: clear min
  signal s_ctrl_cnt_msg_V_i   : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Valid flag - cnt_msg
  signal s_ctrl_cnt_msg_i     : std_logic_vector(64-1 downto 0) := (others => '0'); -- Message Count
  signal s_ctrl_diff_acc_V_i  : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Valid flag - diff_acc
  signal s_ctrl_diff_acc_i    : std_logic_vector(64-1 downto 0) := (others => '0'); -- Accumulated differences (dl - ts)
  signal s_ctrl_diff_min_V_i  : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Valid flag - diff_min
  signal s_ctrl_diff_min_i    : std_logic_vector(64-1 downto 0) := (others => '0'); -- Minimum difference
  signal s_ctrl_diff_max_V_i  : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Valid flag - diff_max
  signal s_ctrl_diff_max_i    : std_logic_vector(64-1 downto 0) := (others => '0'); -- Maximum difference
  


begin

    s_ctrl_in.we   <= wbs_we;
    s_ctrl_in.stb  <= wbs_stb;
    s_ctrl_in.dat  <= wbs_datwr;
    s_ctrl_in.adr  <= wbs_adr;
    s_ctrl_in.sel  <= x"f";
    s_ctrl_in.cyc  <= wbs_cyc;
    wbs_datrd    <= s_ctrl_out.dat;
    wbs_ack      <= s_ctrl_out.ack;
    wbs_stall    <= s_ctrl_out.stall;
    wbs_err      <= s_ctrl_out.err;
  
  INST_eca_tap_auto : eca_tap_auto
  port map (
    clk_sys_i     => clk,
    rst_sys_n_i   => reset_n,
    error_i       => s_ctrl_error_i,
    stall_i       => s_ctrl_stall_i,
    reset_o       => s_ctrl_reset_o,
    clear_o       => s_ctrl_clear_o,
    cnt_msg_V_i   => s_ctrl_cnt_msg_V_i,
    cnt_msg_i     => s_ctrl_cnt_msg_i,
    diff_acc_V_i  => s_ctrl_diff_acc_V_i,
    diff_acc_i    => s_ctrl_diff_acc_i,
    diff_min_V_i  => s_ctrl_diff_min_V_i,
    diff_min_i    => s_ctrl_diff_min_i,
    diff_max_V_i  => s_ctrl_diff_max_V_i,
    diff_max_i    => s_ctrl_diff_max_i,
    ctrl_i        => s_slave_in,
    ctrl_o        => s_slave_out  );
  
end rtl;
