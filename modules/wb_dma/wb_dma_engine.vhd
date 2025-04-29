library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wb_dma_pkg.all;
use work.gencores_pkg.all;

entity wb_dma_engine is
  port(
    clk_i : in std_logic;
    rstn_i : in std_logic;

    -- communication signals
    dma_active_i : in std_logic;
    descriptor_active_i : in std_logic;

    master_idle_o : out std_logic
  );
end entity;

architecture behavioral of wb_dma_engine is

begin

controller : process (clk_i, rstn_i)
begin

end process;


end architecture;
