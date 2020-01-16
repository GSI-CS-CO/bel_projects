library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.monster_pkg.all;

entity beam_dump is
  port(
    clk_i      : in  std_logic;
    rst_n_i    : in  std_logic;
    slave_i    : in  t_wishbone_slave_in;
    slave_o    : out t_wishbone_slave_out);
end beam_dump;

architecture rtl of beam_dump is
  signal r_ack  : std_logic := '0';
  signal r_dat  : t_wishbone_data := (others => '0');

  signal mem_reg_000 : std_logic_vector(31 downto 0) := (others => '0');
  signal mem_reg_001 : std_logic_vector(31 downto 0) := (others => '0');
  signal mem_reg_010 : std_logic_vector(31 downto 0) := (others => '0');
  signal mem_reg_011 : std_logic_vector(31 downto 0) := (others => '0');
  signal mem_reg_100 : std_logic_vector(31 downto 0) := (others => '0');
  signal mem_reg_101 : std_logic_vector(31 downto 0) := (others => '0');
  signal mem_reg_110 : std_logic_vector(31 downto 0) := (others => '0');
  signal mem_reg_111 : std_logic_vector(31 downto 0) := (others => '0');

begin

  slave_o.dat   <= r_dat;
  slave_o.ack   <= r_ack;
  slave_o.err   <= '0';
  slave_o.stall <= '0';

  -- to be removed:
  slave_o.int <= '0';
  slave_o.rty <= '0';

  main : process(clk_i, rst_n_i) is
  begin
    if rst_n_i = '0' then
      r_ack  <= '0';
      r_dat  <= (others => '0');

    elsif rising_edge(clk_i) then
      r_ack <= slave_i.cyc and slave_i.stb;
      r_dat <= (others => '0');

      if (slave_i.cyc and slave_i.stb and slave_i.we) = '1' then
        case slave_i.adr(4 downto 2) is
          when "000" => mem_reg_000 <= slave_i.dat;
          when "001" => mem_reg_001 <= slave_i.dat;
          when "010" => mem_reg_010 <= slave_i.dat;
          when "011" => mem_reg_011 <= slave_i.dat;
          when "100" => mem_reg_100 <= slave_i.dat;
          when "101" => mem_reg_101 <= slave_i.dat;
          when "110" => mem_reg_110 <= slave_i.dat;
          when "111" => mem_reg_111 <= slave_i.dat;
          when others => null;
        end case;
      end if;

      case slave_i.adr(4 downto 2) is
        when "000" => r_dat <= mem_reg_000;
        when "001" => r_dat <= mem_reg_001;
        when "010" => r_dat <= mem_reg_010;
        when "011" => r_dat <= mem_reg_011;
        when "100" => r_dat <= mem_reg_100;
        when "101" => r_dat <= mem_reg_101;
        when "110" => r_dat <= mem_reg_110;
        when "111" => r_dat <= mem_reg_111;
        when others => null;
      end case;
    end if;
  end process;

end rtl;
