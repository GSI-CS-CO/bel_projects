library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

package oled_display_pkg is

component display_console is
	port	(
			
			clk_i				: in  std_logic;
			nRst_i   : in std_logic;
			
			slave_i     : in  t_wishbone_slave_in;
			slave_o     : out t_wishbone_slave_out;
    --
			RST_DISP_o			: out std_logic;										-- Display Reset on AI0/pin AB3
			DC_SPI_o				: out  std_logic;
			SS_SPI_o				: out  std_logic;
			SCK_SPI_o				: out  std_logic;
			SD_SPI_o				: out std_logic;
--
			SH_VR_o				: out  std_logic
			);
end component display_console;


	
	component wb_console is
  port(
    -- Common wishbone signals
    clk_i       : in  std_logic;
    nRst_i     : in  std_logic;
    -- Slave control port
    slave_i     : in  t_wishbone_slave_in;
    slave_o     : out t_wishbone_slave_out;
	 
	 -- General Purpose Signals common to all modes
	 valid_o			: out std_logic;
	 mode_o			:	out std_logic_vector(1 downto 0);
	 reset_disp_o 	: out std_logic; 							--issue a reset of the display controller
	 col_offset_o  : out std_logic_vector(7 downto 0);
	 
	 --Raw Image
	 raw_data_i		: in  std_logic_vector(7 downto 0);
	 raw_data_o 	: out std_logic_vector(7 downto 0);
	 raw_addr_o 	: out std_logic_vector(10 downto 0);
	 raw_wren_o		: out std_logic;
	 
	 --Raw Char
	 char_row_o		:	out std_logic_vector(2 downto 0);
	 char_col_o 	: 	out std_logic_vector(3 downto 0); 
	 
	 --UART console	
    fifo_o			:	out std_logic_vector(7 downto 0);
	 empty_o			: out std_logic;
	 
	 read_i		: in std_logic
	 
	 
	); 
end component wb_console;
	
---------------------------------------------------------------------------------------------------------------------------
	component Display_RAM_Ini_v01
		port	(
				address_a	: IN STD_LOGIC_VECTOR (10 DOWNTO 0);
				address_b	: IN STD_LOGIC_VECTOR (10 DOWNTO 0);
				clock			: IN STD_LOGIC  := '1';
				data_a		: IN STD_LOGIC_VECTOR (7 DOWNTO 0);
				data_b		: IN STD_LOGIC_VECTOR (7 DOWNTO 0);
				wren_a		: IN STD_LOGIC  := '0';
				wren_b		: IN STD_LOGIC  := '0';
				q_a			: OUT STD_LOGIC_VECTOR (7 DOWNTO 0);
				q_b			: OUT STD_LOGIC_VECTOR (7 DOWNTO 0)
				);
	end component;
----------------------------------------------------------------------------------------------------------------------------
	component spi_master is
port (
    clk_i         : in  std_logic;
    nRst_i        : in  std_logic;
    
    -- Control
    load_i		   : in std_logic;
	  DC_i         : in  std_logic;                   -- 1 Datastream, Commandstream 0
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
			   q_o			: out std_logic_vector(7 downto 0)
	);
	end component;

  constant cDISPMODE_NONE : std_logic_vector(1 downto 0) := "00";
  constant cDISPMODE_UART : std_logic_vector(1 downto 0) := "01";
	constant cDISPMODE_CHAR : std_logic_vector(1 downto 0) := "10";
	constant cDISPMODE_RAW  : std_logic_vector(1 downto 0)  := "11";


constant c_oled_display : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"000000000003ffff",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"93a6f3c4",
    version       => x"00000001",
    date          => x"20130130",
    name          => "OLED_Display       ")));



end oled_display_pkg;
