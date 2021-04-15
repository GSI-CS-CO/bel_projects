library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.arria2_lvds_pkg.all;
use work.arria5_lvds_pkg.all;
use work.arria10_lvds_pkg.all;

entity altera_lvds_tx_multi_scu4_wrap is
  generic(
    g_family : string);
  port(
    tx_core    : in  std_logic;
    tx_inclock : in  std_logic;
    tx_enable  : in  std_logic;
    tx_in      : in  std_logic_vector(23 downto 0);
    tx_out     : out std_logic_vector(2 downto 0));
end altera_lvds_tx_multi_scu4_wrap;

architecture rtl of altera_lvds_tx_multi_scu4_wrap is
  signal reg : std_logic_vector(23 downto 0);
begin

  main : process(tx_core) is
  begin
    if rising_edge(tx_core) then
      reg <= tx_in;
    end if;
  end process;

  arria10_scu4 : if g_family = "Arria 10 GX SCU4" generate
  tx : arria10_scu4_lvds_tx_multi
    port map(
      ext_coreclock => tx_core,
      ext_fclk      => tx_inclock,
      ext_loaden    => tx_enable,
      tx_coreclock  => open,
      tx_in         => reg,
      tx_out        => tx_out);
  end generate;

end rtl;
