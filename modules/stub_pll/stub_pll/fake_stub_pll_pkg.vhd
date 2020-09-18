library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package fake_stub_pll_pkg is

  component stub_pll is
    port (
      rst      : in  std_logic := 'X'; -- reset
      refclk   : in  std_logic := 'X'; -- clk
      locked   : out std_logic;        -- export
      outclk_0 : out std_logic;        -- clk
      outclk_1 : out std_logic;        -- clk
      outclk_2 : out std_logic;        -- clk
      outclk_3 : out std_logic         -- clk
    );
  end component;

end stub_pll_pkg;
