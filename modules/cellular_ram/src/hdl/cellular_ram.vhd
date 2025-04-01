library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

entity cellular_ram is
  generic(
    g_bits     : natural := 24;
    g_row_bits : natural := 8);
  port(
    clk_i     : in    std_logic;
    rstn_i    : in    std_logic;
    slave_i   : in    t_wishbone_slave_in;
    slave_o   : out   t_wishbone_slave_out;
    ps_clk    : out   std_logic;
    ps_addr   : out   std_logic_vector(g_bits-1 downto 0);
    ps_data   : inout std_logic_vector(15 downto 0);
    ps_seln   : out   std_logic_vector(1 downto 0);
    ps_cen    : out   std_logic;
    ps_oen    : out   std_logic;
    ps_wen    : out   std_logic;
    ps_cre    : out   std_logic;
    ps_advn   : out   std_logic;
    ps_wait   : in    std_logic);
end entity;

-- !!! does not respect tCEM burst-limit

architecture rtl of cellular_ram is

  type t_state is (S_INITIAL, S_CHECK_VERSION, S_RESET, S_IDLE, S_WRITE_REQUEST, S_WRITE_WAIT, S_WRITE_LATCH, S_READ_REQUEST, S_READ_WAIT, S_READ_LATCH, S_BCR_REQUEST, S_BCR_WAIT, S_BCR_GAP, S_BCR_READ);
  signal r_state   : t_state := S_INITIAL;

begin

  ps_clk        <= '0';
  ps_advn       <= '0';
  slave_o.err   <= '0';
  slave_o.rty   <= '0';
  slave_o.stall <= '0';

  fsm : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      r_state <= S_INITIAL;
    elsif rising_edge(clk_i) then
      ps_cre <= '1';
    end if;
  end process;


end rtl;
