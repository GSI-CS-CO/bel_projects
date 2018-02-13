library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.wishbone_pkg.all;
use work.genram_pkg.all;
use work.oled_display_pkg.all;
use work.oled_auto_pkg.all;

-- FIXME obsolete with oled.xml and wbgenplus


--Only low data byte is used in all operations
--Note: Form Feed (FF) 0x0c clears screen in mode 1 and 2

--Register Layout
--AddrMask: 0x3FFFF
--0x00000 Mode       Data Bits(1-0) 00 : idle
--                          01 : uart
--                          10 : char
--                          11 : raw  
--
--0x00004 Reset      Write resets FSMs and display controller
--
--0x00008 ColOffset     Offset of the first visible column. 0x23 for old, 0x30 for new controllers
--
--0x10000 UART        Written ascii code is put directly on screen
--
--0x20000 Char        Written ascii code is put on supplied location
--                Address Bits(8-6) Char location row
--                Address Bits(5-2) Char location col
--                  
--                   
--0x30000 Raw        Raw read/write to display memory. Organisation is column based, 
--                8px per column, 
--                Address Bits(12-2) Disp RAM address  


  
entity wb_console is
generic(
    FifoDepth : integer := 128);
  port(
    -- Common wishbone signals
    clk_i       : in  std_logic;
    nRst_i      : in  std_logic;
    -- Slave control port
    slave_i     : in  t_wishbone_slave_in;
    slave_o     : out t_wishbone_slave_out;
   
   -- General Purpose Signals common to all modes
   valid_o      : out std_logic;              --data from WBIF is valid        
   mode_o       : out std_logic_vector(1 downto 0); --display mode (idle, uart, char, raw)
   reset_disp_o : out std_logic;               --issue a reset of the display controller
   col_offset_o : out std_logic_vector(7 downto 0);
   
   --Raw Image
   raw_data_i   : in   std_logic_vector(7 downto 0);
   raw_data_o   : out std_logic_vector(7 downto 0);
   raw_addr_o   : out std_logic_vector(10 downto 0);
   raw_wren_o   : out std_logic;
   
   --Raw Char
   char_row_o   : out std_logic_vector(2 downto 0);
   char_col_o   : out std_logic_vector(3 downto 0); 
   
   --UART console  
   fifo_o      : out std_logic_vector(7 downto 0);
   empty_o      : out std_logic;
   
   read_i       : in   std_logic
   
   
  ); 
end wb_console;

architecture rtl of wb_console is

  constant fifo_width : natural := 26;
  signal s_mux_sel  :  std_logic_vector(2 downto 0);
  signal s_mode     :  std_logic_vector(1 downto 0); 
  signal s_full, s_push, s_empty, s_pop : std_logic;
  signal s_d, s_q : std_logic_vector(fifo_width-1 downto 0);

  signal s_slave_error_i    : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Error control
  signal s_slave_stall_i    : std_logic_vector(1-1 downto 0)  := (others => '0'); -- flow control
  signal s_slave_reset_o    : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Resets the OLED display
  signal s_slave_col_offs_o : std_logic_vector(8-1 downto 0)  := (others => '0'); -- first visible pixel column. 0x23 for old, 0x30 for new controllers
  signal s_slave_uart_WR_o  : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Write enable flag - uart
  signal s_slave_uart_o     : std_logic_vector(8-1 downto 0)  := (others => '0'); -- UART input FIFO. Ascii on b7..0
  signal s_slave_char_WR_o  : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Write enable flag - char
  signal s_slave_char_o     : std_logic_vector(20-1 downto 0) := (others => '0'); -- Char input FIFO. Row b18..16, Col b11..8, Ascii b7..0
  signal s_slave_raw_WR_o   : std_logic_vector(1-1 downto 0)  := (others => '0'); -- Write enable flag - raw
  signal s_slave_raw_o      : std_logic_vector(20-1 downto 0) := (others => '0'); -- Raw  input FIFO. Disp RAM Adr b18..8, Pixel (Col) b7..0
  signal s_rst_n : std_logic;
  constant dummy : std_logic_vector (fifo_width-1 downto 0) := (others => '0');

begin
  s_rst_n <= nRst_i and not s_slave_reset_o(0);

  INST_oled_auto : oled_auto
  port map (
    clk_sys_i   => clk_i,
    rst_sys_n_i => nRst_i,
    error_i     => s_slave_error_i,
    stall_i     => s_slave_stall_i,
    reset_o     => s_slave_reset_o,
    col_offs_o  => s_slave_col_offs_o,
    uart_WR_o   => s_slave_uart_WR_o,
    uart_o      => s_slave_uart_o,
    char_WR_o   => s_slave_char_WR_o,
    char_o      => s_slave_char_o,
    raw_WR_o    => s_slave_raw_WR_o,
    raw_o       => s_slave_raw_o,
    slave_i     => slave_i,
    slave_o     => slave_o  );

  fifo : generic_sync_fifo
    generic map(
      g_data_width => fifo_width,
      g_size       => FifoDepth,
      g_show_ahead => true)
    port map(
      rst_n_i        => s_rst_n,
      clk_i          => clk_i,
      d_i            => s_d,
      we_i           => s_push,
      q_o            => s_q,
      rd_i           => s_pop,
      empty_o        => s_empty,
      full_o         => s_full);

  s_slave_stall_i(0) <= s_full;
  
  s_pop           <= not s_empty and read_i;
  s_push          <= s_slave_raw_WR_o(0) or s_slave_char_WR_o(0) or s_slave_uart_WR_o(0);
  s_mux_sel       <= s_slave_raw_WR_o & s_slave_char_WR_o & s_slave_uart_WR_o;



  --fifo input mux
  inmux : with s_mux_sel select
  s_d   <=  cDISPMODE_UART & dummy(23 downto 8)   & s_slave_uart_o  when "001",
            cDISPMODE_CHAR & dummy(23 downto 20)  & s_slave_char_o  when "010",
            cDISPMODE_RAW  & dummy(23 downto 20)  & s_slave_raw_o   when "100",
            cDISPMODE_UART & dummy(23 downto 8)   & s_slave_uart_o  when others;
  
  -- Output pins
  valid_o       <= not s_empty;
  mode_o        <= s_q(25 downto 24);
  reset_disp_o  <= s_slave_reset_o(0);
  col_offset_o  <= s_slave_col_offs_o;
  
  --<= raw_data_i
  raw_data_o    <= s_q(7 downto 0);
  raw_addr_o    <= s_q(18 downto 8);
  raw_wren_o    <= s_q(25) and s_q(24);

  char_row_o    <= s_q(14 downto 12);
  char_col_o    <= s_q(11 downto 8);

  fifo_o        <= s_q(7 downto 0);

  empty_o       <= s_empty;
  
end rtl;