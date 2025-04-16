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
    cr_clk_o   : out   std_logic;
    cr_addr_o  : out   std_logic_vector(g_bits-1 downto 0);
    cr_data_io : inout std_logic_vector(15 downto 0);
    cr_ubn_o   : out   std_logic;
    cr_lbn_o   : out   std_logic;
    cr_cen_o   : out   std_logic;
    cr_oen_o   : out   std_logic;
    cr_wen_o   : out   std_logic;
    cr_cre_o   : out   std_logic;
    cr_advn_o  : out   std_logic;
    cr_wait_i  : in    std_logic);
end entity;

architecture rtl of cellular_ram is

  type t_state      is (S_INITIAL, S_IDLE, S_READ, S_WRITE);
  type t_state_word is (S_LOW_WORD, S_PREP_NEXT_WORD, S_HIGH_WORD);
  signal r_state      : t_state := S_INITIAL;
  signal r_state_word : t_state_word := S_LOW_WORD;
  signal r_ram_out    : t_cellular_ram_out;
  signal r_ack        : std_logic := '0';
  constant c_trc      : natural := 5;
  constant c_tcem     : natural := 5;
  signal r_counter_r  : unsigned(f_ceil_log2(c_trc+1)-1 downto 0) := (others => '0');
  signal r_counter_w  : unsigned(f_ceil_log2(c_trc+1)-1 downto 0) := (others => '0');

begin

  -- Wishbone
  slave_o.stall <= not(r_ack);
  slave_o.ack   <= r_ack;

  -- Unused Wishbone signals
  slave_o.rty <= '0';
  slave_o.err <= '0';

 -- Cellular RAM
  cr_cre_o  <= r_ram_out.cre;
  cr_oen_o  <= r_ram_out.oen;
  cr_wen_o  <= r_ram_out.wen;
  cr_cen_o  <= r_ram_out.cen;
  cr_ubn_o  <= r_ram_out.ubn;
  cr_lbn_o  <= r_ram_out.lbn;

  -- Unused cellular RAM
  cr_clk_o  <= '0';
  cr_advn_o <= '0';

  p_wishbone_handler : process(clk_i, rstn_i) is
  begin
    if rstn_i = '0' then
      -- Wishbone
      r_ack        <= '0';
      slave_o.dat  <= (others => '0');
      -- Intenral state machine
      r_state      <= S_INITIAL;
      r_state_word <= S_LOW_WORD;
      r_counter_r  <= (others => '0');
      r_counter_w  <= (others => '0');
      -- Data and addr
      cr_data_io   <= (others => 'Z');
      cr_addr_o    <= (others => '0');
      r_ram_out    <= f_cellular_ram_set_standby;
    elsif rising_edge(clk_i) then
      case r_state is
       when S_INITIAL =>
         r_state      <= S_IDLE;
         r_state_word <= S_LOW_WORD;
         cr_data_io   <= (others => 'Z');
         cr_addr_o    <= (others => '0');
         r_ram_out    <= f_cellular_ram_set_standby;
         r_ack        <= '0';
       when S_IDLE =>
         if (slave_i.cyc and slave_i.stb) = '1' then
           if slave_i.we = '1' then
             r_state    <= S_WRITE;
             cr_addr_o  <= slave_i.adr(g_bits-1 downto 0);
             cr_data_io <= slave_i.dat(15 downto 0);
             r_ram_out  <= f_cellular_ram_set_write;
           else
             r_state    <= S_READ;
             cr_addr_o  <= slave_i.adr(g_bits-1 downto 0);
             cr_data_io <= (others => 'Z');
             r_ram_out  <= f_cellular_ram_set_read;
           end if;
           r_state_word <= S_LOW_WORD;
           r_ack        <= '0';
        else
          cr_data_io  <= (others => 'Z');
          cr_addr_o   <= (others => '0');
          r_ram_out   <= f_cellular_ram_set_standby;
          r_ack       <= '0';
          --slave_o.dat <= (others => '0');
        end if;
      when S_WRITE =>
        r_counter_w <= r_counter_w + 1;
        if r_counter_w = c_tcem then
          r_counter_w <= (others => '0');
          case r_state_word is
             when S_LOW_WORD =>
               r_state_word              <= S_PREP_NEXT_WORD;
               r_ram_out                 <= f_cellular_ram_set_standby;
               cr_addr_o                 <= std_logic_vector(unsigned(slave_i.adr(g_bits-1 downto 0)) + 2);
               cr_data_io                <= slave_i.dat(31 downto 16);
             when S_PREP_NEXT_WORD =>
               r_state_word              <= S_HIGH_WORD;
               r_ram_out                 <= f_cellular_ram_set_write;
             when S_HIGH_WORD =>
               r_state_word              <= S_LOW_WORD;
               r_ram_out                 <= f_cellular_ram_set_standby;
               r_ack                     <= '1';
               r_state                   <= S_INITIAL;
            end case;
          end if;
      when S_READ =>
        r_counter_r <= r_counter_r + 1;
        if r_counter_r = c_tcem then
          r_counter_r <= (others => '0');
          case r_state_word is
             when S_LOW_WORD =>
               r_state_word              <= S_PREP_NEXT_WORD;
               r_ram_out                 <= f_cellular_ram_set_standby;
               cr_addr_o                 <= std_logic_vector(unsigned(slave_i.adr(g_bits-1 downto 0)) + 2);
               slave_o.dat(15 downto 0)  <= cr_data_io;
             when S_PREP_NEXT_WORD =>
               r_state_word              <= S_HIGH_WORD;
               r_ram_out                 <= f_cellular_ram_set_read;
             when S_HIGH_WORD =>
               r_state_word              <= S_LOW_WORD;
               r_ram_out                 <= f_cellular_ram_set_standby;
               slave_o.dat(31 downto 16) <= cr_data_io;
               r_ack                     <= '1';
               r_state                   <= S_INITIAL;
          end case;
        end if;
      end case;
    end if;
  end process;

end rtl;
