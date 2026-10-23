library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

--------------------------------------------------------------------------------
-- vuart_fifo: FIFO-only virtual UART wishbone slave (no serialization)
-- A core (e.g. LM32) writes console bytes into a FIFO, an external
-- (Etherbone) host reads them out; host writes and core reads carry no data.
-- Use with c_vuart_sdb so eb-console.c finds it. Register map (big endian):
--   core side, compatible with wb_simple_uart (wrpc-import/uart.c):
--     SR  @ base+0x00: read = 0 (never TX busy, no RX)
--     BCR @ base+0x04: ignored
--     TDR @ base+0x08: write = push byte (dropped if the FIFO is full)
--   host side, compatible with eb-console.c:
--     TX  @ base+0x10: read = {bit8:ready}, write ignored
--     RX  @ base+0x14: read = {bit8:ready, bit7..0:byte}, pops the FIFO
--------------------------------------------------------------------------------
entity vuart_fifo is
  generic (
    g_size : natural := 1024);  -- FIFO depth in bytes
  port (
    clk_i   : in  std_logic;
    rst_n_i : in  std_logic;
    slave_i : in  t_wishbone_slave_in;
    slave_o : out t_wishbone_slave_out);
end entity vuart_fifo;

architecture rtl of vuart_fifo is

  signal r_ack      : std_logic := '0';
  signal r_dat      : t_wishbone_data := (others => '0');

  signal s_fifo_q   : std_logic_vector(7 downto 0);
  signal s_fifo_emp : std_logic;
  signal s_fifo_wr  : std_logic;
  signal s_fifo_rd  : std_logic;
  signal s_access   : std_logic;

begin

  slave_o.ack   <= r_ack;
  slave_o.dat   <= r_dat;
  slave_o.err   <= '0';
  slave_o.stall <= '0';
  slave_o.rty   <= '0';

  s_access <= slave_i.cyc and slave_i.stb;

  vuart_fifo_wb : process(clk_i, rst_n_i)
  begin
    if rst_n_i = '0' then
      r_ack <= '0';
      r_dat <= (others => '0');
    elsif rising_edge(clk_i) then
      r_ack <= s_access;
      r_dat <= (others => '0');
      if s_access = '1' and slave_i.we = '0' then
        case slave_i.adr(4 downto 0) is
          when "10000" => -- 0x10: TX (read = {bit8:ready})
            r_dat(8) <= '1';
          when "10100" => -- 0x14: RX (read = {ready, byte}, pops the FIFO)
            if s_fifo_emp = '0' then
              r_dat(8) <= '1';
              r_dat(7 downto 0) <= s_fifo_q;
            end if;
          when others => null; -- SR/BCR/TDR read as 0
        end case;
      end if;
    end if;
  end process;

  -- Combinational push/pop: back-to-back accesses never lose or repeat a byte.
  -- A full FIFO drops the byte (generic_sync_fifo ignores we_i when full).
  s_fifo_wr <= '1' when s_access = '1' and slave_i.we = '1' and slave_i.adr(4 downto 0) = "01000" else '0';
  s_fifo_rd <= '1' when s_access = '1' and slave_i.we = '0' and slave_i.adr(4 downto 0) = "10100" and s_fifo_emp = '0' else '0';

  U_VUART_FIFO : entity work.generic_sync_fifo
    generic map (
      g_data_width => 8,
      g_size       => g_size,
      g_show_ahead => true)
    port map (
      rst_n_i        => rst_n_i,
      clk_i          => clk_i,
      d_i            => slave_i.dat(7 downto 0),
      we_i           => s_fifo_wr,
      q_o            => s_fifo_q,
      rd_i           => s_fifo_rd,
      empty_o        => s_fifo_emp,
      full_o         => open,
      almost_empty_o => open,
      almost_full_o  => open,
      count_o        => open);

end architecture rtl;
