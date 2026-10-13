library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

--------------------------------------------------------------------------------
-- vuart: serial virtual UART wishbone slave
-- Bridges an external (Etherbone) host to a core's physical console UART so
-- that tools/eb-console.c can talk to that core. Register map (big endian,
-- matches eb-console.c): TX @ base+0x10 (write = send byte, read = tx ready),
-- RX @ base+0x14 (read = {bit8:ready, bit7..0:byte}, pops the RX FIFO).
--------------------------------------------------------------------------------
entity vuart is
  generic (
    g_baud : natural := 966);  -- SUART_CALC_BAUD value for the core console
  port (
    clk_i      : in  std_logic;
    rst_n_i    : in  std_logic;
    slave_i    : in  t_wishbone_slave_in;
    slave_o    : out t_wishbone_slave_out;
    uart_rxd_i : in  std_logic := '1';  -- core console TX (serial in)
    uart_txd_o : out std_logic);        -- drives core console RX (idle high)
end entity vuart;

architecture rtl of vuart is

  signal r_ack     : std_logic := '0';
  signal r_dat     : t_wishbone_data := (others => '0');
  signal r_tx_trig : std_logic := '0';
  signal r_tx_data : std_logic_vector(7 downto 0) := (others => '0');

  signal s_baud    : std_logic;
  signal s_baud8   : std_logic;
  signal s_rx_data : std_logic_vector(7 downto 0);
  signal s_rx_ready: std_logic;
  signal s_tx_busy : std_logic;

  signal s_fifo_q   : std_logic_vector(7 downto 0);
  signal s_fifo_emp : std_logic;
  signal s_fifo_full: std_logic;
  signal s_fifo_wr  : std_logic;
  signal s_fifo_rd  : std_logic;
  signal s_rx_match : std_logic;

begin

  slave_o.ack   <= r_ack;
  slave_o.dat   <= r_dat;
  slave_o.err   <= '0';
  slave_o.stall <= '0';
  slave_o.rty   <= '0';

  vuart_wb : process(clk_i, rst_n_i)
  begin
    if rst_n_i = '0' then
      r_ack     <= '0';
      r_dat     <= (others => '0');
      r_tx_trig <= '0';
      r_tx_data <= (others => '0');
    elsif rising_edge(clk_i) then
      r_ack     <= slave_i.cyc and slave_i.stb;
      r_dat     <= (others => '0');
      r_tx_trig <= '0';
      if slave_i.cyc = '1' and slave_i.stb = '1' then
        case slave_i.adr(4 downto 0) is
          when "10000" => -- 0x10: TX (write = send byte, read = {bit8:ready})
            if slave_i.we = '1' then
              r_tx_trig <= '1';
              r_tx_data <= slave_i.dat(7 downto 0);
            else
              r_dat(8) <= not s_tx_busy and not r_tx_trig;
            end if;
          when "10100" => -- 0x14: RX (read = {ready, byte}, pops the FIFO)
            if slave_i.we = '0' then
              r_dat(8) <= not s_fifo_emp;
              r_dat(7 downto 0) <= s_fifo_q;
            end if;
          when others => null;
        end case;
      end if;
    end if;
  end process;

  -- Combinational pop: advances the show-ahead head in time for back-to-back
  -- reads so a byte is never returned twice.
  s_rx_match <= '1' when slave_i.adr(4 downto 0) = "10100" else '0';
  s_fifo_rd  <= slave_i.cyc and slave_i.stb and not slave_i.we and s_rx_match and not s_fifo_emp;

  U_VUART_BAUD : entity work.uart_baud_gen
    generic map (g_baud_acc_width => 16)
    port map (
      clk_sys_i    => clk_i,
      rst_n_i      => rst_n_i,
      baudrate_i   => std_logic_vector(to_unsigned(g_baud, 17)),
      baud_tick_o  => s_baud,
      baud8_tick_o => s_baud8);

  U_VUART_RX : entity work.uart_async_rx
    port map (
      clk_sys_i    => clk_i,
      rst_n_i      => rst_n_i,
      baud8_tick_i => s_baud8,
      rxd_i        => uart_rxd_i,
      rx_ready_o   => s_rx_ready,
      rx_error_o   => open,
      rx_data_o    => s_rx_data);

  s_fifo_wr <= s_rx_ready and not s_fifo_full;

  U_VUART_FIFO : entity work.generic_sync_fifo
    generic map (
      g_data_width => 8,
      g_size       => 1024,
      g_show_ahead => true)
    port map (
      rst_n_i        => rst_n_i,
      clk_i          => clk_i,
      d_i            => s_rx_data,
      we_i           => s_fifo_wr,
      q_o            => s_fifo_q,
      rd_i           => s_fifo_rd,
      empty_o        => s_fifo_emp,
      full_o         => s_fifo_full,
      almost_empty_o => open,
      almost_full_o  => open,
      count_o        => open);

  U_VUART_TX : entity work.uart_async_tx
    port map (
      clk_sys_i    => clk_i,
      rst_n_i      => rst_n_i,
      baud_tick_i  => s_baud,
      txd_o        => uart_txd_o,
      tx_start_p_i => r_tx_trig,
      tx_data_i    => r_tx_data,
      tx_busy_o    => s_tx_busy);

end architecture rtl;
