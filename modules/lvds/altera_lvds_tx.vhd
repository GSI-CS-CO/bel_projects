library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.arria2_lvds_pkg.all;
use work.arria5_lvds_pkg.all;
use work.arria10_lvds_pkg.all;

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
begin

  main : process(tx_core) is
  begin
    if rising_edge(tx_core) then
      reg <= tx_in;
    end if;
  end process;

  arria2 : if g_family = "Arria II" generate
    tx : arria2_lvds_tx
      port map(
        tx_inclock => tx_inclock,
        tx_enable  => tx_enable,
        tx_in      => reg,
        tx_out(0)  => tx_out);
  end generate;
  
  arria5 : if g_family = "Arria V" generate
    tx : arria5_lvds_tx
      port map(
        tx_inclock => tx_inclock,
        tx_enable  => tx_enable,
        tx_in      => reg,
        tx_out(0)  => tx_out);
  end generate;

  arria10_scu4 : if g_family = "Arria 10 GX SCU4" generate
  tx : arria10_scu4_lvds_tx
    port map(
      ext_coreclock => tx_core,
      ext_fclk      => tx_inclock,
      ext_loaden    => tx_enable,
      tx_coreclock  => open,
      tx_in         => reg,
      tx_out(0)     => tx_out);
  end generate;

  arria10_pex10 : if g_family = "Arria 10 GX PEX10" generate
  tx : arria10_pex10_lvds_tx
    port map(
      ext_coreclock => tx_core,
      ext_fclk      => tx_inclock,
      ext_loaden    => tx_enable,
      tx_coreclock  => open,
      tx_in         => reg,
      tx_out(0)     => tx_out);
  end generate;

  arria10_ftm10 : if g_family = "Arria 10 GX FTM10" generate
  tx : arria10_ftm10_lvds_tx
    port map(
      ext_coreclock => tx_core,
      ext_fclk      => tx_inclock,
      ext_loaden    => tx_enable,
      tx_coreclock  => open,
      tx_in         => reg,
      tx_out(0)     => tx_out);
  end generate;

end rtl;
