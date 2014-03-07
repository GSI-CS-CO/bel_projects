library IEEE;
--! Standard packages
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;


package wr_serialtimestamp_pkg is 


component BuTiS_T0_generator is
  generic(
		g_clockcyclesperbit                      : integer := 4);
  port(
		wr_clock_i                             : in  std_logic; -- 125MHz
		wr_rst_n_i                             : in  std_logic;
		wr_PPSpulse_i                          : in  std_logic; -- 1s
		BuTis_rst_n_i                          : in std_logic; -- /reset
		timestamp_i                            : in  std_logic_vector(63 downto 0);
		BuTis_C2_i                             : in std_logic; -- 200MHz
		BuTis_T0_o                             : out std_logic; -- 100kHz
		BuTis_T0_timestamp_o                   : out std_logic; -- 100kHz with timestamp
		error_o                                : out std_logic);
end component;

component TimestampEncoder is
  generic(
		g_clockcyclesperbit                      : integer := 4);
  port(
		BuTis_C2_i                               : in  std_logic;
		BuTis_T0_i                               : in  std_logic;
		reset_i                                  : in  std_logic;
		timestamp_on_next_T0_i                   : in  std_logic_vector(63 downto 0);
		serial_o                                 : out std_logic;
		error_o                                  : out std_logic);
end component;

component TimestampDecoder is
  generic(
	g_clockcyclesperbit                      : integer := 4;
	g_BuTis_T0_precision                     : integer := 3);
  port(
	BuTis_C2_i                               : in std_logic;
	reset_i                                  : in std_logic;
	serial_i                                 : in std_logic;
	BuTis_T0_o                               : out std_logic;
	timestamp_o                              : out std_logic_vector(63 downto 0);
	uncertain_o                              : out std_logic;
	error_o                                  : out std_logic);
end component;

  
end package;
