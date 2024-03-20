library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.monster_pkg.all;
use work.a10ts_pkg.all;

entity a10ts is
  port(
    clk_i                  : in  std_logic := '0';
    rst_n_i                : in  std_logic := '1';
    clk_20m_i              : in  std_logic := '0';
    slave_i                : in  t_wishbone_slave_in;
    slave_o                : out t_wishbone_slave_out);
end a10ts;

architecture rtl of a10ts is
  constant c_divider : integer := 20;
  signal s_counter   : integer range 0 to c_divider - 1 := 0;
  signal r_ack       : std_logic := '0';
  signal r_dat       : std_logic_vector(31 downto 0) := (others => '0');
  signal r_temp      : std_logic_vector(31 downto 0) := (others => '0');
  signal s_clk_1m    : std_logic := '0';
  signal s_eoc       : std_logic := '0';
  signal s_eoc_latch : std_logic := '0';
  signal s_temp_out  : std_logic_vector(9 downto 0) := (others => '0');
begin

  slave_o.dat        <= r_dat;
  slave_o.ack        <= r_ack;
  slave_o.err        <= '0';
  slave_o.stall      <= '0';
  slave_o.rty        <= '0';

  core_a10ts_ip : a10ts_ip
  port map (
    corectl => '1',
    eoc     => s_eoc,
    reset   => not(rst_n_i),
    tempout => s_temp_out
  );

  main : process(clk_i, rst_n_i) is
  begin
    if rst_n_i = '0' then
      r_ack <= '0';
      r_dat <= (others => '0');
    elsif rising_edge(clk_i) then
      r_ack <= slave_i.cyc and slave_i.stb;
      r_dat <= (others => '0');
      if (slave_i.cyc and slave_i.stb) = '1' then
        r_dat <= r_temp;
      end if;
    end if;
  end process;

  update_temp : process(clk_i, rst_n_i) is
  begin
    if rst_n_i = '0' then
      r_temp      <= (others => '0');
      s_eoc_latch <= '0';
    elsif rising_edge(clk_i) then
      s_eoc_latch <= s_eoc;
      if (s_eoc_latch = '1' and s_eoc = '0') then -- TBD: This is not save
        r_temp(9 downto 0) <= s_temp_out;
      end if;
    end if;
  end process;

  gen_1m_clk : process(clk_20m_i, rst_n_i) is
  begin
    if rst_n_i = '0' then
      s_counter <= 0;
      s_clk_1m  <= '0';
    elsif rising_edge(clk_20m_i) then
      if s_counter = c_divider - 1 then
        s_counter <= 0;
        s_clk_1m  <= not s_clk_1m;
      else
        s_counter <= s_counter + 1;
      end if;
    end if;
  end process;

end rtl;
