library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.monster_pkg.all;
use work.a10ts_pkg.all;

entity a10ts is
  generic (
    g_use_ext_trigger : boolean := false);
  port(
    clk_i     : in  std_logic := '0';
    rst_n_i   : in  std_logic := '1';
    clk_20m_i : in  std_logic := '0';
    ge_85_c_o : out std_logic := '0';
    slave_i   : in  t_wishbone_slave_in;
    slave_o   : out t_wishbone_slave_out);
end a10ts;

architecture rtl of a10ts is
  constant c_divider      : integer := 20000;
  signal s_counter        : integer range 0 to c_divider - 1 := 0;

  signal s_temp_out       : std_logic_vector(9 downto 0) := (others => '0');
  signal s_temp_sync      : std_logic_vector(9 downto 0) := (others => '0');

  signal s_clk_1k         : std_logic := '0';

  signal s_eoc            : std_logic := '0';
  signal s_eoc_sync       : std_logic := '0';
  signal s_eoc_sync_latch : std_logic := '0';

begin

  -- Fixed assignments
  slave_o.err   <= '0';
  slave_o.stall <= '0';
  slave_o.rty   <= '0';

  -- Instantiate the ADC/temperature sensor
  ext_trigger_y : if g_use_ext_trigger generate
    core_a10ts_ip : a10ts_ip
    port map (
      corectl => s_clk_1k,
      eoc     => s_eoc,
      reset   => '0',
      tempout => s_temp_out
    );
  end generate;

  ext_trigger_n : if not g_use_ext_trigger generate
    core_a10ts_ip : a10ts_ip
    port map (
      corectl => '1', -- s_clk_1k
      eoc     => s_eoc,
      reset   => '0',
      tempout => s_temp_out
    );
  end generate;

  -- Sync the EOC signal from the ADC into the Wishbone clock domain
  sync_eoc : gc_sync_ffs
  port map (
    clk_i    => clk_i,
    rst_n_i  => rst_n_i,
    data_i   => s_eoc,
    synced_o => s_eoc_sync
  );

  -- Handle all Wishbone requests
  wb_handler : process(clk_i, rst_n_i) is
  begin
    if rst_n_i = '0' then
      slave_o.ack <= '0';
      slave_o.dat <= (others => '0');
    elsif rising_edge(clk_i) then
      slave_o.ack <= slave_i.cyc and slave_i.stb;
      if (slave_i.cyc and slave_i.stb) = '1' then
        slave_o.dat (9 downto 0)   <= s_temp_sync;
        slave_o.dat (31 downto 10) <= (others => '0');
      else
        slave_o.dat <= (others => '0');
      end if;
    end if;
  end process;

  ge_85_c_detector : process(clk_i, rst_n_i) is
  begin
    if rst_n_i = '0' then
      ge_85_c_o <= '0';
    elsif rising_edge(clk_i) then
      if unsigned(s_temp_sync) >= to_unsigned(517, s_temp_sync'length) then -- 517 ~= 85 °C
        ge_85_c_o <= '1';
      else
        ge_85_c_o <= '0';
      end if;
    end if;
  end process;

  -- Detect new ADC/temperature sensor values (EOC goes zero)
  eoc_detector : process(clk_i, rst_n_i) is
  begin
    if rst_n_i = '0' then
      s_eoc_sync_latch <= '0';
      s_temp_sync      <= (others => '0');
    elsif rising_edge(clk_i) then
      s_eoc_sync_latch <= s_eoc_sync;
      if (s_eoc_sync_latch = '1' and s_eoc_sync = '0') then
        s_temp_sync <= s_temp_out;
      end if;
    end if;
  end process;

  -- Generate 1MHz to trigger the ADC
  ext_trigger_generator_y : if g_use_ext_trigger generate
    gen_adc_trigger : process(clk_20m_i, rst_n_i) is
    begin
      if rst_n_i = '0' then
        s_counter <= 0;
        s_clk_1k  <= '0';
      elsif rising_edge(clk_20m_i) then
        if s_counter = c_divider - 1 then
          s_counter <= 0;
          s_clk_1k  <= not s_clk_1k;
        else
          s_counter <= s_counter + 1;
        end if;
      end if;
    end process;
  end generate;

  ext_trigger_generator_n : if g_use_ext_trigger generate
    s_counter <= 0;
    s_clk_1k  <= '0';
  end generate;

end rtl;
