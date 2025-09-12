library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neorv32;
use neorv32.neorv32_package.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;
use work.neorv32_shell_pkg.all;

entity neorv32_tb_xbar is
end entity;

architecture rtl of neorv32_tb_xbar is

  signal s_clk      : std_logic;
  signal s_rstn     : std_logic;
  signal s_gpio_out : std_logic_vector(31 downto 0);
  signal s_gpio_in  : std_logic_vector(31 downto 0);
  signal s_uart_out : std_logic := '0';

  signal s_dummy_slave_i  : t_wishbone_slave_in := cc_dummy_slave_in;
  signal s_dummy_slave_o  : t_wishbone_slave_out := cc_dummy_slave_out;
  signal s_dummy_dff_data : std_logic_vector(31 downto 0);

  constant g_low_phase  : time := 8 ns;
  constant g_high_phase : time := 8 ns;
  constant c_reset_time : time := (g_high_phase+g_low_phase)*10;

  signal cbar_slave_i  : t_wishbone_slave_in_array (0 downto 0);
  signal cbar_slave_o  : t_wishbone_slave_out_array(0 downto 0);
  signal cbar_master_i : t_wishbone_master_in_array(0 downto 0);
  signal cbar_master_o : t_wishbone_master_out_array(0 downto 0);

  constant RAM_SIZE : natural := 131072/4;

  constant c_layout : t_sdb_record_array(0 downto 0) :=
    (0 => f_sdb_embed_device(f_xwb_dpram(RAM_SIZE), x"00000000"));
  constant c_sdb_address : t_wishbone_address := x"00200000";

  -- constant c_master_layout : t_sdb_record_array(0 downto 0) :=
  --   (0 => f_sdb_auto_msi(c_null_msi,    false));

  -- constant c_layout      : t_sdb_record_array := f_sdb_auto_layout(c_master_layout, c_slave_layout);


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
    g_mem_wishbone_init_file => "../src/sw/sim_xbar/program.mif"
  )
  port map (
    clk_i      => s_clk,
    rstn_i     => s_rstn,
    rstn_ext_i => '1',
    slave_i    => s_dummy_slave_i,
    slave_o    => s_dummy_slave_o,
    master_i   => cbar_slave_o(0),
    master_o   => cbar_slave_i(0),
    uart_o     => s_uart_out,
    jtag_tck_i => '0',
    jtag_tdi_i => '0',
    jtag_tdo_o => open,
    jtag_tms_i => '0'
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

  WB_CON : xwb_sdb_crossbar
    generic map(
      g_num_masters => 1,
      g_num_slaves  => 1,
      g_registered  => true,
      g_wraparound  => true,
      g_layout      => c_layout,
      g_sdb_wb_mode => PIPELINED,
      g_sdb_addr    => c_sdb_address
      )
    port map(
      clk_sys_i => s_clk,
      rst_n_i   => s_rstn,
      -- Master connections (INTERCON is a slave)
      slave_i   => cbar_slave_i,
      slave_o   => cbar_slave_o,
      -- Slave connections (INTERCON is a master)
      master_i  => cbar_master_i,
      master_o  => cbar_master_o
      );

  RAM : xwb_dpram
    generic map(
      g_size                  => RAM_SIZE,
      g_slave1_interface_mode => PIPELINED,
      g_slave2_interface_mode => PIPELINED,
      g_slave1_granularity    => BYTE,
      g_slave2_granularity    => BYTE)
    port map(
      clk_sys_i => s_clk,
      rst_n_i   => s_rstn,
      -- First port connected to the crossbar
      slave1_i  => cbar_master_o(0),
      slave1_o  => cbar_master_i(0),
      -- Second port disconnected
      slave2_i  => cc_dummy_slave_in, -- CYC always low
      slave2_o  => open);
end;
