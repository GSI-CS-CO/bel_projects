library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.arria5_lvds_pkg.all;

entity altera_lvds_tx is
  generic(
    g_family : string);
  port(
    tx_inclock : in  std_logic;
    tx_enable  : in  std_logic;
    tx_in      : in  std_logic_vector(7 downto 0);
    tx_out     : out std_logic);
end altera_lvds_tx;

architecture rtl of altera_lvds_tx is
begin

  arria5 : if g_family = "Arria V" generate
    tx : arria5_lvds_tx
      port map(
        tx_inclock => tx_inclock,
        tx_enable  => tx_enable,
        tx_in      => tx_in,
        tx_out(0)  => tx_out);
  end generate;
  
end rtl;
