library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

library work;

package simple_tag_decoder_pkg is

component simple_tag_decoder is
  generic (
    start_tag:            std_logic_vector(31 downto 0)
    );
  port (
    clk_i:              in    std_logic;                     
    nrst_i:             in    std_logic;

    -- SCU_bus_slave interface
    Timing_Pattern_LA_i:   in    std_logic_vector(31 downto 0);  -- connect to SCU_bus_slave
    Timing_Pattern_RCV_i:  in    std_logic;                      -- connect to SCU_bus_slave
    -- fg_quad_scu_bus interface
    start_o:              out   std_logic                       -- connect to fg_quad_scu_bus

    );
end component;

end package;
