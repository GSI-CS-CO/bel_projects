-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library altera_mf;
use altera_mf.all;

-- wishbone/gsi/cern
library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;

-- package declaration
package lvds_pll_pkg is

  -- lvds support pll for arria2 (vetar2 and scu2/3)
  component lvds_pll
    port ( 
      areset : in  std_logic := '0';
      inclk0 : in  std_logic := '0';
      c0     : out std_logic;
      c1     : out std_logic;
      c2     : out std_logic;
      locked : out std_logic
    );
  end component;

end lvds_pll_pkg;
