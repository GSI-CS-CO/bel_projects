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

  type t_state is (S_INITIAL, S_IDLE, S_READ, S_WRITE);
  signal r_state     : t_state := S_INITIAL;
  signal r_ram_out   : t_cellular_ram_out;
  signal r_ack       : std_logic := '0';
  constant c_trc     : natural := 5;
  constant c_tcem    : natural := 5;
  signal r_counter_r : unsigned(f_ceil_log2(c_trc+1)-1 downto 0) := (others => '0');
  signal r_counter_w : unsigned(f_ceil_log2(c_trc+1)-1 downto 0) := (others => '0');

begin

  -- Wishbone
  slave_o.stall <= not(r_ack);
  slave_o.ack   <= r_ack;

  -- Unused Wishbone signals
  slave_o.rty <= '0';
  slave_o.err <= '0';

 -- Cellular RAM
  ps_cre_o  <= r_ram_out.cre;
  ps_oen_o  <= r_ram_out.oen;
  ps_wen_o  <= r_ram_out.wen;
  ps_cen_o  <= r_ram_out.cen;
  ps_ubn_o  <= r_ram_out.ubn;
  ps_lbn_o  <= r_ram_out.lbn;

  -- Unused cellular RAM
  ps_clk_o  <= '0';
  ps_advn_o <= '0';

  p_wishbone_handler : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      -- Wishbone
      r_ack       <= '0';
      slave_o.dat <= (others => '0');
      -- Intenral state machine
      r_state     <= S_INITIAL;
      r_counter_r <= (others => '0');
      r_counter_w <= (others => '0');
      -- Data and addr
      ps_data_io  <= (others => 'Z');
      ps_addr_o   <= (others => '0');
      r_ram_out   <= f_cellular_ram_set_standby;
    elsif rising_edge(clk_i) then
      case r_state is
       when S_INITIAL =>
         r_state      <= S_IDLE;
         ps_data_io   <= (others => 'Z');
         ps_addr_o    <= (others => '0');
         r_ram_out    <= f_cellular_ram_set_standby;
         r_ack        <= '0';
       when S_IDLE =>
         if (slave_i.cyc and slave_i.stb) = '1' then
           if slave_i.we = '1' then
             r_state    <= S_WRITE;
             ps_addr_o  <= slave_i.adr(g_bits-1 downto 0);
             ps_data_io <= slave_i.dat(15 downto 0);
             r_ram_out  <= f_cellular_ram_set_write;
           else
             r_state    <= S_READ;
             ps_addr_o  <= slave_i.adr(g_bits-1 downto 0);
             ps_data_io <= (others => 'Z');
             r_ram_out  <= f_cellular_ram_set_read;
           end if;
        else
          ps_data_io  <= (others => 'Z');
          ps_addr_o   <= (others => '0');
          r_ram_out   <= f_cellular_ram_set_standby;
          r_ack       <= '0';
          slave_o.dat <= (others => '0');
        end if;
      when S_WRITE =>
        r_counter_w <= r_counter_w + 1;
        if r_counter_w = c_trc then
          r_state     <= S_IDLE;
          r_counter_w <= (others => '0');
          r_ram_out   <= f_cellular_ram_set_standby;
          r_ack       <= '1';
        end if;
      when S_READ =>
      r_counter_r <= r_counter_r + 1;
        if r_counter_r = c_tcem then
          r_state                   <= S_IDLE;
          r_counter_r               <= (others => '0');
          r_ram_out                 <= f_cellular_ram_set_standby;
          r_ack                     <= '1';
          slave_o.dat(15 downto 0)  <= (ps_data_io);
          slave_o.dat(31 downto 16) <= (others => '0');
        end if;
      end case;
    end if;
  end process;

end rtl;
