library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

package lpc_uart_pkg is

  component lpc_uart is
	port (
		lpc_clk:		in std_logic;
		lpc_serirq:		inout std_logic;
		lpc_ad:			inout std_logic_vector(3 downto 0);
		lpc_frame_n:		in std_logic;
		lpc_reset_n:		in std_logic;
    
    kbc_out_port:   out std_logic_vector(7 downto 0);
    kbc_in_port:   in std_logic_vector(7 downto 0);

		serial_rxd:		in std_logic;
		serial_txd:		out std_logic;
		serial_dtr:		out std_logic;
		serial_dcd:		in std_logic;
		serial_dsr:		in std_logic;
		serial_ri:		in std_logic;
		serial_cts:		in std_logic;
		serial_rts:		out std_logic;
		
		      
		seven_seg_L:	out std_logic_vector(7 downto 0);    -- SSeg Data output
		seven_seg_H:	out std_logic_vector(7 downto 0)    -- SSeg Data output  
	
	);
  end component;

end lpc_uart_pkg;
