library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;
use work.neorv32_shell_pkg.all;

library neorv32;
use neorv32.neorv32_package.all;

entity neorv32_shell is
  generic (
    g_clock_frequency           : natural := 62500000;                           -- clock frequency of clk_i in Hz
    g_sdb_addr                  : t_wishbone_address := x"00001000";             -- base SDB record
    g_mem_wishbone_imem_size    : natural := 4*8000;                             -- memory size in bytes
    g_mem_wishbone_imem_addr    : std_ulogic_vector(31 downto 0) := x"71000000"; -- imem RAM start address
    g_mem_wishbone_init_file    : string := "";                                  -- init file Wishbone instruction memory
    g_mem_int_imem_size         : natural := 8*1024;                             -- size of processor-internal instruction memory in bytes
    g_mem_int_dmem_size         : natural := 8*1024                              -- size of processor-internal data memory in bytes
  );
  port (
    -- Global control
    clk_i      : in std_logic;
    rstn_i     : in std_logic;
    rstn_ext_i : in std_logic;
    -- Peripherals
    gpio_o     : out std_logic_vector(31 downto 0);
    gpio_i     : in  std_logic_vector(31 downto 0) := (others => '0');
    uart_o     : out std_logic;
    -- Wishbone
    slave_i    : in  t_wishbone_slave_in;
    slave_o    : out t_wishbone_slave_out;
    master_i   : in  t_wishbone_master_in;
    master_o   : out t_wishbone_master_out);
end neorv32_shell;

architecture rtl of neorv32_shell is

  constant c_wb_imem_base    : unsigned(31 downto 0) := unsigned(g_mem_wishbone_imem_addr);
  constant c_wb_imem_size    : unsigned(31 downto 0) := to_unsigned(g_mem_wishbone_imem_size, 32);
  constant c_wb_imem_end     : unsigned(31 downto 0) := c_wb_imem_base + c_wb_imem_size;
  constant c_stack_start_slv : std_logic_vector(31 downto 0) := x"80000000"; -- NEORV32 fixed address, see doc
  constant c_stack_start     : unsigned(31 downto 0) := unsigned(c_stack_start_slv);

  constant BOOT_ADDR_CUSTOM_C : std_ulogic_vector(31 downto 0) := g_mem_wishbone_imem_addr;

  signal s_xbus_adr     : std_ulogic_vector(31 downto 0);
  signal s_xbus_dat_out : std_ulogic_vector(31 downto 0);
  signal s_xbus_tag     : std_ulogic_vector(2 downto 0);
  signal s_xbus_we      : std_ulogic;
  signal s_xbus_sel     : std_ulogic_vector(3 downto 0);
  signal s_xbus_stb     : std_ulogic;
  signal s_xbus_cyc     : std_ulogic;
  signal s_xbus_dat_in  : std_ulogic_vector(31 downto 0);
  signal s_xbus_ack     : std_ulogic;
  signal s_xbus_err     : std_ulogic;

  signal s_gpio_out     : std_ulogic_vector(31 downto 0);
  signal s_gpio_in      : std_ulogic_vector(31 downto 0);

  signal s_wb_ram_neorv32_i : t_wishbone_slave_in;
  signal s_wb_ram_neorv32_o : t_wishbone_slave_out;

  signal s_resetn_com   : std_ulogic;

  signal addr : std_logic_vector(31 downto 0) := (others => '0');
  signal comp : std_logic_vector(31 downto 0) := (others => '0');

  signal s_instruction  : std_logic;

  attribute keep                 : boolean;
  signal wb_imem_addr            : std_logic := '0';
  attribute keep of wb_imem_addr : signal is true;

begin

  -- NEORV32 CPU
  neorv32_top_inst : neorv32_top
  generic map (
    CLOCK_FREQUENCY   => g_clock_frequency,
    BOOT_MODE_SELECT  => 1,
    XBUS_EN           => true,
    --BOOT_ADDR_CUSTOM  => std_ulogic_vector(unsigned(g_mem_wishbone_imem_addr)),
    --BOOT_ADDR_CUSTOM => std_ulogic_vector(to_unsigned(g_mem_wishbone_imem_addr, 32)),
    BOOT_ADDR_CUSTOM  => BOOT_ADDR_CUSTOM_C, -- Keep Quartus happy
    MEM_INT_IMEM_EN   => false,
    MEM_INT_IMEM_SIZE => g_mem_int_imem_size,
    MEM_INT_DMEM_EN   => true,
    MEM_INT_DMEM_SIZE => g_mem_int_dmem_size,
    IO_GPIO_NUM       => 32,
    IO_UART0_EN       => true
  )
  port map (
    clk_i       => clk_i,
    rstn_i      => s_resetn_com,
    xbus_adr_o  => s_xbus_adr,
    xbus_dat_o  => s_xbus_dat_out,
    xbus_tag_o  => s_xbus_tag,
    xbus_we_o   => s_xbus_we,
    xbus_sel_o  => s_xbus_sel,
    xbus_stb_o  => s_xbus_stb,
    xbus_cyc_o  => s_xbus_cyc,
    xbus_dat_i  => s_xbus_dat_in,
    xbus_ack_i  => s_xbus_ack,
    xbus_err_i  => s_xbus_err,
    gpio_o      => s_gpio_out,
    gpio_i      => s_gpio_in,
    uart0_txd_o => uart_o
  );

  -- Reset logic
  s_resetn_com <= rstn_i and rstn_ext_i;

  -- Report IMEM address range
  wb_imem_addr <= f_neorv32_report_wb_range(g_mem_wishbone_imem_addr, g_sdb_addr);

  -- SDB magic
  process(clk_i, rstn_i)
  begin
    if rstn_i = '0' then
      s_gpio_in <= (others => '0');
    elsif rising_edge(clk_i) then
      s_gpio_in <= std_ulogic_vector(unsigned(g_sdb_addr));
    end if;
  end process;

  -- GPIOs
  gpio_o <= std_logic_vector(s_gpio_out);

  -- Wishbone RAM
  neorv32_wb_ext_ram : xwb_dpram
    generic map(
      g_size                  => g_mem_wishbone_imem_size,
      g_init_file             => g_mem_wishbone_init_file,
      g_must_have_init_file   => false,
      g_slave1_interface_mode => PIPELINED,
      g_slave2_interface_mode => PIPELINED,
      g_slave1_granularity    => BYTE,
      g_slave2_granularity    => BYTE)
    port map (
      clk_sys_i => clk_i,
      rst_n_i   => rstn_i,
      slave1_i  => s_wb_ram_neorv32_i,
      slave1_o  => s_wb_ram_neorv32_o,
      slave2_i  => slave_i,
      slave2_o  => slave_o);

  s_wb_ram_neorv32_i.cyc <= s_xbus_cyc;
  s_wb_ram_neorv32_i.stb <= s_xbus_stb;
  s_wb_ram_neorv32_i.adr <= std_logic_vector(s_xbus_adr);
  s_wb_ram_neorv32_i.sel <= std_logic_vector(s_xbus_sel);
  s_wb_ram_neorv32_i.we  <= s_xbus_we;
  s_wb_ram_neorv32_i.dat <= std_logic_vector(s_xbus_dat_out);

  master_o.cyc <= s_xbus_cyc and not(s_instruction);
  master_o.stb <= s_xbus_stb and not(s_instruction);
  master_o.adr <= std_logic_vector(s_xbus_adr);
  master_o.sel <= std_logic_vector(s_xbus_sel);
  master_o.we  <= s_xbus_we and not(s_instruction);
  master_o.dat <= std_logic_vector(s_xbus_dat_out);

  -- Wishbone arbiter (read from own RAM or access the whole Wishbone space)
  s_instruction <= '1' when
    ((unsigned(s_xbus_adr) >= c_wb_imem_base) and
     (unsigned(s_xbus_adr) < c_wb_imem_end)) or
     (unsigned(s_xbus_adr) >= c_stack_start)
  else '0';

  s_xbus_dat_in <= std_ulogic_vector(s_wb_ram_neorv32_o.dat) when (s_instruction = '1') else std_ulogic_vector(master_i.dat);
  s_xbus_ack    <= s_wb_ram_neorv32_o.ack when (s_instruction = '1') else std_ulogic(master_i.ack);
  s_xbus_err    <= s_wb_ram_neorv32_o.err when (s_instruction = '1') else std_ulogic(master_i.err);

end rtl;
