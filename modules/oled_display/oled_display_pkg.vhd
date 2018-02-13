library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.oled_auto_pkg.all;

package oled_display_pkg is

  subtype byte_t is std_logic_vector(7 downto 0);
  type bytearray is array (integer range <>) of byte_t;
    constant adr_sequence          : bytearray(0 to 2)              := (  x"B0", x"00", x"1F");
    constant scroll_start_sequence : bytearray(0 to 6)              := (  x"29", x"00", x"00", x"00", x"00", x"01", x"2F");
    constant scroll_stop_sequence  : bytearray(0 to 1)              := (  x"00", x"2E" );
    constant init_sequence         : bytearray(0 to 11)             := (  x"A1", x"C8", x"00", x"81",
                                                                          x"50", x"AC", x"82", x"F0",
                                                                          x"00", x"00", x"00", x"AF");
        
    constant c_Char_LF             : std_logic_vector(7 downto 0)   := x"0a";
    constant c_Char_CR             : std_logic_vector(7 downto 0)   := x"0d";
    constant c_Char_FF             : std_logic_vector(7 downto 0)   := x"0c";
        
    constant c_col_width           : unsigned(7 downto 0)           := x"41";
    constant c_firstBank           : unsigned(2 downto 0)           := "000";
    constant c_firstVisibleBank    : unsigned(2 downto 0)           := "010";
    constant c_lastBank            : unsigned(2 downto 0)           := "111";
        
    constant cDISPMODE_UART        : std_logic_vector(1 downto 0)   := "01";
    constant cDISPMODE_CHAR        : std_logic_vector(1 downto 0)   := "10";
    constant cDISPMODE_RAW         : std_logic_vector(1 downto 0)   := "11";
        
    constant c_WAIT_CYCLES_RST     : unsigned(32 downto 0)          := to_unsigned(30000, 32+1);

    constant Pix_Dis_INT           : std_logic_vector(15 downto 0)  := x"0084";  -- for 132 pixels.

    type STATES is (IDLE_STATE, RESET_STATE, INI_STATE, START_DATA_STATE, ADR_STATE, TRN_ADR_STATE, FETCH_DATA_STATE, TRN_DATA_STATE,
                      BANK_DATA_DONE_STATE, REFRESH_RAM_STATE, SCROLL_START, SCROLL_WAIT, SCROLL_STOP);
    type RFSTATES is (IDLE, RAW_DATA, GET_CHAR, COMMAND_CHAR, CLEAR_BANKS, START_RENDER, RENDER, UPDATE_DISPLAY_START, UPDATE_DISPLAY);

    type pixmap is array(integer range 0 to 4) of byte_t;
    type char_map is array(integer range 0 to 117) of pixmap;
    constant mymap : char_map := (
    (x"00", x"00", x"00", x"00", x"00"),    --#Space
    (x"00", x"00", x"f2", x"00", x"00"),    --#exclamation
    (x"00", x"C0", x"00", x"C0", x"00"),    --#dquotes
    (x"68", x"38", x"6c", x"38", x"2c"),    --#suqare
    (x"48", x"54", x"fe", x"54", x"24"),    --#dollar
    (x"46", x"26", x"10", x"c8", x"c4"),    --#percent
    (x"0a", x"44", x"aa", x"92", x"6c"),    --#ampersand
    (x"00", x"00", x"c0", x"00", x"00"),    --#quote
    (x"00", x"82", x"44", x"38", x"00"),    --#bracketl
    (x"00", x"38", x"44", x"82", x"00"),    --#bracketr
    (x"28", x"10", x"7c", x"10", x"28"),    --#star
    (x"10", x"10", x"7c", x"10", x"10"),    --#plus
    (x"00", x"00", x"0C", x"02", x"00"),    --#comma
    (x"10", x"10", x"10", x"10", x"10"),    --#minus
    (x"00", x"00", x"06", x"06", x"00"),    --#period
    (x"40", x"20", x"10", x"08", x"04"),    --#slash
    (x"7c", x"a2", x"92", x"8a", x"7c"),    --#0
    (x"00", x"02", x"fe", x"42", x"00"),    --#1
    (x"62", x"92", x"8a", x"86", x"42"),    --#2
    (x"8c", x"d2", x"a2", x"82", x"84"),    --#3
    (x"08", x"fe", x"48", x"28", x"18"),    --#4
    (x"9c", x"a2", x"a2", x"a2", x"e4"),    --#5
    (x"0c", x"92", x"92", x"52", x"3c"),    --#6
    (x"c0", x"a0", x"90", x"8e", x"80"),    --#7
    (x"6c", x"92", x"92", x"92", x"6c"),    --#8
    (x"78", x"94", x"92", x"92", x"60"),    --#9
    (x"00", x"00", x"6C", x"00", x"00"),    --#colon
    (x"00", x"00", x"6C", x"02", x"00"),    --#semicolon
    (x"00", x"82", x"44", x"28", x"10"),    --#lessthan
    (x"28", x"28", x"28", x"28", x"28"),    --#equals
    (x"10", x"28", x"44", x"82", x"00"),    --#greaterthan
    (x"60", x"90", x"8a", x"80", x"40"),    --#question
    (x"78", x"AA", x"9A", x"82", x"7C"),    --#@
    (x"7e", x"88", x"88", x"88", x"7e"),    --#A
    (x"6c", x"92", x"92", x"92", x"fe"),    --#B
    (x"44", x"82", x"82", x"82", x"7c"),    --#C
    (x"38", x"44", x"82", x"82", x"fe"),    --#D
    (x"82", x"92", x"92", x"92", x"fe"),    --#E
    (x"80", x"90", x"90", x"90", x"fe"),    --#F
    (x"5e", x"92", x"92", x"82", x"7c"),    --#G
    (x"fe", x"10", x"10", x"10", x"fe"),    --#H
    (x"00", x"82", x"fe", x"82", x"00"),    --#I
    (x"80", x"fc", x"82", x"02", x"04"),    --#J
    (x"82", x"44", x"28", x"10", x"fe"),    --#K
    (x"02", x"02", x"02", x"02", x"fe"),    --#L
    (x"fe", x"40", x"30", x"40", x"fe"),    --#M
    (x"fe", x"08", x"10", x"20", x"fe"),    --#N
    (x"7c", x"82", x"82", x"82", x"7c"),    --#O
    (x"60", x"90", x"90", x"90", x"fe"),    --#P
    (x"7a", x"84", x"8a", x"82", x"7c"),    --#Q
    (x"62", x"94", x"98", x"90", x"fe"),    --#R
    (x"8c", x"92", x"92", x"92", x"62"),    --#S
    (x"80", x"80", x"fe", x"80", x"80"),    --#T
    (x"fc", x"02", x"02", x"02", x"fc"),    --#U
    (x"f8", x"04", x"02", x"04", x"f8"),    --#V
    (x"fc", x"02", x"1c", x"02", x"fc"),    --#W
    (x"c6", x"28", x"10", x"28", x"c6"),    --#X
    (x"e0", x"10", x"0e", x"10", x"e0"),    --#Y
    (x"c2", x"a2", x"92", x"8a", x"86"),    --#Z
    (x"00", x"00", x"00", x"82", x"fe"),    --#sqbracketl
    (x"06", x"08", x"10", x"20", x"c0"),    --#backslash
    (x"fe", x"82", x"00", x"00", x"00"),    --#sqbracketr
    (x"20", x"40", x"80", x"40", x"20"),    --#caret
    (x"02", x"02", x"02", x"02", x"02"),    --#underscore
    (x"00", x"20", x"C0", x"00", x"00"),    --#leftquote
    (x"1e", x"2a", x"2a", x"2a", x"04"),    --#a
    (x"1c", x"22", x"22", x"12", x"fe"),    --#b
    (x"04", x"22", x"22", x"22", x"1c"),    --#c
    (x"fe", x"12", x"22", x"22", x"1c"),    --#d
    (x"18", x"2a", x"2a", x"2a", x"1c"),    --#e
    (x"40", x"80", x"90", x"7e", x"10"),    --#f
    (x"7c", x"4a", x"4a", x"4a", x"30"),    --#g
    (x"1e", x"20", x"20", x"10", x"fe"),    --#h
    (x"00", x"02", x"be", x"22", x"00"),    --#i
    (x"00", x"bc", x"22", x"02", x"04"),    --#j
    (x"00", x"22", x"14", x"08", x"fe"),    --#k
    (x"00", x"02", x"fe", x"82", x"00"),    --#l
    (x"1e", x"20", x"18", x"20", x"3e"),    --#m
    (x"1e", x"20", x"20", x"10", x"3e"),    --#n
    (x"1c", x"22", x"22", x"22", x"1c"),    --#o
    (x"10", x"28", x"28", x"28", x"3e"),    --#p
    (x"3e", x"18", x"28", x"28", x"10"),    --#q
    (x"10", x"20", x"20", x"10", x"3e"),    --#r
    (x"04", x"2a", x"2a", x"2a", x"12"),    --#s
    (x"04", x"02", x"22", x"fc", x"20"),    --#t
    (x"3e", x"04", x"02", x"02", x"3c"),    --#u
    (x"38", x"04", x"02", x"04", x"38"),    --#v
    (x"3c", x"02", x"0c", x"02", x"3c"),    --#w
    (x"22", x"14", x"08", x"14", x"22"),    --#x
    (x"3c", x"0a", x"0a", x"0a", x"30"),    --#y
    (x"22", x"32", x"2a", x"26", x"22"),    --#z
    (x"82", x"82", x"7C", x"10", x"10"),    --#curlyBracketL
    (x"00", x"00", x"FE", x"00", x"00"),    --#Pipe
    (x"7C", x"10", x"10", x"82", x"82"),    --#curlyBracketR
    (x"10", x"08", x"18", x"10", x"08"),    --#Tilde
    (x"00", x"00", x"00", x"00", x"00"),    --#del
    --- End of continuous ASCII table, begin special char cherry pick
    (x"00", x"0E", x"0A", x"0E", x"00"),    --#128# degree
    (x"1e", x"A8", x"28", x"A8", x"1e"),    --#129#
    (x"1c", x"A2", x"22", x"A2", x"1c"),    --#130#
    (x"3e", x"82", x"02", x"82", x"3e"),    --#131#
    (x"0c", x"52", x"b2", x"80", x"7e"),    --#132#
    (x"3e", x"84", x"02", x"82", x"3c"),    --#133#
    (x"1e", x"aa", x"2a", x"aa", x"04"),    --#134#
    (x"1c", x"A2", x"22", x"A2", x"1c"),    --#135#
    (x"7A", x"86", x"80", x"86", x"7A"),    --#136#Omega
    (x"3e", x"04", x"02", x"02", x"3f"),    --#137#Mu
    
    (x"00", x"28", x"10", x"28", x"00"),    --#138#times
    (x"10", x"10", x"54", x"10", x"10"),    --#139#divide
    (x"10", x"38", x"54", x"10", x"10"),    --#140#arrowl
    (x"10", x"10", x"54", x"38", x"10"),    --#141#arrowr
    (x"fe", x"fe", x"fe", x"fe", x"fe"),    --#142#block
    (x"ff", x"ff", x"ff", x"ff", x"ff"),    --#143#fullblock
    (x"12", x"12", x"7E", x"12", x"12"),    --#144#plusminus
    
    
    (x"00", x"00", x"FE", x"00", x"00"),    --#142#menuUD
    (x"00", x"00", x"FE", x"01", x"01"),    --#143#menuLUD
    (x"00", x"00", x"FE", x"00", x"00"),    --#144#menuD
    (x"00", x"00", x"FE", x"00", x"00"),    --#145#menuD
    (x"00", x"00", x"FE", x"00", x"00") --#146#menuD
    );

    component display_console is
      port  (
          
          clk_i      : in  std_logic;
          nRst_i     : in std_logic;
          
          slave_i    : in  t_wishbone_slave_in;
          slave_o    : out t_wishbone_slave_out;
        --
          RST_DISP_o : out std_logic;                    -- Display Reset on AI0/pin AB3
          DC_SPI_o   : out  std_logic;
          SS_SPI_o   : out  std_logic;
          SCK_SPI_o  : out  std_logic;
          SD_SPI_o   : out std_logic;
    --
          SH_VR_o    : out  std_logic
          );
    end component display_console;

---------------------------------------------------------------------------------------------------------------------------
  component Display_RAM_Ini_v01
    port  (
        address_a : IN STD_LOGIC_VECTOR (10 DOWNTO 0);
        address_b : IN STD_LOGIC_VECTOR (10 DOWNTO 0);
        clock     : IN STD_LOGIC                       := '1';
        data_a    : IN STD_LOGIC_VECTOR (7 DOWNTO 0);
        data_b    : IN STD_LOGIC_VECTOR (7 DOWNTO 0);
        wren_a    : IN STD_LOGIC                       := '0';
        wren_b    : IN STD_LOGIC                       := '0';
        q_a       : OUT STD_LOGIC_VECTOR (7 DOWNTO 0);
        q_b       : OUT STD_LOGIC_VECTOR (7 DOWNTO 0)
        );
  end component;
----------------------------------------------------------------------------------------------------------------------------
  component spi_master is
port (
    clk_i         : in  std_logic;
    nRst_i        : in  std_logic;
    
    -- Control
    load_i        : in std_logic;
    DC_i          : in  std_logic;                   -- 1 Datastream, Commandstream 0
    data_i        : in  std_logic_vector(7 downto 0);-- parallel data in
    stream_len_i  : in std_logic_vector(15 downto 0);           -- length of stream for controlling cs_n
    
    word_done_o   : out std_logic;                   -- ack after each word sent
    stream_done_o : out std_logic;                   -- ack after complete sream sent
    buf_empty_o   : out std_logic;
    
    
    --SPI
    spi_clk       : out std_logic;
    spi_mosi      : out std_logic;
    spi_miso      : in  std_logic;
    spi_cs_n      : out std_logic;
    
    DC_o          : out std_logic
);
end component;

---------------------------------------------------------------------------------------------------------------------------
  component char_render is
  port(
        clk_i       : in std_logic;
        nRst_i      : std_logic;
        addr_char_i : in std_logic_vector(7 downto 0);
        load_i      : in std_logic;
        valid_o     : out std_logic;
        q_o         : out std_logic_vector(7 downto 0)
  );
  end component;

  component  wb_console is
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
  end component ;
----------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------

constant c_oled_display : t_sdb_device := c_oled_slave_sdb;



end oled_display_pkg;