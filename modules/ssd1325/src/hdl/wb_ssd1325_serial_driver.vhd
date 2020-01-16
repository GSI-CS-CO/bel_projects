-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;

-- submodules
library work;
use work.generic_fifo_pkg.all;
use work.generic_serial_master_pkg.all;

-- entity
entity wb_ssd1325_serial_driver is
  generic (
    g_address_size  : natural := 32;  -- in bit(s)
    g_data_size     : natural := 32;  -- in bit(s)
    g_spi_fifo_size : natural := 128; -- in g_spi_data_size(s)
    g_spi_data_size : natural := 8    -- in bit(s)
  );
  port (
    -- generic system interface
    clk_sys_i  : in  std_logic;
    rst_n_i    : in  std_logic;
    -- wishbone slave interface
    slave_i    : in  t_wishbone_slave_in;
    slave_o    : out t_wishbone_slave_out;
    -- ssd1325 interface (external signals)
    ssd_rst_o  : out std_logic;
    ssd_dc_o   : out std_logic;
    ssd_ss_o   : out std_logic;
    ssd_sclk_o : out std_logic;
    ssd_data_o : out std_logic;
    -- optional system interface (internal signal)
    ssd_irq_o  : out std_logic
  );
end wb_ssd1325_serial_driver;

-- architecture
architecture rtl of wb_ssd1325_serial_driver is

  -- wb signals
  signal s_ack                          : std_logic;
  signal s_stall                        : std_logic;
  signal s_ss                           : std_logic;
  signal s_irq                          : std_logic;
  signal s_irq_en                       : std_logic;
  signal s_irq_clear                    : std_logic;
  signal s_tx_done                      : std_logic;

  -- tx fifo signals
  signal s_tx_fifo_data_in              : std_logic_vector (g_spi_data_size-1 downto 0);
  signal s_tx_fifo_data_out             : std_logic_vector (g_spi_data_size-1 downto 0);
  signal s_tx_fifo_write_en             : std_logic;
  signal s_tx_fifo_read_en              : std_logic;
  signal s_tx_fifo_empty                : std_logic;
  signal s_tx_fifo_full                 : std_logic;
  signal s_tx_fifo_fill_level           : std_logic_vector (g_spi_data_size-1 downto 0);

  -- serial signals
  signal s_rst_config                   : std_logic; -- Directly control RST
  signal s_dc_config                    : std_logic; -- Directly control DC
  signal s_ss_ctrl_config               : std_logic; -- 0=SPI controls SS - 1=SS is controlled by "tx_control" register
  signal s_ss_state                     : std_logic; -- Directly control SS if s_ss_ctrl_config is set to '1'

  -- registers
  signal s_tx_fifo_data_reg             : std_logic_vector (g_data_size-1 downto 0); -- Write data into the fifo [W]
  signal s_tx_fifo_status_reg           : std_logic_vector (g_data_size-1 downto 0); -- Get fifo flags           [R]
  signal s_tx_fifo_fill_level_reg       : std_logic_vector (g_data_size-1 downto 0); -- Get fill level from fifo [R]
  signal s_tx_control_reg               : std_logic_vector (g_data_size-1 downto 0); -- Configuration register   [W/R]

  -- register mapping
  constant c_address_tx_data_reg        : std_logic_vector (1 downto 0):= "00"; -- 0(3 downto 2) - 00(31 downto 0)
  constant c_address_tx_data_status_reg : std_logic_vector (1 downto 0):= "01"; -- 1(3 downto 2) - 04(31 downto 0)
  constant c_address_tx_fill_level_reg  : std_logic_vector (1 downto 0):= "10"; -- 2(3 downto 2) - 08(31 downto 0)
  constant c_address_tx_control_reg     : std_logic_vector (1 downto 0):= "11"; -- 3(3 downto 2) - 0C(31 downto 0)

  -- constant wishbone bus error (register is not readable or writable, ...)
  constant c_wb_bus_read_error          : std_logic_vector (g_data_size-1 downto 0):= x"DEADBEEF";

begin

  -- submodule transmit fifo
  tx_fifo : generic_fifo
  generic map (
    g_fifo_depth   => g_spi_fifo_size,
    g_data_width   => g_spi_data_size
  )
  port map (
    clk_sys_i      => clk_sys_i,
    rst_n_i        => rst_n_i,
    write_en_i     => s_tx_fifo_write_en,
    data_i         => s_tx_fifo_data_in,
    read_en_i      => s_tx_fifo_read_en,
    data_o         => s_tx_fifo_data_out,
    flag_empty_o   => s_tx_fifo_empty,
    flag_full_o    => s_tx_fifo_full,
    fill_level_o   => s_tx_fifo_fill_level
  );

  -- submodule serial master
  serial_master : generic_serial_master
  generic map (
    g_system_clock => 62500000,
    g_serial_clock => 2000000,
    g_data_width   => g_spi_data_size
  )
  port map (
    clk_sys_i      => clk_sys_i,
    rst_n_i        => rst_n_i,
    mosi_o         => ssd_data_o,
    tx_start_i     => s_tx_fifo_empty,
    tx_data_i      => s_tx_fifo_data_out(g_spi_data_size-1 downto 0),
    sclk_o         => ssd_sclk_o,
    ss_o           => s_ss,
    tx_done_o      => s_tx_done,
    rx_read_o      => s_tx_fifo_read_en
  );

  -- register bit layout
  s_tx_fifo_status_reg(g_data_size-1 downto 3)     <= (others => '0');
  s_tx_fifo_status_reg(2 downto 0)                 <= s_irq & s_tx_fifo_empty & s_tx_fifo_full;

  s_tx_fifo_fill_level_reg(g_data_size-1 downto 8) <= (others => '0');
  s_tx_fifo_fill_level_reg(7 downto 0)             <= s_tx_fifo_fill_level;

  s_tx_control_reg(g_data_size-1 downto 6)         <= (others => '0');
  s_tx_control_reg(5 downto 0)                     <= s_irq_clear & s_irq_en & s_ss_state & s_ss_ctrl_config & s_dc_config & s_rst_config;

  -- signals depending on tx_control_reg
  ssd_dc_o  <= s_dc_config;
  ssd_rst_o <= s_rst_config;
  ssd_irq_o <= s_irq and s_irq_en;

  -- process slave select depending on slave select configuration
  p_ss : process(s_ss, s_ss_state, s_ss_ctrl_config)
  begin
    if (s_ss_ctrl_config = '0') then
      ssd_ss_o <= s_ss;
    else
      ssd_ss_o <= s_ss_state;
    end if;
  end process;

  -- process interrupt signal
  p_irq : process(clk_sys_i, rst_n_i)
  begin
    -- reset detection
    if (rst_n_i = '0') then
      s_irq <= '1';
    -- process with normal flow
    elsif (rising_edge(clk_sys_i)) then
      if (s_tx_done='1' and s_tx_fifo_empty='1') then
        s_irq <= '1';
      elsif (s_tx_fifo_empty = '0') then
        s_irq <= '0';
      elsif (s_irq_clear = '1') then
        s_irq <= '0';
      end if;
    end if;
  end process;

  -- process wishbone acknowlegde
  p_wb_ack : process(s_ack)
  begin
    slave_o.ack <= s_ack;
  end process;

  -- process wishbone stall
  p_wb_stall : process(s_stall)
  begin
    slave_o.stall <= s_stall;
  end process;

  -- process handle wishbone requests
  p_wb_handle_requests : process(clk_sys_i, rst_n_i)
  begin
  -- reset detection
    if (rst_n_i = '0') then
      s_ack              <= '0';
      s_stall            <= '0';
      slave_o.err        <= '0';
      slave_o.rty        <= '0';
      slave_o.dat        <= (others => '0');
      s_dc_config        <= '0';
      s_rst_config       <= '0';
      s_ss_ctrl_config   <= '0';
      s_ss_state         <= '0';
      s_irq_clear        <= '0';
      s_irq_en           <= '0';
      s_tx_fifo_data_in  <= (others => '0');
      s_tx_fifo_write_en <= '0';
      -- process with normal flow
    elsif (rising_edge(clk_sys_i)) then
      -- generate ack and others wishbone signals
      s_ack       <= slave_i.cyc and slave_i.stb; -- and not(s_stall) -- (for fifo full check/if stall is wanted)
      s_stall     <= '0';                         -- s_tx_fifo_full   -- (for fifo full check/...)
      slave_o.err <= '0';
      slave_o.rty <= '0';

      -- check if a request is incoming
      if (slave_i.stb='1' and slave_i.cyc='1') then
        -- evaluate address and write enable signals
        case slave_i.adr(3 downto 2) is

          -- handle requests for tx data register
          when c_address_tx_data_reg =>
            if (slave_i.we='1') then
              s_tx_fifo_data_in  <= slave_i.dat(7 downto 0);
              s_tx_fifo_write_en <= '1';
            end if;
            slave_o.dat          <= c_wb_bus_read_error; -- this is no read address

          -- handle requests for tx status register
          when c_address_tx_data_status_reg =>
            slave_o.dat          <= s_tx_fifo_status_reg; -- this is read only

          -- handle requests for tx fill level register
          when c_address_tx_fill_level_reg =>
            slave_o.dat          <= s_tx_fifo_fill_level_reg; -- this is read only

          -- handle requests for tx control register
          when c_address_tx_control_reg =>
            if (slave_i.we='1') then
              s_rst_config       <= slave_i.dat(0);
              s_dc_config        <= slave_i.dat(1);
              s_ss_ctrl_config   <= slave_i.dat(2);
              s_ss_state         <= slave_i.dat(3);
              s_irq_en           <= slave_i.dat(4);
              s_irq_clear        <= slave_i.dat(5);
            end if;
            slave_o.dat          <= s_tx_control_reg; -- this is read only

          -- unknown access
          when others =>
            slave_o.dat          <= c_wb_bus_read_error; -- this is no write or read address

        end case; -- end address based switching

      -- no cycle or strobe
      else
        s_tx_fifo_write_en <= '0';
        s_irq_clear        <= '0';
        slave_o.dat        <= (others => '0');
      end if; -- check for cycle and strobe

    end if; -- check reset
  end process;

end rtl;
