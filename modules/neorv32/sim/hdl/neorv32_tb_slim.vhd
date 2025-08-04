library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neorv32;
use neorv32.neorv32_package.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;
use work.neorv32_shell_pkg.all;

entity neorv32_tb_slim is
end entity;

architecture rtl of neorv32_tb_slim is

  signal s_clk      : std_logic;
  signal s_rstn     : std_logic;
  signal s_gpio_out : std_logic_vector(31 downto 0);
  signal s_gpio_in  : std_logic_vector(31 downto 0);
  signal s_uart_out : std_logic := '0';

  signal s_dummy_slave_i  : t_wishbone_slave_in := cc_dummy_slave_in;
  signal s_dummy_slave_o  : t_wishbone_slave_out := cc_dummy_slave_out;
  signal s_dummy_master_i : t_wishbone_master_in;
  signal s_dummy_master_o : t_wishbone_master_out;
  signal s_dummy_dff_data : std_logic_vector(31 downto 0);

  constant g_low_phase  : time := 8 ns;
  constant g_high_phase : time := 8 ns;
  constant c_reset_time : time := (g_high_phase+g_low_phase)*10;

begin

  -- Generate clock
  p_clock : process
  begin
    s_clk <= '0';
    wait for g_low_phase;
    s_clk <= '1' and s_rstn;
    wait for g_high_phase;
  end process;

  -- Generate reset
  p_reset : process
  begin
    s_rstn <= '0';
    wait for c_reset_time;
    s_rstn <= '1';
    wait;
  end process;

  -- NEORV32 shell
  neorv32_shell_inst: neorv32_shell
  generic map (
    g_sdb_addr               => x"12345678",
    g_mem_wishbone_init_file => "../src/sw/sim/program.mif"
  )
  port map (
    clk_i      => s_clk,
    rstn_i     => s_rstn,
    rstn_ext_i => '1',
    slave_i    => s_dummy_slave_i,
    slave_o    => s_dummy_slave_o,
    master_i   => s_dummy_master_i,
    master_o   => s_dummy_master_o,
    gpio_o     => s_gpio_out,
    gpio_i     => s_gpio_in,
    uart_o     => s_uart_out
  );

  sim_rx_uart0: entity work.sim_uart_rx
  generic map (
    NAME => "uart0",
    FCLK => real(62500000),
    BAUD => real(921600)
  )
  port map (
    clk => s_clk,
    rxd => s_uart_out
  );

  -- Wishbone emulator
  process(s_clk, s_rstn)
  begin
    if s_rstn = '0' then
      s_dummy_master_i.ack   <= '0';
      s_dummy_master_i.err   <= '0';
      s_dummy_master_i.dat   <= (others => '0');
      s_dummy_master_i.rty   <= '0';
      s_dummy_master_i.stall <= '0';
      s_dummy_dff_data       <= (others => '0');
    elsif rising_edge(s_clk) then
      if (s_dummy_master_o.stb = '1' and s_dummy_master_o.cyc = '1' and s_dummy_master_o.we = '1') then
        s_dummy_dff_data <= s_dummy_master_o.dat;
      end if;
      s_dummy_master_i.dat <= s_dummy_dff_data;
       if (s_dummy_master_o.stb = '1' and s_dummy_master_o.stb = '1') then
         s_dummy_master_i.ack <= '1';
       else
         s_dummy_master_i.ack <= '0';
       end if;
    end if;
  end process;

end;
