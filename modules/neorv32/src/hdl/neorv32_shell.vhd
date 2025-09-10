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
    g_mem_int_imem_size         : natural := 16*1024;                            -- size of processor-internal instruction memory in bytes
    g_mem_int_dmem_size         : natural := 16*1024;                            -- size of processor-internal data memory in bytes
    g_use_wb_adapter            : boolean := false;                              -- use wishbone slave adapter CLASSIC/PIPELINED
    g_en_debugging              : boolean := false                               -- enable OCD debugging
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
    master_o   : out t_wishbone_master_out;
    jtag_tck_i : in std_logic;
    jtag_tdi_i : in std_logic;
    jtag_tdo_o : out std_logic;
    jtag_tms_i : in std_logic);
end neorv32_shell;

architecture rtl of neorv32_shell is

  -- helper function to save the whole transfer request in one FIFO
  function to_vector(i : t_wishbone_master_out) return std_logic_vector is
  variable v : std_logic_vector(c_wishbone_data_width + 1 + (c_wishbone_address_width/8) + c_wishbone_address_width + 1 downto 0);
  begin
    v(c_wishbone_data_width + 1 + (c_wishbone_address_width/8) + c_wishbone_address_width + 1)                                                                  := i.cyc;
    v(c_wishbone_data_width + 1 + (c_wishbone_address_width/8) + c_wishbone_address_width)                                                                      := i.stb;
    v(c_wishbone_data_width + 1 + (c_wishbone_address_width/8) + c_wishbone_address_width - 1 downto c_wishbone_data_width + 1 + (c_wishbone_address_width/8))  := i.adr;
    v(c_wishbone_data_width + 1 + (c_wishbone_address_width/8) - 1 downto c_wishbone_address_width + 1)                                                         := i.sel;
    v(c_wishbone_address_width)                                                                                                                                 := i.we;
    v(c_wishbone_data_width-1 downto 0)                                                                                                                         := i.dat;
    return v;
  end function;

  -- helper function to save the whole transfer request in one FIFO
  function to_master_out(i : std_logic_vector) return t_wishbone_master_out is
  variable v : t_wishbone_master_out;
  begin
    v.cyc := '1';
    v.stb := i(c_wishbone_data_width + 1 + (c_wishbone_address_width/8) + c_wishbone_address_width + 1);
    v.adr := i(c_wishbone_data_width + 1 + (c_wishbone_address_width/8) + c_wishbone_address_width - 1 downto c_wishbone_data_width + 1 + (c_wishbone_address_width/8));
    v.sel := i(c_wishbone_data_width + 1 + (c_wishbone_address_width/8) - 1 downto c_wishbone_address_width + 1);
    v.we  := i(c_wishbone_address_width);
    v.dat := i(c_wishbone_data_width-1 downto 0);
    return v;
  end function;

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

  signal s_cfs_in       : std_ulogic_vector(32 downto 0);
  signal s_cfs_out      : std_ulogic_vector(0 downto 0);

  signal s_burst_dummy_master : std_logic_vector(1 + 1 + c_wishbone_address_width + (c_wishbone_address_width/8) + 1 + c_wishbone_data_width - 1 downto 0) := (others => '0');

  signal s_wb_ram_neorv32_i : t_wishbone_slave_in;
  signal s_wb_ram_neorv32_o : t_wishbone_slave_out;

  signal s_resetn_com   : std_ulogic;

  signal addr : std_logic_vector(31 downto 0) := (others => '0');
  signal comp : std_logic_vector(31 downto 0) := (others => '0');

  signal s_instruction  : std_logic;

  attribute keep                 : boolean;
  signal wb_imem_addr            : std_logic := '0';
  attribute keep of wb_imem_addr : signal is true;

  signal s_arbited_master : t_wishbone_master_out;

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
    IO_UART0_EN       => true,
    XBUS_TIMEOUT      => 0,
    OCD_EN            => g_en_debugging
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
    uart0_txd_o => uart_o,
    jtag_tck_i  => jtag_tck_i,
    jtag_tdi_i  => jtag_tdi_i,
    jtag_tdo_o  => jtag_tdo_o,
    jtag_tms_i  => jtag_tms_i
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

  s_wb_ram_neorv32_i.cyc <= s_xbus_cyc and s_instruction;
  s_wb_ram_neorv32_i.stb <= s_xbus_stb and s_instruction;
  s_wb_ram_neorv32_i.adr <= std_logic_vector(s_xbus_adr);
  s_wb_ram_neorv32_i.sel <= std_logic_vector(s_xbus_sel);
  s_wb_ram_neorv32_i.we  <= s_xbus_we and s_instruction;
  s_wb_ram_neorv32_i.dat <= std_logic_vector(s_xbus_dat_out);

  s_arbited_master.cyc <= s_xbus_cyc and not(s_instruction);
  s_arbited_master.stb <= s_xbus_stb and not(s_instruction);
  s_arbited_master.adr <= std_logic_vector(s_xbus_adr);
  s_arbited_master.sel <= std_logic_vector(s_xbus_sel);
  s_arbited_master.we  <= s_xbus_we and not(s_instruction);
  s_arbited_master.dat <= std_logic_vector(s_xbus_dat_out);

  n_g_use_wb_adapter: if not g_use_wb_adapter generate
    master_o <= s_arbited_master;
  end generate;

  y_g_use_wb_adapter: if g_use_wb_adapter generate
    signal r_master_o       : t_wishbone_master_out;

    signal s_fifo_out     : std_logic_vector(1 + 1 + c_wishbone_address_width + (c_wishbone_address_width/8) + 1 + c_wishbone_data_width - 1 downto 0);
    signal s_block_we     : std_logic;
    signal s_block_rd     : std_logic;
    signal s_fifo_empty   : std_logic;
  begin

    p_output_demux: process (s_gpio_out, r_master_o, s_fifo_empty, s_fifo_out)
    begin
    if(s_gpio_out(0) = '0') then
      master_o <= r_master_o;
    else -- when the ECA block cycle is active, it takes control of the bus until the transfer is done
      if(s_fifo_empty = '1') then  -- as long as there is no bus request from the processor keep the cycle open with a dummy output
        master_o <= to_master_out(s_burst_dummy_master);
      else
        master_o <= to_master_out(s_fifo_out); 
      end if;
    end if;
    end process;

    s_block_we  <= s_arbited_master.stb;
    s_block_rd  <= not master_i.STALL;

    -- FIFO to stall transfers that are requested during a stall cycle 
    block_transfer_FIFO: generic_sync_fifo
    generic map(
      g_data_width  => 1 + 1 + c_wishbone_address_width + (c_wishbone_address_width/8) + 1 + c_wishbone_data_width, -- size of all master out bus signals
      g_size        => 8, -- number of transfers in an ECA burst, thus the assumed maximum transfers per cycle
      g_with_empty  => true,
      g_with_full   => false,
      g_show_ahead  => true  -- so that no rd_i is necessary to see the first datum 
    )
    port map(
      rst_n_i => s_gpio_out(0), -- if the burst mode signal is low the FIFO gets reset so it is empty for the next transfer
      clk_i   => clk_i,
      d_i     => to_vector(s_arbited_master), -- the whole wishbone transfer 
      we_i    => s_block_we,
      q_o     => s_fifo_out,
      rd_i    => s_block_rd,
      empty_o => s_fifo_empty,
      full_o  => open,
      almost_empty_o  => open,
      almost_full_o   => open,
      count_o         => open
    );

    p_single_access : process (clk_i, rstn_i) -- single access wishbone adapter
    begin
      if(rstn_i = '0') then
        r_master_o.cyc <= '0';
        r_master_o.stb <= '0';
        r_master_o.adr <= (others => '0');
        r_master_o.sel <= (others => '0');
        r_master_o.we  <= '0';
        r_master_o.dat <= (others => '0');
      elsif rising_edge(clk_i) then
        if(master_i.STALL = '1' and s_arbited_master.stb = '1') then -- when the crossbar stalls, keep any strobe from the master
          r_master_o <= s_arbited_master;
        elsif ( master_i.STALL = '1' and r_master_o.stb = '1') then -- keep the signals until the stall ends
          r_master_o <= r_master_o;
        else
          r_master_o <= s_arbited_master;
        end if;
      end if;
    end process;
  end generate;

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