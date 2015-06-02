library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.arria2_lvds_pkg.all;
use work.arria5_lvds_pkg.all;

entity altera_lvds_tx is
  generic(
    g_family : string);
  port(
    tx_core    : in  std_logic;
    tx_inclock : in  std_logic;
    tx_enable  : in  std_logic;
    tx_in      : in  std_logic_vector(7 downto 0);
    tx_out     : out std_logic);
end altera_lvds_tx;

architecture rtl of altera_lvds_tx is
  signal reg : std_logic_vector(7 downto 0);
  signal reg1 : std_logic_vector(7 downto 0);
begin

  main : process(tx_core) is
  begin
    if rising_edge(tx_core) then
      reg <= tx_in;
      reg1 <= reg;
    end if;
  end process;

  arria2 : if g_family = "Arria II" generate
    tx : arria2_lvds_tx
      port map(
        tx_inclock => tx_inclock,
        tx_enable  => tx_enable,
        tx_in      => reg1,
        tx_out(0)  => tx_out);
  end generate;
  
  arria5 : if g_family = "Arria V" generate
    tx : arria5_lvds_tx
      port map(
        tx_inclock => tx_inclock,
        tx_enable  => tx_enable,
        tx_in      => reg1,
        tx_out(0)  => tx_out);
  end generate;
  
end rtl;
