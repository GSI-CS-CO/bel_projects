library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.arria2_lvds_pkg.all;
use work.arria5_lvds_pkg.all;
use work.arria10_lvds_pkg.all;

entity altera_lvds_rx_multi_scu4_wrap is
  generic(
    g_family : string);
  port(
    rx_core    : in  std_logic;
    rx_inclock : in  std_logic;
    rx_enable  : in  std_logic;
    rx_in      : in  std_logic_vector(2 downto 0);
    rx_out     : out std_logic_vector(23 downto 0));
end altera_lvds_rx_multi_scu4_wrap;

architecture rtl of altera_lvds_rx_multi_scu4_wrap is
  signal raw : std_logic_vector(23 downto 0);
begin

  main : process(rx_core) is
  begin
    if rising_edge(rx_core) then
      rx_out <= raw;
    end if;
  end process;

  arria10_scu4 : if g_family = "Arria 10 GX SCU4" generate
  rx : arria10_scu4_lvds_rx_multi
    port map(
      ext_coreclock       => rx_core,
      ext_fclk            => rx_inclock,
      ext_loaden          => rx_enable,
      rx_in               => rx_in,
      rx_out(23 downto 0) => raw); -- Weird patch, according to Quartus this has more than 24 bits
  end generate;

end rtl;
