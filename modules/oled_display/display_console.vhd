----------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------
-- Company: 			GSI Darmstadt, BEL
-- Engineer:			M. Kreider
----------------------------------------------------------------------------------------------------------------------------
-- Create Date:		26-04-2012 
-- Design Name:		Controller for Display with SPI.
-- Project Name:		PEXARIA
-- Target Devices:	****************************
-- Tool versions:		****************************
-- Revision: 			Display SPI
----------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.numeric_std.all;

use work.wishbone_pkg.all;
----------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------
entity display_console is
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
end display_console;
----------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------
architecture behavioral of display_console is
----------------------------------------------------------------------------------------------------------------------------

	
	component wb_console is
generic(
    FifoDepth : integer := 4);
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
----------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------

	subtype byte is std_logic_vector(7 downto 0);
	type bytearray is array (integer range <>) of byte;

	constant    adr_sequence     			: bytearray(0 to 2) 	:= (	x"B0", x"00", x"1F");
	constant    scroll_start_sequence  	: bytearray(0 to 6) 	:= (	x"29", x"00", x"00", x"00", x"00", x"01", x"2F");
	constant    scroll_stop_sequence  	: bytearray(0 to 1) 	:= (	x"00",x"2E" );
	constant 	init_sequence 				: bytearray(0 to 11) := ( 	x"A1", x"C8", x"00", x"81",
																				x"50", x"AC", x"82", x"F0",
																				x"00", x"00", x"00", x"AF");
	
	
	constant c_Char_LF          : 	 std_logic_vector(7 downto 0) := x"0a";
	constant c_Char_CR          : 	 std_logic_vector(7 downto 0) := x"0d";
	constant c_Char_FF          : 	 std_logic_vector(7 downto 0) := x"0c";
	
	constant c_col_width        : 	 unsigned(7 downto 0) := x"41";
	constant c_firstBank        : 	 unsigned(2 downto 0) := "000";
	constant c_firstVisibleBank : 	 unsigned(2 downto 0) := "010";
	constant c_lastBank         : 	 unsigned(2 downto 0) := "111";
	
	signal s_firstCol           : 	 unsigned(7 downto 0);
	signal s_lastCol            : 	 unsigned(7 downto 0);
	signal col_offset				 : 	 std_logic_vector(7 downto 0);  
	
	signal nRst : std_logic;
	signal reset_disp: std_logic;	
	
 	
	signal need_refresh : std_logic;
	signal request_spi_transfer : std_logic;
	signal spi_done : std_logic;
	
   signal ascii_code : std_LOGIC_VECTOR(7 downto 0);
   signal ascii_code_reg : std_LOGIC_VECTOR(7 downto 0);
   
   signal char_valid : std_logic;
   signal char_load : std_logic;
   signal char_row	: std_logic_vector(2 downto 0);
	signal char_col : std_logic_vector(3 downto 0); 
  
   signal char_buffer_empty : std_logic;
	signal char_en : std_logic;
	signal scroll_request : std_logic;
  
   signal raw_mode : std_logic;
   signal raw_byte : std_logic_vector(7 downto 0);
   signal WRITE_DISP_RAM_DATA : std_logic_vector(7 downto 0);
  
   signal raw_addr : std_logic_vector(10 downto 0);
   signal raw_wren : std_logic;
	
	signal displaymode : std_logic_vector(1 downto 0);
	
	constant cDISPMODE_UART : std_logic_vector(1 downto 0) := "01";
	constant cDISPMODE_CHAR : std_logic_vector(1 downto 0) := "10";
	constant cDISPMODE_RAW : std_logic_vector(1 downto 0)  := "11";
	
	
	signal ptr : natural;
	
	signal bank_offset : unsigned(2 downto 0);
	
	
	signal stream_done : std_logic;
	 signal load : std_logic;
	signal spi_rdy : std_logic;

	signal wait_done_rst : std_logic;
	signal cnt_wait_rst : unsigned(32 downto 0);
	signal wait_update_cnt : unsigned(4 downto 0);
	
	signal scroll_wait_cnt : unsigned(32 downto 0);
	constant c_WAIT_CYCLES_RST : unsigned(32 downto 0) := to_unsigned(30000, 32+1); 

  signal wait1cyc : std_logic;
  signal disp_ram_refresh_in_progress : std_logic;
 signal done : std_logic;
 
 
 signal AnRst : std_logic;
  signal RST_done : std_logic;

	signal SCK_SPI_INT				: std_logic := '0';
	signal SS_SPI_INT					: std_logic := '0';
	signal SD_SPI_INT					: std_logic := '0';
--
	signal DC_SPI_INT					: std_logic := '0';
	signal SH_VR_INT					: std_logic := '0';
signal DISP_RST_INT				: std_logic := '0';
	
	type STATES is (IDLE_STATE, 				RESET_STATE, 			INI_STATE, 			START_DATA_STATE, 
						 ADR_STATE, 				TRN_ADR_STATE, 		FETCH_DATA_STATE, TRN_DATA_STATE,
						 BANK_DATA_DONE_STATE, 	REFRESH_RAM_STATE, SCROLL_START, SCROLL_WAIT, SCROLL_STOP);
	
	signal DISP_STATE	: STATES;
	
	type RFSTATES is (IDLE, RAW_DATA, GET_CHAR, COMMAND_CHAR, CLEAR_BANKS, START_RENDER, RENDER, UPDATE_DISPLAY_START, UPDATE_DISPLAY);
	signal rfstate  : RFSTATES;
	

	----------------------------------------------------------------------------------------------------------------
	-- Render and Bus interface side of the display memory --------------------------------------------------------|
	----------------------------------------------------------------------------------------------------------------
	signal disp_ram_addr			      : std_logic_vector (10 downto 0);
	alias  a_disp_ram_addr_bank  :  std_logic_vector(2 downto 0) is  disp_ram_addr(10 downto 8);
	alias  a_disp_ram_addr_byte  : std_logic_vector(7 downto 0) is  disp_ram_addr(7 downto 0);
	signal disp_ram_we		         : std_logic;
	signal disp_ram_data         : std_logic_vector (7 downto 0);
	signal disp_ram_q         : std_logic_vector (7 downto 0);

  --sources
  --Clear
  signal disp_ram_clear_addr    : std_logic_vector (10 downto 0);
  alias  a_disp_ram_clear_addr_bank : std_logic_vector(2 downto 0) is  disp_ram_clear_addr(10 downto 8);
	alias  a_disp_ram_clear_addr_byte : std_logic_vector(7 downto 0) is  disp_ram_clear_addr(7 downto 0);	
  signal disp_ram_clear_we   : std_logic;
  constant c_disp_ram_clear_data : std_logic_vector(7 downto 0) := (others => '0'); 
  signal  LastBank2Clear : std_logic_vector(2 downto 0);
  
  --Render
  signal disp_ram_render_addr    : std_logic_vector (10 downto 0);
  alias  a_disp_ram_render_addr_bank : std_logic_vector(2 downto 0) is  disp_ram_render_addr(10 downto 8);
	alias  a_disp_ram_render_addr_byte : std_logic_vector(7 downto 0) is  disp_ram_render_addr(7 downto 0);	
  signal disp_ram_render_we		         : std_logic;
	signal disp_ram_render_data         : std_logic_vector (7 downto 0);
  
  --RAW
  signal disp_ram_raw_addr : std_logic_vector (10 downto 0);
  signal disp_ram_raw_data : std_logic_vector (7 downto 0);
  signal disp_ram_raw_q : std_logic_vector (7 downto 0);
	signal disp_ram_raw_we   : std_logic;
	----------------------------------------------------------------------------------------------------------------
	
	
	----------------------------------------------------------------------------------------------------------------
	-- Display controller side of the display memory --------------------------------------------------------------|
	----------------------------------------------------------------------------------------------------------------
	signal disp_rom_addr			: std_logic_vector (10 downto 0);
	alias a_disp_rom_addr_bank : std_logic_vector(2 downto 0) is  disp_rom_addr(10 downto 8);
	alias a_disp_rom_addr_byte : std_logic_vector(7 downto 0) is  disp_rom_addr(7 downto 0);
	signal disp_rom_q			: std_logic_vector (7 downto 0);
	signal cnt_bank : unsigned(2 downto 0);
	----------------------------------------------------------------------------------------------------------------
	  
	
	constant	Pix_Dis_INT				: std_logic_vector (15 downto 0)	:= x"0084";	-- for 132 pixels.
	signal stream_len 				: std_logic_vector(15 downto 0); 
signal spi_data : std_logic_vector(7 downto 0); 


begin



		wbif : wb_console
generic map(FifoDepth => 16)
  port map (
    -- Common wishbone signals
    clk_i			=> clk_i,
	 nRst_i 			=> nRst_i, -- only connect to external reset
    -- Slave control port
    slave_i     => slave_i,
    slave_o     => slave_o,
	 --Raw Image
	 raw_data_i	=> disp_ram_raw_q,
	 raw_data_o => disp_ram_raw_data,
	 raw_addr_o => disp_ram_raw_addr,
	 raw_wren_o	=> disp_ram_raw_we,
	 col_offset_o => col_offset,


	 
reset_disp_o => 	 reset_disp,
	 
    fifo_o		=> ascii_code,
    char_row_o	=> char_row,
	 char_col_o => char_col,
	 empty_o 	=> char_buffer_empty ,
	 valid_o		=> char_valid,
	 read_i		=> char_en,
	 mode_o		=> displaymode
	); 

s_firstCol <= unsigned(col_offset);

	Character_Render_Engine : char_render
		port map	(
					clk_i				      => clk_i,
					nRst_i 			     => nRst, -- external / WB reset
					load_i 			     => char_load,
					addr_char_i		  => ascii_code_reg,
					valid_o			     =>	disp_ram_render_we,
					q_o				       => disp_ram_render_data
					);

----------------------------------------------------------------------------------------------------------------------------
	Inst_Display_RAM_Ini_v01: Display_RAM_Ini_v01
		port map	(
					address_a	 => disp_rom_addr,
					address_b	 => disp_ram_addr,
					clock			   => clk_i,
					data_a		   => (others => '0'),
					data_b		   => disp_ram_data,
					wren_a		   => '0',
					wren_b		   => disp_ram_we,
					q_a			     => disp_rom_q,
					q_b			     => disp_ram_q
					);
----------------------------------------------------------------------------------------------------------------------------
Inst_SPI_Control_Block: spi_master
		port map	(
    clk_i              => clk_i,
    nRst_i             => wait_done_rst, 
    
    -- Control
    load_i		           => load,
	  DC_i               => DC_SPI_INT,                   -- 1 Datastream, Commandstream 0
    data_i             => spi_data,     -- parallel data in
    stream_len_i       => stream_len,           -- length of stream for controlling cs_n
    
    word_done_o        => open,                  -- ack after each word sent
    stream_done_o      => stream_done,                   -- ack after complete sream sent
    buf_empty_o        => spi_rdy,
    
    
    --SPI
    spi_clk            => SCK_SPI_INT,
    spi_mosi           => SD_SPI_INT,
    spi_miso           => '0',
    spi_cs_n           => SS_SPI_INT,
    
    DC_o               => DC_SPI_o
);

----------------------------------------------------------------------------------------------------------------------------
-- Reset Functions
----------------------------------------------------------------------------------------------------
	nRst 				<= nRst_i and not reset_disp; 
	wait_done_rst 	<= std_logic(cnt_wait_rst(cnt_wait_rst'left));
	RST_DISP_o		<= (nRst and wait_done_rst);
	

	SH_VR_o		<= SH_VR_INT;
	SS_SPI_o		<= SS_SPI_INT;
	SCK_SPI_o	<= SCK_SPI_INT;
	SD_SPI_o		<= SD_SPI_INT;
	
----------------------------------------------------------------------------------------------------
-- Muxes for Render and Bus side of display memory
----------------------------------------------------------------------------------------------------
 DISP_RAM_ADDR_MUX : with rfstate select
			disp_ram_addr		<= 	disp_ram_clear_addr   when	CLEAR_BANKS,
	                       disp_ram_render_addr  when START_RENDER | RENDER,
	                       disp_ram_raw_addr     when RAW_DATA,
	                       disp_ram_render_addr  when others;	 
	
 DISP_RAM_WE_MUX : with rfstate select
			disp_ram_we		<= 	disp_ram_clear_we     when	CLEAR_BANKS,
	                       disp_ram_render_we    when START_RENDER | RENDER,
	                       disp_ram_raw_we       when RAW_DATA,
	                       disp_ram_render_we  when others;	                       
	                       
 DISP_RAM_DATA_MUX : with rfstate select
			disp_ram_data		<= 	c_disp_ram_clear_data when	CLEAR_BANKS,
	                       disp_ram_render_data  when START_RENDER | RENDER,
	                       disp_ram_raw_data     when RAW_DATA,
	                       disp_ram_render_data  when others;
	                       
disp_ram_raw_q <= disp_ram_q;	                       	 
----------------------------------------------------------------------------------------------------                      	 	                       
s_lastCol <= s_firstCol + c_col_width;

REFRESH_RAM : process(clk_i)
begin
	
	if(rising_edge(clk_i)) then
	  
		char_en             <= '0';
		scroll_request      <= '0';
		char_load           <= '0';
	  ascii_code_reg      <= ascii_code;
	
	if(nRst = '0') then
		rfstate              <=  IDLE;
		bank_offset          <= (others => '0');
		need_refresh         <= '0';
		disp_ram_clear_addr  <= std_logic_vector(c_firstVisibleBank & s_firstCol);
		disp_ram_render_addr <= std_logic_vector(c_firstVisibleBank & s_firstCol);
		disp_ram_clear_we    <= '0';
	else	
		
	
	 
  	case rfstate is
  		
  		when IDLE =>        need_refresh <= '0';  
  		                    request_spi_transfer <= '0';
  		                    if(displaymode = cDISPMODE_CHAR OR displaymode = cDISPMODE_UART) then
  			                     if(char_buffer_empty = '0') then
  			                       rfstate <= GET_CHAR;
  			                     end if;
  			                  elsif(displaymode = cDISPMODE_RAW) then
  			                      rfstate <= RAW_DATA;
  			                  end if;
  		
		when RAW_DATA =>    if(displaymode /= cDISPMODE_RAW ) then
		                        rfstate <= UPDATE_DISPLAY_START;
		                    end if;
  		
  		                      
  		when GET_CHAR =>    if(char_buffer_empty = '0' and disp_ram_we = '0') then
  		                        need_refresh <= '1';
  		                        char_en 	<= '1';	
  		                        rfstate <= COMMAND_CHAR;
  		                      else
  		                        if(need_refresh = '1') then
  		                          rfstate <=  UPDATE_DISPLAY_START;
										  
  		                      else
		                          rfstate <=  IDLE;
		                        end if;  
  		                      end if;
									 
		when COMMAND_CHAR	=>  if(char_valid = '1') then
		
		              case ascii_code is
										when c_CHAR_LF | c_CHAR_CR => if(displaymode = cDISPMODE_UART) then
																					a_disp_ram_clear_addr_bank 	<= std_logic_vector(unsigned(a_disp_ram_render_addr_bank)+1);
																					LastBank2clear  		<= std_logic_vector(unsigned(a_disp_ram_render_addr_bank)+1);
																					a_disp_ram_render_addr_byte 				<= std_logic_vector(s_firstCol);
																					a_disp_ram_render_addr_bank 		<= std_logic_vector(unsigned(a_disp_ram_render_addr_bank) +1); -- inc row/bank ptr
																					if(a_disp_ram_render_addr_bank = std_logic_vector(c_lastBank-bank_offset)) then 		-- last row/bank?
																						 bank_offset 		<= bank_offset-1;
																					end if;
																						rfstate <= CLEAR_BANKS;
																				else
																					rfstate <= GET_CHAR;
																				end if;
																				
										when c_CHAR_FF 				=> a_disp_ram_clear_addr_bank 	<= std_logic_vector(unsigned(a_disp_ram_render_addr_bank)+1);
																				LastBank2clear  		<= a_disp_ram_render_addr_bank;
																				a_disp_ram_render_addr_bank 		<= std_logic_vector(c_firstVisibleBank);
																				a_disp_ram_render_addr_byte 				<= std_logic_vector(s_firstCol);
																				bank_offset 			<= "000";
																				rfstate 					<= CLEAR_BANKS;	
																											
										when others 					=> char_load <= '1';
										                    if(displaymode = cDISPMODE_CHAR) then
										                      a_disp_ram_render_addr_byte <= std_logic_vector(s_firstCol+unsigned(char_col)*6);
										                      a_disp_ram_render_addr_bank <= std_logic_vector(c_firstVisibleBank+unsigned(char_row));
										                      bank_offset 			<= "000";
										                    end if;
										                    rfstate <= START_RENDER;
									 end case;
									end if;		

		when CLEAR_BANKS	=>   	
										disp_ram_clear_we <= '1';
										if(unsigned(a_disp_ram_clear_addr_byte) >= s_lastCol) then						-- out of visible display area?
											a_disp_ram_clear_addr_byte <= std_logic_vector(s_firstCol-1);
											if(a_disp_ram_clear_addr_bank	= LastBank2clear) then
												rfstate 					<= UPDATE_DISPLAY_START;
												scroll_request 	<= '1';
																					 
											else
												a_disp_ram_clear_addr_bank 	<= std_logic_vector(unsigned(a_disp_ram_clear_addr_bank)+1);											
											end if;
										else	
											a_disp_ram_clear_addr_byte <= std_logic_vector(unsigned(a_disp_ram_clear_addr_byte)+1);
										end if;
  		
  		when START_RENDER =>   if(disp_ram_render_we = '1') then
  		                        a_disp_ram_render_addr_byte 				<= std_logic_vector(unsigned(a_disp_ram_render_addr_byte)+1); 		-- inc col ptr
  		                        rfstate <=  RENDER;
                            end if;
  		
  		when RENDER      =>   if(disp_ram_we = '1') then
  		                        a_disp_ram_render_addr_byte 				<= std_logic_vector(unsigned(a_disp_ram_render_addr_byte) +1); 		-- inc col ptr
  		                      else
  		                        rfstate <=  GET_CHAR;
		                      if(displaymode = cDISPMODE_UART) then
  		                        if(unsigned(a_disp_ram_render_addr_byte) >= s_lastCol) then						-- out of visible display area?
  					                     a_disp_ram_render_addr_byte <= std_logic_vector(s_firstCol);							-- reset col ptr
  					                     a_disp_ram_render_addr_bank <= std_logic_vector(unsigned(a_disp_ram_render_addr_bank) +1); -- inc row/bank ptr
  					                     if(a_disp_ram_render_addr_bank = std_logic_vector(c_lastBank-bank_offset)) then 		-- last row/bank?
  						                     if(displaymode = cDISPMODE_CHAR) then     
													         rfstate <=  GET_CHAR;
													       else
													          bank_offset <= bank_offset-1;
													          a_disp_ram_clear_addr_bank 	<= std_logic_vector(unsigned(a_disp_ram_render_addr_bank)+1);
																    LastBank2clear  		<= std_logic_vector(unsigned(a_disp_ram_render_addr_bank)+1);
																    rfstate <=  CLEAR_BANKS;  
													       end if;
													        
  						                     
  					                     end if;
  				                    end if;
		                       end if; 
  		                        
                          end if;
                          
		when UPDATE_DISPLAY_START =>  request_spi_transfer <= '1';
												wait_update_cnt <= to_unsigned(10, 5);
												rfstate 					<= UPDATE_DISPLAY;
		
		when UPDATE_DISPLAY =>  if(wait_update_cnt(wait_update_cnt'left) = '1') then
											request_spi_transfer <= '0';
											if(spi_done = '1') then
											  rfstate <=  IDLE;  
										  end if;
										else
												wait_update_cnt <= wait_update_cnt -1;
		                        end if;
										
  		when others =>        rfstate <=  IDLE;                  
  	end case;                
		
 end if;	
end if;
	  
	  
	  
end process REFRESH_RAM;




----------------------------------------------------------------------------------------------------------------------------
	main : process (clk_i, nRst )
	begin
		if (nRst = '0') then
			disp_rom_addr <= (others => '0');
			SH_VR_INT			<= '0';
			DC_SPI_INT			<= '0';
			DISP_STATE			<= IDLE_STATE;
			load <= '0';
			stream_len <= std_logic_vector(to_unsigned(15, stream_len'length));
			spi_done <= '0';
			cnt_wait_rst <= c_WAIT_CYCLES_RST;
		elsif (clk_i'event and clk_i = '1') then
			
		load <= '0';
		wait1cyc <= '0';
		spi_done <= '0';	
			if (wait1cyc = '0') then
				
	------------------------------------------------------------
					case DISP_STATE is
	------------------------------------------------------------
						when IDLE_STATE 			=>
							cnt_wait_rst <= c_WAIT_CYCLES_RST;
							DISP_STATE			<= RESET_STATE;
							
	------------------------------------------------------------
						when RESET_STATE 			=>
							cnt_wait_rst <= cnt_wait_rst -1;
							if (wait_done_rst = '1') then
								SH_VR_INT			<= '1';
								DISP_STATE			<= INI_STATE;
								ptr <= 0;
							
							end if;
	------------------------------------------------------------
						when INI_STATE 			=>
						  stream_len <= std_logic_vector(to_unsigned(init_sequence'length, stream_len'length));
										
							if(spi_rdy = '1') then
								if(ptr = init_sequence'length) then
							       if(stream_done = '1') then
									     DISP_STATE <= START_DATA_STATE;
									  
								     end if;
									  
								else
									wait1cyc <= '1';
									load		<= '1';
									DC_SPI_INT				   <= '0';				-- sent out a command
							    spi_data	   <= init_sequence(ptr);
							    ptr <= ptr +1;
								end if;
							end if;
								 
						when START_DATA_STATE 		=>
							
								ptr <= 0;
							  cnt_bank <= c_firstBank;
							  DISP_STATE <= TRN_ADR_STATE;
								spi_done <= '0';
							
														
							--set bank address
						when TRN_ADR_STATE 			=>
						  stream_len <= std_logic_vector(to_unsigned(adr_sequence'length, stream_len'length));	
							if(ptr = adr_sequence'length) then
								--wait for refresh to be finished
								  if(stream_done = '1') then
									 ptr <= 0;
									 DISP_STATE <= FETCH_DATA_STATE;
								  end if;
							else
								if(spi_rdy= '1') then
																	
    									   if(ptr = 0) then
    									       spi_data		<= adr_sequence(ptr)(7 downto 3) & std_logic_vector(cnt_bank+bank_offset);
    									   else
    									       spi_data		<= adr_sequence(ptr);
    									   end if;
    									   wait1cyc <= '1';
    									   load		<= '1';
    									   DC_SPI_INT				   <= '0';				-- sent out a command
    							       ptr <= ptr +1;
							    end if;
								end if;
							
						
						when FETCH_DATA_STATE 			=>
						   a_disp_rom_addr_bank <= std_logic_vector(cnt_bank);
						   a_disp_rom_addr_byte <= std_logic_vector(to_unsigned(ptr, 8));
						   DISP_STATE <= TRN_DATA_STATE; 
						
						when TRN_DATA_STATE 			=>
							stream_len <= Pix_Dis_INT;
							if(ptr = to_integer(unsigned(Pix_Dis_INT))) then
								 if(stream_done = '1') then
									   DISP_STATE <= BANK_DATA_DONE_STATE;
								 end if;
								
							else
								if(spi_rdy= '1') then
								
									wait1cyc <= '1';
									load <= '1';
									spi_data <= disp_rom_q;
							
									DC_SPI_INT				   <= '1';				-- sent out a command
							   
									ptr <= ptr +1;
									
									DISP_STATE <= FETCH_DATA_STATE;
								end if;
							end if;
						
						when BANK_DATA_DONE_STATE 	=>
							
							ptr <= 0;
							
							if(cnt_bank < c_lastBank) then
								cnt_bank <= cnt_bank + 1;
								
								DISP_STATE <= TRN_ADR_STATE;
							else
								if(scroll_request = '1') then
										ptr <= 0;
										DISP_STATE <= SCROLL_START;
									else
										DISP_STATE <= REFRESH_RAM_STATE;
									end if;	
								
								wait1cyc <= '1';
							end if;	
						
						when REFRESH_RAM_STATE =>
							spi_done <= '1';
							if(wait1cyc = '0') then
								if(request_spi_transfer = '1') then 
									DISP_STATE <= START_DATA_STATE;
								end if;
							end if;
							
						when SCROLL_START => stream_len <= std_logic_vector(to_unsigned(scroll_start_sequence'length, stream_len'length));
							if(spi_rdy = '1') then
								if(ptr = scroll_start_sequence'length) then
							       if(stream_done = '1') then
									     DISP_STATE <= SCROLL_WAIT;
										  scroll_wait_cnt <= to_unsigned(20, 33);
										  --scroll_wait_cnt <= to_unsigned(500000, 33);
									 end if;
								else
									wait1cyc <= '1';
									load		<= '1';
									DC_SPI_INT				   <= '0';				-- sent out a command
									spi_data	   <= scroll_start_sequence(ptr);
									ptr <= ptr +1;
								end if;
							end if;
						
						when SCROLL_WAIT =>
								if(scroll_wait_cnt(scroll_wait_cnt'left) = '1') then
									     DISP_STATE <= SCROLL_STOP;
										  ptr <= 0;
								else
									scroll_wait_cnt <= scroll_wait_cnt-1;
								end if;
						
						when SCROLL_STOP => stream_len <= std_logic_vector(to_unsigned(scroll_stop_sequence'length, stream_len'length));
							if(spi_rdy = '1') then
								if(ptr = scroll_stop_sequence'length) then
							       if(stream_done = '1') then
									     DISP_STATE <= REFRESH_RAM_STATE;
									  end if;
								else
									wait1cyc <= '1';
									load		<= '1';
									DC_SPI_INT				   <= '0';				-- sent out a command
									spi_data	   <= scroll_stop_sequence(ptr);
									ptr <= ptr +1;
								end if;
							end if;	
						
						when others => DISP_STATE <= IDLE_STATE;
						end case;
						
						
							
			end if;
		end if;
	end process;
	
	
----------------------------------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------------------------------




----------------------------------------------------------------------------------------------------------------------------
end behavioral;
----------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------


