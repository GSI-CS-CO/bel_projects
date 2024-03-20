library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.monster_pkg.all;

entity a10ts is
  port(
    clk_i                  : in  std_logic := '0';
    rst_n_i                : in  std_logic := '1';
    clk_20m_i              : in  std_logic := '0';
    slave_i                : in  t_wishbone_slave_in;
    slave_o                : out t_wishbone_slave_out;
end a10ts;

architecture rtl of a10ts is
  constant c_divider : integer := 20;
  signal s_counter   : integer range 0 to c_divider - 1 := 0;
  signal r_ack       : std_logic := '0';
  signal r_dat       : std_logic_vector(31 downto 0) := (others => '0');
  signal r_temp      : std_logic_vector(31 downto 0) := (others => '0');
  signal s_clk_1m    : std_logic := '0';
begin

  slave_o.dat        <= r_dat;
  slave_o.ack        <= r_ack;
  slave_o.err        <= '0';
  slave_o.stall      <= '0';
  slave_o.rty        <= '0';

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
      r_temp <= (others => '0');
    elsif rising_edge(clk_i) then
      r_temp <= (others => '1');
    end if;
  end process;

  gen_1m_clk : process(clk_20m_i, rst_n_i) is
  begin
    if rst_n_i = '0' then
      s_counter <= 0;
      s_clk_1m  <= '0';
    elsif rising_edge(clk_20m_i) then
      if counter = DIVIDER_VALUE - 1 then
        s_counter <= 0;
        s_clk_1m  <= not s_clk_1m;
      else
        s_counter <= s_counter + 1;
      end if;
    end if;
  end process;

end rtl;
