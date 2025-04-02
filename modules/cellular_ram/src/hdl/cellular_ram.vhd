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
    clk_i      : in    std_logic;
    rstn_i     : in    std_logic;
    slave_i    : in    t_wishbone_slave_in;
    slave_o    : out   t_wishbone_slave_out;
    ps_clk_o   : out   std_logic;
    ps_addr_o  : out   std_logic_vector(g_bits-1 downto 0);
    ps_data_io : inout std_logic_vector(15 downto 0);
    ps_ubn_o   : out   std_logic;
    ps_lbn_o   : out   std_logic;
    ps_cen_o   : out   std_logic;
    ps_oen_o   : out   std_logic;
    ps_wen_o   : out   std_logic;
    ps_cre_o   : out   std_logic;
    ps_advn_o  : out   std_logic;
    ps_wait_i  : in    std_logic);
end entity;

architecture rtl of cellular_ram is

  type t_state is (S_INITIAL, S_CHECK_VERSION, S_RESET, S_IDLE, S_WRITE_REQUEST, S_WRITE_WAIT, S_WRITE_LATCH, S_READ_REQUEST, S_READ_WAIT, S_READ_LATCH, S_BCR_REQUEST, S_BCR_WAIT, S_BCR_GAP, S_BCR_READ);
  signal r_state : t_state := S_INITIAL;

  signal r_ack : std_logic := '0';

begin

  ps_clk_o      <= '0';
  ps_advn_o     <= '0';
  slave_o.stall <= not(r_ack);
  slave_o.rty   <= '0';

  p_wishbone_handler : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      -- Wishbone
      r_ack         <= '0';
      slave_o.ack   <= '0';
      slave_o.err   <= '0';
      slave_o.dat   <= (others => '0');
      -- Intenral state machine
      r_state       <= S_INITIAL;
      -- IOs - idle mode
      ps_cre_o      <= '0';
      ps_oen_o      <= '0'; -- X
      ps_wen_o      <= '0'; -- X
      ps_cre_o      <= '0';
      ps_ubn_o      <= '0';
      ps_lbn_o      <= '0'; -- X
      ps_data_io    <= (others => '0');
      ps_addr_o     <= (others => '0');
    elsif rising_edge(clk_i) then


    end if;
  end process;

end rtl;
