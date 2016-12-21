library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.wishbone_pkg.all;
use work.genram_pkg.all;
use work.wbgenplus_pkg.all;
use work.genram_pkg.all;
use work.oled_auto_pkg.all;
use work.oled_display_pkg.all;

entity wb_console is
  port(
    -- Common wishbone signals
    clk_i       	: in  std_logic;
    nRst_i     	: in  std_logic;
    -- Slave control port
    slave_i     	: in  t_wishbone_slave_in;
    slave_o     	: out t_wishbone_slave_out;
	 
	 -- General Purpose Signals common to all modes
	 valid_o			 : out std_logic;							--data from WBIF is valid				
	 mode_o			   : out std_logic_vector(1 downto 0); --display mode (idle, uart, char, raw)
	 reset_disp_o  : out std_logic; 							--issue a reset of the display controller
	 col_offset_o  : out std_logic_vector(7 downto 0);
	 
	 --Raw Image
	 raw_data_i		: in 	std_logic_vector(7 downto 0);
	 raw_data_o 	: out std_logic_vector(7 downto 0) ;
	 raw_addr_o 	: out std_logic_vector(10 downto 0);
	 raw_wren_o		: out std_logic;
	 
	 --Raw Char
	 char_row_o		: out std_logic_vector(2 downto 0);
	 char_col_o 	: out std_logic_vector(3 downto 0); 
	 
	 --UART console	
    fifo_o			: out std_logic_vector(7 downto 0);
	 empty_o			: out std_logic;
	 
	 read_i			: in 	std_logic
	 
	 
	); 
end wb_console;

architecture rtl of wb_console is

  signal s_slave_error_i    : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Error control
  signal s_slave_stall_i    : std_logic_vector(1-1 downto 0)  := (others => '0'); -- flow control
  signal s_slave_reset_o    : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Resets the OLED display
  signal s_slave_col_offs_o : std_logic_vector(8-1 downto 0)  := (others => '0'); -- first visible pixel column. 0x23 for old, 0x30 for new controllers
  signal s_slave_uart_WR_o  : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Write enable flag - uart
  signal s_slave_uart_o     : std_logic_vector(8-1 downto 0)  := (others => '0'); -- UART input FIFO. Ascii on b7..0
  signal s_slave_char_WR_o  : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Write enable flag - char
  signal s_slave_char_o     : std_logic_vector(14-1 downto 0) := (others => '0'); -- Char input FIFO. Row b13..11, Col b10..8, Ascii b7..0
  signal s_slave_raw_WR_o   : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Write enable flag - raw
  signal s_slave_raw_o      : std_logic_vector(19-1 downto 0) := (others => '0'); -- Raw  input FIFO. Disp RAM Adr b18..8, Pixel (Col) b7..0
  


  signal r_rdy, s_valid, r_valid, r_read	:	std_logic;
  signal s_mode		              :	std_logic_vector(1 downto 0);


begin
  
   INST_oled_auto : oled_auto
  port map (
    clk_sys_i   => clk_i,
    rst_sys_n_i => nRst_i,
    error_i     => s_slave_error_i,
    stall_i     => s_slave_stall_i,
    reset_o     => s_slave_reset_o,
    col_offs_o  => col_offset_o,
    uart_WR_o   => s_slave_uart_WR_o,
    uart_o      => s_slave_uart_o,
    char_WR_o   => s_slave_char_WR_o,
    char_o      => s_slave_char_o,
    raw_WR_o    => s_slave_raw_WR_o,
    raw_o       => s_slave_raw_o,
    slave_i     => slave_i,
    slave_o     => slave_o  );  

  reset_disp_o <= s_slave_reset_o(0);
  s_mode <= cDISPMODE_UART when (s_slave_uart_WR_o = "1")
  else      cDISPMODE_CHAR when (s_slave_char_WR_o = "1")
  else      cDISPMODE_RAW  when (s_slave_raw_WR_o  = "1")
  else      cDISPMODE_NONE ;

  s_valid <= '1' when s_mode /= cDISPMODE_NONE
        else '0';

  valid_o <= s_valid;
  empty_o <= not r_rdy when ((s_mode = cDISPMODE_UART) or (s_mode = cDISPMODE_CHAR)) 
    else '1';

  fifo_o     <= s_slave_uart_o(7  downto  0) when (s_mode = cDISPMODE_UART)
           else s_slave_char_o(7  downto  0);
	char_col_o <= s_slave_char_o(10 downto  8);
  char_row_o <= s_slave_char_o(13 downto 11);
  raw_data_o <=  s_slave_raw_o(7  downto  0);
  raw_addr_o <=  s_slave_raw_o(18 downto  8);
	mode_o     <= s_mode;

									
  main : process(clk_i)
	begin
    if rising_edge(clk_i) then
      if nRst_i = '0' then
        s_slave_stall_i <= "1";
        raw_wren_o      <= '0';
        r_valid         <= '0';
        r_read          <= '0';
        r_rdy           <= '0';
      else
        s_slave_stall_i <= "1";
        r_read          <= read_i or r_read; -- save if there were char read requests from display controller
        raw_wren_o      <= '0';              
        r_valid         <= s_valid;          -- register valid signal from wb core
        
        if((r_read = '1' or read_i = '1' or s_mode = cDISPMODE_RAW) and r_rdy = '1') then
          -- proceed
          r_rdy           <= '0';
          r_valid         <= '0';
          r_read          <= '0'; 
          s_slave_stall_i <= "0"; -- lift stall for one cycle
          if (s_mode = cDISPMODE_RAW) then
            raw_wren_o      <= '1'; -- if it was a raw access, set raw write enable
          end if;
        else
          -- wait
          r_rdy <= r_rdy or r_valid; -- wait two cycles for display controller              
        end if;
      end if;  
    end if;
  end process;

end rtl;
