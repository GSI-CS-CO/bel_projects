library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.cellular_ram_pkg.all;

entity cellular_ram is
  generic(
    g_bits     : natural := 24);
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

  type t_state is (S_INITIAL, S_READ, S_WRITE);
  signal r_state   : t_state := S_INITIAL;
  signal r_ram_out : t_cellular_ram_out;
  signal r_ack     : std_logic := '0';

begin

  -- Wishbone
  slave_o.stall <= not(r_ack);
  slave_o.rty   <= '0';

 -- Cellular RAM
  ps_clk_o      <= '0';
  ps_advn_o     <= '0';
  ps_cre_o      <= r_ram_out.cre;
  ps_oen_o      <= r_ram_out.oen;
  ps_wen_o      <= r_ram_out.wen;
  ps_cen_o      <= r_ram_out.cen;
  ps_ubn_o      <= r_ram_out.ubn;
  ps_lbn_o      <= r_ram_out.lbn;

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
      -- Data and addr
      ps_data_io    <= (others => '0'); -- X
      ps_addr_o     <= (others => '0');
      r_ram_out     <= f_cellular_ram_set_idle;
    elsif rising_edge(clk_i) then


    end if;
  end process;

end rtl;
