library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

library work;

package sio3_sys_clk_local_clk_switch_pkg is




component sio3_sys_clk_local_clk_switch is
  generic (
    Base_Addr:  unsigned(15 downto 0)  := x"0040"
    );
  port(
    local_clk_i:          in    std_logic;
    sys_clk_i:            in    std_logic;
    nReset:               in    std_logic;
    master_clk_o:         out   std_logic;
    pll_locked:           out   std_logic;
		A_ME_12MHz:           out   std_logic;
    sys_clk_is_bad:       out   std_logic;
    sys_clk_is_bad_la:    out   std_logic;
    local_clk_is_bad:     out   std_logic;
    local_clk_is_running: out   std_logic;
    sys_clk_deviation:    out   std_logic;
    sys_clk_deviation_la: out   std_logic;
    Adr_from_SCUB_LA:     in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:    in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus
    Ext_Adr_Val:          in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:        in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:        in    std_logic;                      -- '1' => Wr-Cycle is active
    Rd_Port:              out   std_logic_vector(15 downto 0);  -- output for all read sources of this macro
    Rd_Activ:             out   std_logic;                      -- this acro has read data available at the Rd_Port.
    Dtack:                out   std_logic;
    signal_tap_clk_250mhz:   out   std_logic
    );
end component;


component sio3_local_clk_to_12p5_mhz is
  port
  (
    inclk0    : IN STD_LOGIC  := '0';
    c0        : OUT STD_LOGIC ;
    locked    : OUT STD_LOGIC 
  );
end component;


end package;
