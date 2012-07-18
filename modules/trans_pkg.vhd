library ieee;
use ieee.std_logic_1164.all;

library work;

package trans_pkg is

component trans_loop
  GENERIC (
    starting_channel_number : integer
    );
	PORT
	(
		cal_blk_clk		: IN STD_LOGIC ;
		pll_inclk		: IN STD_LOGIC ;
		reconfig_clk		: IN STD_LOGIC ;
		reconfig_togxb		: IN STD_LOGIC_VECTOR (3 DOWNTO 0);
		rx_datain		: IN STD_LOGIC_VECTOR (3 DOWNTO 0);
		rx_digitalreset		: IN STD_LOGIC_VECTOR (3 DOWNTO 0);
		rx_seriallpbken		: IN STD_LOGIC_VECTOR (3 DOWNTO 0);
		tx_digitalreset		: IN STD_LOGIC_VECTOR (3 DOWNTO 0);
		reconfig_fromgxb		: OUT STD_LOGIC_VECTOR (16 DOWNTO 0);
		rx_bistdone		: OUT STD_LOGIC_VECTOR (3 DOWNTO 0);
		rx_bisterr		: OUT STD_LOGIC_VECTOR (3 DOWNTO 0);
		rx_clkout		: OUT STD_LOGIC_VECTOR (3 DOWNTO 0);
		rx_signaldetect		: OUT STD_LOGIC_VECTOR (3 DOWNTO 0);
		tx_clkout		: OUT STD_LOGIC_VECTOR (3 DOWNTO 0);
		tx_dataout		: OUT STD_LOGIC_VECTOR (3 DOWNTO 0)
	);
end component;

component trans_rcfg
	PORT
	(
		reconfig_clk		: IN STD_LOGIC ;
		reconfig_fromgxb		: IN STD_LOGIC_VECTOR (50 DOWNTO 0);
		busy		: OUT STD_LOGIC ;
		reconfig_togxb		: OUT STD_LOGIC_VECTOR (3 DOWNTO 0)
	);
end component;

component trans_pll
	PORT
	(
		inclk0		: IN STD_LOGIC  := '0';
		c0		: OUT STD_LOGIC ;
		locked		: OUT STD_LOGIC 
	);
end component;

component reverse_lpb
  GENERIC (
    starting_channel_number : integer
    );
	PORT
	(
		cal_blk_clk		: IN STD_LOGIC ;
		pll_inclk		: IN STD_LOGIC ;
		reconfig_clk		: IN STD_LOGIC ;
		reconfig_togxb		: IN STD_LOGIC_VECTOR (3 DOWNTO 0);
		rx_datain		: IN STD_LOGIC_VECTOR (3 DOWNTO 0);
		rx_digitalreset		: IN STD_LOGIC_VECTOR (3 DOWNTO 0);
		rx_enapatternalign		: IN STD_LOGIC_VECTOR (3 DOWNTO 0);
		tx_datain		: IN STD_LOGIC_VECTOR (79 DOWNTO 0);
		tx_digitalreset		: IN STD_LOGIC_VECTOR (3 DOWNTO 0);
		reconfig_fromgxb		: OUT STD_LOGIC_VECTOR (16 DOWNTO 0);
		rx_clkout		: OUT STD_LOGIC_VECTOR (3 DOWNTO 0);
		rx_dataout		: OUT STD_LOGIC_VECTOR (79 DOWNTO 0);
		tx_clkout		: OUT STD_LOGIC_VECTOR (3 DOWNTO 0);
		tx_dataout		: OUT STD_LOGIC_VECTOR (3 DOWNTO 0)
	);
end component;


end package;