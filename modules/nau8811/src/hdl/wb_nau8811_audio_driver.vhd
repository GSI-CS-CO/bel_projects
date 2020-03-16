-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;

-- submodules
library work;
use work.wb_nau8811_audio_driver_pkg.all;
use work.generic_serial_master_pkg.all;
use work.generic_iis_master_pkg.all;
use work.generic_fifo_pkg.all;

-- entity
entity wb_nau8811_audio_driver is
  generic (
    g_address_size     : natural := 32;   -- in bit(s)
    g_data_size        : natural := 32;   -- in bit(s)
    g_spi_fifo_size    : natural := 32;   -- in g_spi_data_size(s)
    g_spi_data_size    : natural := 16;   -- in bit(s)
    g_iis_tx_fifo_size : natural := 1024; -- in g_iis_tx_fifo_size(s)
    g_iis_rx_fifo_size : natural := 1024; -- in g_iis_rx_fifo_size(s)
    g_iis_word_size    : natural := 32;   -- in bit(s)
    g_use_external_pll : boolean := true  -- true for real synthesis or false for simulation purpose
  );
  port (
    -- generic system interface
    clk_sys_i    : in  std_logic;
    rst_n_i      : in  std_logic;
    -- pll reference
    pll_ref_i    : in  std_logic;
    -- heartbeat trigger
    trigger_i    : in  std_logic;
    -- wishbone slave interface
    slave_i      : in  t_wishbone_slave_in;
    slave_o      : out t_wishbone_slave_out;
    -- nau8811 serial interface
    spi_csb_o    : out std_logic; -- Chip/Slave select
    spi_sclk_o   : out std_logic; -- Serial clock
    spi_sdio_o   : out std_logic; -- Serial data in/out (only out is used here)
    -- nau8811 inter-ic sound interface
    iis_fs_o     : out std_logic; -- Frame sync
    iis_bclk_o   : out std_logic; -- Clock
    iis_adcout_o : out std_logic; -- Data out
    iis_dacin_i  : in  std_logic  -- Data in
  );
end wb_nau8811_audio_driver;

-- architecture
architecture rtl of wb_nau8811_audio_driver is

  -- wb signals
  signal s_ack   : std_logic;
  signal s_stall : std_logic;

  -- spi core
  signal s_spi_csb : std_logic;

  -- iis core
  signal s_iis_clk           : std_logic;
  signal s_iis_reset         : std_logic;
  signal s_iis_adc_out       : std_logic;
  signal s_iis_adc_in        : std_logic;
  signal s_iis_tx_fill_level : std_logic_vector (9 downto 0);
  signal s_iis_rx_fill_level : std_logic_vector (9 downto 0);

  -- pll management
  signal s_pll_reset  : std_logic;
  signal s_pll_locked : std_logic;

  -- external trigger
  signal s_iis_trigger_sync : std_logic;

  -- tx fifo signals (spi)
  signal s_spi_tx_fifo_data_in  : std_logic_vector (g_spi_data_size-1 downto 0);
  signal s_spi_tx_fifo_data_out : std_logic_vector (g_spi_data_size-1 downto 0);
  signal s_spi_tx_fifo_write_en : std_logic;
  signal s_spi_tx_fifo_read_en  : std_logic;
  signal s_spi_tx_fifo_empty    : std_logic;
  signal s_spi_tx_fifo_full     : std_logic;

  -- tx fifo signals (iis)
  signal s_iis_tx_fifo_data_in  : std_logic_vector (g_iis_word_size-1 downto 0);
  signal s_iis_tx_fifo_data_out : std_logic_vector (g_iis_word_size-1 downto 0);
  signal s_iis_tx_fifo_write_en : std_logic;
  signal s_iis_tx_fifo_read_en  : std_logic;
  signal s_iis_tx_fifo_empty    : std_logic;
  signal s_iis_tx_fifo_full     : std_logic;
  signal s_iis_tx_fifo_empty_m  : std_logic;

  -- rx fifo signals (iis)
  signal s_iis_rx_fifo_data_in  : std_logic_vector (g_iis_word_size-1 downto 0);
  signal s_iis_rx_fifo_data_out : std_logic_vector (g_iis_word_size-1 downto 0);
  signal s_iis_rx_fifo_write_en : std_logic;
  signal s_iis_rx_fifo_read_en  : std_logic;
  signal s_iis_rx_fifo_empty    : std_logic;
  signal s_iis_rx_fifo_full     : std_logic;
  signal s_iis_rx_fifo_full_m   : std_logic;

  -- rx fifo signals (iis)
  type   t_iis_rx_fifo_read_state is (state_iis_rx_no_request_pending, state_iis_rx_request_pending, state_iis_rx_request_complete);
  signal s_iis_rx_fifo_read_state : t_iis_rx_fifo_read_state := state_iis_rx_no_request_pending;

    -- stream tx fifo signals
  type   t_iis_tx_fifo_stream_state is (state_iis_tx_no_request_pending, state_iis_tx_request_pending, state_iis_tx_request_complete);
  signal s_iis_tx_fifo_stream_state : t_iis_tx_fifo_stream_state := state_iis_tx_no_request_pending;
  signal s_iis_tx_fifo_stream_cache : std_logic_vector ((g_iis_word_size-1) downto 0);

  -- stream rx fifo signals
  type   t_iis_rx_fifo_stream_state is (state_iis_rx_no_request_pending, state_iis_rx_request_pending, state_iis_rx_request_complete);
  signal s_iis_rx_fifo_stream_state : t_iis_rx_fifo_stream_state := state_iis_rx_no_request_pending;

  -- register mapping
  constant c_address_status_reg         : std_logic_vector (2 downto 0):= "000"; -- 0(3 downto 2) - 0x00(31 downto 0)
  constant c_address_control_reg        : std_logic_vector (2 downto 0):= "001"; -- 1(3 downto 2) - 0x04(31 downto 0)
  constant c_address_tx_spi_data_reg    : std_logic_vector (2 downto 0):= "010"; -- 2(3 downto 2) - 0x08(31 downto 0)
  constant c_address_tx_iis_data_reg    : std_logic_vector (2 downto 0):= "011"; -- 3(3 downto 2) - 0x0C(31 downto 0)
  constant c_address_rx_iis_data_reg    : std_logic_vector (2 downto 0):= "100"; -- 4(3 downto 2) - 0x10(31 downto 0)
  constant c_address_tx_iis_stream_reg  : std_logic_vector (2 downto 0):= "101"; -- 5(3 downto 2) - 0x14(31 downto 0)
  constant c_address_rx_iis_stream_reg  : std_logic_vector (2 downto 0):= "110"; -- 6(3 downto 2) - 0x18(31 downto 0)
  constant c_address_iis_fill_level_reg : std_logic_vector (2 downto 0):= "111"; -- 7(3 downto 2) - 0x1c(31 downto 0)

  -- register content
  signal s_control_reg              : std_logic_vector ((g_data_size-1) downto 0);
  signal s_iis_heartbeat_pol        : std_logic; -- 0 => Falling edge - 1 => Rising edge.
  signal s_iis_heartbeat_mode       : std_logic; -- 0 => No heartbeat trigger - 1 => Play sound on trigger.
  signal s_iis_internal_loop        : std_logic; -- 0 => Don't loop back data - 1 => Loop data back.
  signal s_iis_padding              : std_logic; -- X => Padding Value for unused bits at the audio frame.
  signal s_iis_invert_fs            : std_logic; -- 0 => Don't invert FS - 1 => Invert FS.
  signal s_iis_mono_output          : std_logic; -- 0 => Clone left channel to right channel - 1 => Only send data for left channel.
  signal s_iis_transaction_mode     : std_logic; -- 0 => Only receive data when transmitting new data - 1 => Receive new data all the time.
  signal s_iis_control_clock_enable : std_logic; -- 0 => Disable clockd and frame sync - 1 => Enable clock and frame sync
  signal s_spi_ss_state             : std_logic; -- X => Directly control SS if s_spi_ss_ctrl_config is set to '1'.
  signal s_spi_ss_ctrl_config       : std_logic; -- 0 => SPI controls SS - 1 => SS is controlled by control register.
  signal s_status_reg               : std_logic_vector ((g_data_size-1) downto 0);
  signal s_iis_fill_level_reg       : std_logic_vector ((g_data_size-1) downto 0);

  -- sync
  signal s_wb_to_iis_stage1         : std_logic_vector (7 downto 0);
  signal s_wb_to_iis_stage2         : std_logic_vector (7 downto 0);

  -- constant wishbone bus error (register is not readable or writable, ...)
  constant c_wb_bus_read_error : std_logic_vector (g_data_size-1 downto 0):= x"DEADBEEF";

begin

  -- submodule transmit fifo for spi
  spi_tx_fifo : generic_fifo
  generic map (
    g_fifo_depth   => g_spi_fifo_size,
    g_data_width   => g_spi_data_size
  )
  port map (
    clk_sys_i      => clk_sys_i,
    rst_n_i        => rst_n_i,
    write_en_i     => s_spi_tx_fifo_write_en,
    data_i         => s_spi_tx_fifo_data_in,
    read_en_i      => s_spi_tx_fifo_read_en,
    data_o         => s_spi_tx_fifo_data_out,
    flag_empty_o   => s_spi_tx_fifo_empty,
    flag_full_o    => s_spi_tx_fifo_full
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
    mosi_o         => spi_sdio_o,
    tx_start_i     => s_spi_tx_fifo_empty,
    tx_data_i      => s_spi_tx_fifo_data_out(g_spi_data_size-1 downto 0),
    sclk_o         => spi_sclk_o,
    ss_o           => s_spi_csb,
    rx_read_o      => s_spi_tx_fifo_read_en
  );

  -- submodule iis master
  iis_master : generic_iis_master
  generic map (
    g_word_width     => g_iis_word_size
  )
  port map (
    clk_sys_i         => s_iis_clk,
    rst_n_i           => s_iis_reset,
    iis_fs_o          => iis_fs_o,
    iis_bclk_o        => iis_bclk_o,
    iis_adcout_o      => s_iis_adc_out,
    iis_dacin_i       => s_iis_adc_in,
    tx_fifo_empty_i   => s_iis_tx_fifo_empty_m,
    rx_fifo_full_i    => s_iis_rx_fifo_full_m,
    tx_data_i         => s_iis_tx_fifo_data_out(g_iis_word_size-1 downto 0),
    rx_data_o         => s_iis_rx_fifo_data_in(g_iis_word_size-1 downto 0),
    tx_read_o         => s_iis_tx_fifo_read_en,
    rx_write_o        => s_iis_rx_fifo_write_en,
    iis_heartbeat_i   => s_wb_to_iis_stage2(7), -- s_iis_heartbeat_mode
    iis_trigger_pol_i => s_wb_to_iis_stage2(6), -- s_iis_heartbeat_pol
    iis_trigger_i     => s_wb_to_iis_stage2(5), -- s_iis_trigger_sync
    iis_enable_clk_i  => s_wb_to_iis_stage2(4), -- s_iis_control_clock_enable
    iis_transaction_i => s_wb_to_iis_stage2(3), -- s_iis_transaction_mode
    iis_mono_output_i => s_wb_to_iis_stage2(2), -- s_iis_mono_output
    iis_invert_fs_i   => s_wb_to_iis_stage2(1), -- s_iis_invert_fs
    iis_padding_i     => s_wb_to_iis_stage2(0)  -- s_iis_padding
  );

    iis_tx_fifo : generic_async_fifo
    generic map (
      g_data_width => g_iis_word_size,
      g_size       => g_iis_tx_fifo_size,
      g_show_ahead => true,

      g_with_rd_empty        => true,
      g_with_rd_full         => true,
      g_with_rd_almost_empty => false,
      g_with_rd_almost_full  => false,
      g_with_rd_count        => false,

      g_with_wr_empty        => true,
      g_with_wr_full         => true,
      g_with_wr_almost_empty => false,
      g_with_wr_almost_full  => false,
      g_with_wr_count        => true,

      g_almost_empty_threshold => 0,
      g_almost_full_threshold  => 0
      )
    port map (
      rst_n_i           => rst_n_i,

      clk_wr_i          => clk_sys_i,
      d_i               => s_iis_tx_fifo_data_in,
      we_i              => s_iis_tx_fifo_write_en,
      wr_empty_o        => s_iis_tx_fifo_empty,
      wr_full_o         => s_iis_tx_fifo_full,
      wr_almost_empty_o => open,
      wr_almost_full_o  => open,
      wr_count_o        => s_iis_tx_fill_level,

      clk_rd_i          => s_iis_clk,
      q_o               => s_iis_tx_fifo_data_out,
      rd_i              => s_iis_tx_fifo_read_en,
      rd_empty_o        => s_iis_tx_fifo_empty_m,
      rd_full_o         => open,
      rd_almost_empty_o => open,
      rd_almost_full_o  => open,
      rd_count_o        => open);

    iis_rx_fifo : generic_async_fifo
    generic map (
      g_data_width => g_iis_word_size,
      g_size       => g_iis_tx_fifo_size,
      g_show_ahead => true,

      g_with_rd_empty        => true,
      g_with_rd_full         => true,
      g_with_rd_almost_empty => false,
      g_with_rd_almost_full  => false,
      g_with_rd_count        => true,

      g_with_wr_empty        => true,
      g_with_wr_full         => true,
      g_with_wr_almost_empty => false,
      g_with_wr_almost_full  => false,
      g_with_wr_count        => false,

      g_almost_empty_threshold => 0,
      g_almost_full_threshold  => 0
      )
    port map (
      rst_n_i           => rst_n_i,

      clk_wr_i          => s_iis_clk,
      d_i               => s_iis_rx_fifo_data_in,
      we_i              => s_iis_rx_fifo_write_en,
      wr_empty_o        => open,
      wr_full_o         => s_iis_rx_fifo_full_m,
      wr_almost_empty_o => open,
      wr_almost_full_o  => open,
      wr_count_o        => open,

      clk_rd_i          => clk_sys_i,
      q_o               => s_iis_rx_fifo_data_out,
      rd_i              => s_iis_rx_fifo_read_en,
      rd_empty_o        => s_iis_rx_fifo_empty,
      rd_full_o         => s_iis_rx_fifo_full,
      rd_almost_empty_o => open,
      rd_almost_full_o  => open,
      rd_count_o        => s_iis_rx_fill_level);

  -- pll/clock management simulation purpose
  audio_pll_n : if not g_use_external_pll generate
    -- simulate clock
    p_clock : process
    begin
      s_iis_clk <= '0';
      wait for 20.25 ns;
      s_iis_clk <= '1';
      wait for 20.25 ns;
    end process;
    -- simulate locked signal
    p_lock : process
    begin
      s_pll_locked <= '0';
      wait for 100 ns;
      s_pll_locked <= '1';
      wait;
    end process;
  end generate;

  -- pll/clock management real hardware
  audio_pll_y : if g_use_external_pll generate
    x : audio_pll
    port map (
        refclk   => pll_ref_i,
        rst      => s_pll_reset,
        outclk_0 => s_iis_clk,
        locked   => s_pll_locked
      );
  end generate;

  -- process reset management
  p_reset : process(rst_n_i)
  begin
    -- invert the locked signal and use it as reset
    s_iis_reset <= rst_n_i;
  end process;

  -- process pll reset management
  p_pll_reset : process(clk_sys_i, rst_n_i)
  begin
    -- Timeout state machine
    if (rst_n_i = '0') then
      s_pll_reset <= '0';
    elsif (rising_edge(clk_sys_i)) then
      if (s_pll_reset = '1') then
        s_pll_reset <= '0';
      elsif (s_pll_locked = '0') then
        s_pll_reset <= '1';
      else
        s_pll_reset <= '0';
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

  -- process iis adcout
  p_iis_adcout : process(s_iis_adc_out)
  begin
    iis_adcout_o <= s_iis_adc_out;
  end process;

    -- process iis adcin
  p_iis_adin : process(iis_dacin_i, s_iis_adc_out, s_iis_internal_loop)
  begin
    if (s_iis_internal_loop = '0') then
      s_iis_adc_in <= iis_dacin_i;
    else
      s_iis_adc_in <= s_iis_adc_out;
    end if;
  end process;

  -- status register
   p_status_reg : process(s_iis_rx_fifo_empty, s_iis_rx_fifo_full, s_iis_tx_fifo_empty, s_iis_tx_fifo_full, s_spi_tx_fifo_empty, s_spi_tx_fifo_full)
  begin
    s_status_reg(0)                        <= s_spi_tx_fifo_full;
    s_status_reg(1)                        <= s_spi_tx_fifo_empty;
    s_status_reg(2)                        <= s_iis_tx_fifo_full;
    s_status_reg(3)                        <= s_iis_tx_fifo_empty;
    s_status_reg(4)                        <= s_iis_rx_fifo_full;
    s_status_reg(5)                        <= s_iis_rx_fifo_empty;
    s_status_reg((g_data_size-1) downto 6) <= (others => '0');
  end process;

  -- process slave select depending on slave select configuration
  p_ss : process(s_spi_csb, s_spi_ss_state, s_spi_ss_ctrl_config)
  begin
    if (s_spi_ss_ctrl_config = '0') then
      spi_csb_o <= s_spi_csb;
    else
      spi_csb_o <= s_spi_ss_state;
    end if;
  end process;

  -- synchronize trigger in signal
  p_sync_trigger : process(clk_sys_i, rst_n_i)
  begin
    -- Timeout state machine
    if (rst_n_i = '0') then
      s_iis_trigger_sync <= '0';
    elsif (rising_edge(clk_sys_i)) then
      s_iis_trigger_sync <= trigger_i;
    end if;
  end process;

  -- control register
  p_control_reg : process(s_spi_ss_ctrl_config, s_spi_ss_state, s_iis_control_clock_enable, s_iis_transaction_mode, s_iis_mono_output, s_iis_invert_fs, s_iis_padding, s_iis_internal_loop, s_iis_heartbeat_mode, s_iis_heartbeat_pol)
  begin
    s_control_reg(0)                         <= s_spi_ss_ctrl_config;
    s_control_reg(1)                         <= s_spi_ss_state;
    s_control_reg(2)                         <= s_iis_control_clock_enable;
    s_control_reg(3)                         <= s_iis_transaction_mode;
    s_control_reg(4)                         <= s_iis_mono_output;
    s_control_reg(5)                         <= s_iis_invert_fs;
    s_control_reg(6)                         <= s_iis_padding;
    s_control_reg(7)                         <= s_iis_internal_loop;
    s_control_reg(8)                         <= s_iis_heartbeat_mode;
    s_control_reg(9)                         <= s_iis_heartbeat_pol;
    s_control_reg((g_data_size-1) downto 10) <= (others => '0');
  end process;

  -- fifo fill level register
  p_fill_level_reg : process (s_iis_tx_fill_level, s_iis_tx_fifo_full, s_iis_rx_fill_level, s_iis_rx_fifo_full)
  begin
    s_iis_fill_level_reg(9 downto 0)   <= s_iis_tx_fill_level;
    s_iis_fill_level_reg(10)           <= s_iis_tx_fifo_full;
    s_iis_fill_level_reg(15 downto 11) <= (others => '0');
    s_iis_fill_level_reg(25 downto 16) <= s_iis_rx_fill_level;
    s_iis_fill_level_reg(26)           <= s_iis_rx_fifo_full;
    s_iis_fill_level_reg(31 downto 27) <= (others => '0');
  end process;

  -- process handle wishbone requests
  p_wb_handle_requests : process(clk_sys_i, rst_n_i)
  begin
    -- reset detection
    if (rst_n_i = '0') then
      s_ack                      <= '0';
      s_stall                    <= '0';
      slave_o.err                <= '0';
      slave_o.rty                <= '0';
      slave_o.dat                <= (others => '0');
      s_spi_tx_fifo_data_in      <= (others => '0');
      s_spi_tx_fifo_write_en     <= '0';
      s_spi_ss_ctrl_config       <= '0';
      s_spi_ss_state             <= '0';
      s_iis_control_clock_enable <= '0';
      s_iis_transaction_mode     <= '0';
      s_iis_mono_output          <= '0';
      s_iis_invert_fs            <= '0';
      s_iis_padding              <= '0';
      s_iis_internal_loop        <= '0';
      s_iis_heartbeat_mode       <= '0';
      s_iis_heartbeat_pol        <= '0';
      s_iis_tx_fifo_stream_cache <= (others => '0');
      s_iis_rx_fifo_read_state   <= state_iis_rx_no_request_pending;
      s_iis_tx_fifo_stream_state <= state_iis_tx_no_request_pending;
      s_iis_rx_fifo_stream_state <= state_iis_rx_no_request_pending;

    -- process with normal flow
    elsif (rising_edge(clk_sys_i)) then
      -- generate ack and others wishbone signals
      slave_o.err <= '0';
      slave_o.rty <= '0';

    ---- rx fifo state machine (stall management)
    --------------------------------------------------------------------------------------------------------------------
    -- check if a request is incoming
    if (s_iis_rx_fifo_read_state = state_iis_rx_request_pending) then
      s_iis_rx_fifo_read_state       <= state_iis_rx_request_complete;
      s_iis_rx_fifo_read_en          <= '0';

    -- finish the rx read request
    elsif (s_iis_rx_fifo_read_state = state_iis_rx_request_complete) then
      s_stall                              <= '0';
      s_ack                                <= '1';
      slave_o.dat                          <= s_iis_rx_fifo_data_out;
      s_iis_rx_fifo_read_state             <= state_iis_rx_no_request_pending;

      -- tx fifo stream mode state machine
      ------------------------------------------------------------------------------------------------------------------
      -- write data to fifo is it is not full
      elsif (s_iis_tx_fifo_stream_state = state_iis_tx_request_pending) then
        if(s_iis_tx_fifo_full = '0') then
          s_iis_tx_fifo_data_in            <= s_iis_tx_fifo_stream_cache;
          s_iis_tx_fifo_write_en           <= '1';
          s_iis_tx_fifo_stream_state       <= state_iis_tx_request_complete;
        end if;

      -- clear write request and accept next data
      elsif (s_iis_tx_fifo_stream_state = state_iis_tx_request_complete) then
        s_stall                            <= '0';
        s_ack                              <= '1';
        s_iis_tx_fifo_write_en             <= '0';
        s_iis_tx_fifo_stream_state         <= state_iis_tx_no_request_pending;

      -- rx fifo stream mode state machine
      ------------------------------------------------------------------------------------------------------------------
      -- read data from fifo if it is not empty
      elsif (s_iis_rx_fifo_stream_state = state_iis_rx_request_pending) then
        if(s_iis_rx_fifo_empty = '0') then
          s_iis_rx_fifo_read_en            <= '1';
          s_iis_rx_fifo_stream_state       <= state_iis_rx_request_complete;
        end if;

      elsif (s_iis_rx_fifo_stream_state = state_iis_rx_request_complete) then
        if ( s_iis_rx_fifo_read_en = '1') then
          s_iis_rx_fifo_read_en            <= '0';
        else
          s_stall                          <= '0';
          s_ack                            <= '1';
          slave_o.dat                      <= s_iis_rx_fifo_data_out;
          s_iis_rx_fifo_stream_state       <= state_iis_rx_no_request_pending;
        end if;

      -- handle all initial requests
      ------------------------------------------------------------------------------------------------------------------
      -- expect a valid request/strobe
      elsif (slave_i.stb='1' and slave_i.cyc='1') then
        -- evaluate address and write enable signals
        case slave_i.adr(4 downto 2) is

          -- handle requests for generic status register
          when c_address_status_reg =>
            slave_o.dat                    <= s_status_reg;
            s_stall                        <= '0';
            s_ack                          <= '1';

          -- handle requests for generic control register
          when c_address_control_reg =>
            if (slave_i.we='1') then
              s_spi_ss_ctrl_config         <= slave_i.dat(0);
              s_spi_ss_state               <= slave_i.dat(1);
              s_iis_control_clock_enable   <= slave_i.dat(2);
              s_iis_transaction_mode       <= slave_i.dat(3);
              s_iis_mono_output            <= slave_i.dat(4);
              s_iis_invert_fs              <= slave_i.dat(5);
              s_iis_padding                <= slave_i.dat(6);
              s_iis_internal_loop          <= slave_i.dat(7);
              s_iis_heartbeat_mode         <= slave_i.dat(8);
              s_iis_heartbeat_pol          <= slave_i.dat(9);
            end if;
            slave_o.dat                    <= s_control_reg;
            s_stall                        <= '0';
            s_ack                          <= '1';

          -- handle requests for spi tx data register
          when c_address_tx_spi_data_reg =>
            if (slave_i.we='1') then
              s_spi_tx_fifo_data_in        <= slave_i.dat(g_spi_data_size-1 downto 0);
              s_spi_tx_fifo_write_en       <= '1';
            end if;
            slave_o.dat                    <= c_wb_bus_read_error; -- this is no read address
            s_stall                        <= '0';
            s_ack                          <= '1';

          -- handle requests for iis tx data register
          when c_address_tx_iis_data_reg =>
            if (slave_i.we='1') then
              s_iis_tx_fifo_data_in        <= slave_i.dat((g_iis_word_size)-1 downto 0);
              s_iis_tx_fifo_write_en       <= '1';
            end if;
            slave_o.dat                    <= c_wb_bus_read_error; -- this is no read address
            s_stall                        <= '0';
            s_ack                          <= '1';

          -- handle requests for iis rx data register
          when c_address_rx_iis_data_reg =>
            if (slave_i.we='1') then
              s_stall                      <= '0';
              s_ack                        <= '1';
              slave_o.dat                  <= c_wb_bus_read_error;
            else
              s_iis_rx_fifo_read_en        <= '1';
              s_stall                      <= '0';
              s_ack                        <= '1';
              slave_o.dat                  <= s_iis_rx_fifo_data_out;
            end if;


          -- handle requests for iis tx stream register
          when c_address_tx_iis_stream_reg =>
            if (slave_i.we='1') then
              -- write date to fifo if it is not full
              if (s_iis_tx_fifo_full = '0') then
                s_iis_tx_fifo_data_in      <= slave_i.dat((g_iis_word_size)-1 downto 0);
                s_iis_tx_fifo_write_en     <= '1';
                s_stall                    <= '0';
                s_ack                      <= '1';
              -- wait until fifo is not longer full
              else
                s_stall                    <= '1';
                s_ack                      <= '0';
                s_iis_tx_fifo_stream_cache <= slave_i.dat((g_iis_word_size-1) downto 0);
                s_iis_tx_fifo_stream_state <= state_iis_tx_request_pending;
              end if;
            else
              s_stall                      <= '0';
              s_ack                        <= '1';
            end if;
            slave_o.dat                    <= c_wb_bus_read_error; -- this is no read address

          -- handle requests for iis rx stream register
          when c_address_rx_iis_stream_reg =>
            if (slave_i.we='1') then
              s_stall                        <= '0';
              s_ack                          <= '1';
            else
              if (s_iis_rx_fifo_empty = '0') then
                s_iis_rx_fifo_read_en      <= '1';
                s_stall                    <= '0';
                s_ack                      <= '1';
                slave_o.dat                <= s_iis_rx_fifo_data_out;
              else
                s_stall                    <= '1';
                s_ack                      <= '0';
                s_iis_rx_fifo_stream_state <= state_iis_rx_request_pending;
              end if;
            end if;
            slave_o.dat                    <= c_wb_bus_read_error; -- this is no read address

          -- handle requests for fifo fill level register
          when c_address_iis_fill_level_reg =>
            slave_o.dat                    <= s_iis_fill_level_reg;
            s_stall                        <= '0';
            s_ack                          <= '1';

          -- unknown access
          when others =>
            slave_o.dat                    <= c_wb_bus_read_error; -- this is no write or read address
            s_stall                        <= '0';
            s_ack                          <= '1';

        end case; -- end address based switching

      -- no cycle or strobe
      else
        s_spi_tx_fifo_write_en             <= '0';
        s_iis_tx_fifo_write_en             <= '0';
        s_iis_rx_fifo_read_en              <= '0';
        slave_o.dat                        <= (others => '0');
        s_stall                            <= '0';
        s_ack                              <= '0';
      end if; -- check for cycle and strobe

    end if; -- check reset
  end process;

  -- sync signals for iis master
  p_sync_signals : process(pll_ref_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      s_wb_to_iis_stage1 <= (others => '0');
      s_wb_to_iis_stage2 <= (others => '0');
    elsif (rising_edge(pll_ref_i)) then
      s_wb_to_iis_stage1(7) <= s_iis_heartbeat_mode;
      s_wb_to_iis_stage1(6) <= s_iis_heartbeat_pol;
      s_wb_to_iis_stage1(5) <= s_iis_trigger_sync;
      s_wb_to_iis_stage1(4) <= s_iis_control_clock_enable;
      s_wb_to_iis_stage1(3) <= s_iis_transaction_mode;
      s_wb_to_iis_stage1(2) <= s_iis_mono_output;
      s_wb_to_iis_stage1(1) <= s_iis_invert_fs;
      s_wb_to_iis_stage1(0) <= s_iis_padding;
      s_wb_to_iis_stage2 <= s_wb_to_iis_stage1;
    end if;
  end process;

end rtl;
