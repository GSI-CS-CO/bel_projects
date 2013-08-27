library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.wishbone_pkg.all;
use work.genram_pkg.all;


--Only low data byte is used in all operations
--Note: Form Feed (FF) 0x0c clears screen in mode 1 and 2

--Register Layout
--AddrMask: 0x3FFFF
--0x00000 Mode 			Data Bits(1-0) 00 : idle
--													01 : uart
--													10 : char
--													11 : raw	
--
--0x00004 Reset			Write resets FSMs and display controller
--
--0x00008 ColOffset     Offset of the first visible column. 0x23 for old, 0x30 for new controllers
--
--0x10000 UART				Written ascii code is put directly on screen
--
--0x20000 Char				Written ascii code is put on supplied location
--								Address Bits(8-6) Char location row
--								Address Bits(5-2) Char location col
--									
--				  				 
--0x30000 Raw				Raw read/write to display memory. Organisation is column based, 
--								8px per column, 
--								Address Bits(12-2) Disp RAM address	


  
entity wb_console is
generic(
    FifoDepth : integer := 4);
  port(
    -- Common wishbone signals
    clk_i       	: in  std_logic;
    nRst_i     	: in  std_logic;
    -- Slave control port
    slave_i     	: in  t_wishbone_slave_in;
    slave_o     	: out t_wishbone_slave_out;
	 
	 -- General Purpose Signals common to all modes
	 valid_o			: out std_logic;							--data from WBIF is valid				
	 mode_o			: out std_logic_vector(1 downto 0); --display mode (idle, uart, char, raw)
	 reset_disp_o 	: out std_logic; 							--issue a reset of the display controller
	 col_offset_o  : out std_logic_vector(7 downto 0);
	 
	 --Raw Image
	 raw_data_i		: in 	std_logic_vector(7 downto 0);
	 raw_data_o 	: out std_logic_vector(7 downto 0);
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

signal adrmode : natural;
  signal mode		:	std_logic_vector(1 downto 0);
  signal w_full, w_en, r_empty, r_en : std_logic;
  signal w_data, r_data : std_logic_vector(c_wishbone_data_width/4-1 downto 0);

  signal wbuffer : std_logic_vector(7 downto 0);
  signal char_row : unsigned(2 downto 0);
	signal char_col : unsigned(3 downto 0);
  signal valid : std_logic;
  signal slave_o_ACK : std_logic;
  signal slave_o_STALL : std_logic;
  signal slave_o_DAT : std_logic_vector(c_wishbone_data_width-1 downto 0);

  signal disp_ram_ack_sh : std_logic;
  signal disp_ram_ack_shreg : std_logic_vector(0 downto 0);
  
  constant c_firstCol : 	 std_logic_vector(7 downto 0) := x"30";
  signal   col_offset : std_logic_vector(7 downto 0);
  	
begin
  
  
  col_offset_o  <= col_offset;
  -- Hard-wired slave pins
  slave_o.ACK   <= slave_o_ACK or disp_ram_ack_shreg(disp_ram_ack_shreg'left);
  slave_o.ERR   <= '0';
  slave_o.RTY   <= '0';
  slave_o.STALL <= slave_o_STALL;
  
  --output mux, wbif/displayram
  data_out_mux : with disp_ram_ack_shreg(disp_ram_ack_shreg'left) select
  slave_o.DAT   <= 	slave_o_DAT when '0',
					          x"000000" & raw_data_i when others;
  
  -- Output pins
  mode_o  <= mode;
  fifo_o  <= r_data;
  empty_o <= r_empty;
  valid_o <= valid;
  
  fifo : generic_sync_fifo
    generic map(
      g_data_width => 8,
      g_size       => 16)
    port map(
      rst_n_i        => nRst_i,
      clk_i          => clk_i,
      d_i            => wbuffer,
      we_i           => w_en,
      q_o            => r_data,
      rd_i           => r_en,
      empty_o        => r_empty,
      full_o         => w_full,
      almost_empty_o => open,
      almost_full_o  => open,
      count_o        => open);

adrmode <= to_integer(unsigned(slave_i.ADR(17 downto 16)));  
char_col_o <= std_logic_vector(char_col);
										char_row_o <= std_logic_vector(char_row);
										

										
										
  main : process(clk_i)
	begin
    if rising_edge(clk_i) then
      if nRst_i = '0' then
			w_en <= '0';
			r_en <= '0';
			valid <= '0';
			raw_wren_o <= '0';
			mode <= "01";
			char_col <= x"0";
			char_row <= "000";
			reset_disp_o <= '0';
			col_offset <= c_firstCol;
		else
        
		  reset_disp_o <= '0';
		  slave_o_ACK <= '0';
		  slave_o_STALL <= '1';
		  r_en <= read_i AND not r_empty AND not r_en;
        w_en <= '0';
        
		  if(slave_i.SEL(0) = '1') then
			wbuffer <= slave_i.DAT(7 downto 0);
		  end if;
		  
        raw_wren_o <= '0';
        
			valid		<= r_en;
			
			
      raw_addr_o <= (others => '0'); 
			disp_ram_ack_sh <= '0';
			disp_ram_ack_shreg <= disp_ram_ack_shreg(disp_ram_ack_shreg'left-1 downto 0) & disp_ram_ack_sh;
		
       if (slave_i.CYC = '1' and slave_i.STB = '1' ) then
			
									
			case adrmode is
					when 0 =>  --only lower stall take on new commands if there are no acks pending
								if(unsigned(disp_ram_ack_shreg) = 0) then
									slave_o_STALL <= '0';
								end if;
								slave_o_ACK <= not slave_o_STALL;
								if(slave_i.SEL(0) = '1') then
									--mode register
									if(unsigned(slave_i.ADR(15 downto 2)) = 0) then
										if(slave_i.WE = '1') then
								        mode <= wbuffer(1 downto 0);
								      else
											slave_o_DAT <= std_logic_vector(to_unsigned(0,30)) & mode;
										end if;
									--reset register
									elsif(unsigned(slave_i.ADR(15 downto 2)) = 1) then
										reset_disp_o <= slave_i.WE;
									elsif(unsigned(slave_i.ADR(15 downto 2)) = 2) then
										slave_o_DAT <= std_logic_vector(to_unsigned(0,24)) & col_offset;
										if(slave_i.WE = '1') then
											col_offset <= slave_i.DAT(7 downto 0);
										end if; 	
									end if;
								end if; 
								
					when 1 =>	--only lower stall take on new commands if there are no acks pending
								if(unsigned(disp_ram_ack_shreg) = 0) then
									slave_o_STALL <= not (not w_full and slave_o_STALL);
								end if;
								slave_o_ACK <= not slave_o_STALL;
								 if(slave_i.WE = '1' and slave_i.SEL(0) = '1' and slave_o_stall = '0' and unsigned(slave_i.ADR(15 downto 2)) = 0) then -- UART FIFO
									w_en <= '1';
								 end if;
					
					when 2 => 
								--only lower stall take on new commands if there are no acks pending
								if(unsigned(disp_ram_ack_shreg) = 0) then
									slave_o_STALL <= not (not w_full and slave_o_STALL and r_empty);
								end if;
								slave_o_ACK <= not slave_o_STALL;
								if(slave_i.WE = '1' and slave_i.SEL(0) = '1' and slave_o_stall = '0' and unsigned(slave_i.ADR(5 downto 2)) < 11) then -- Raw Char
										w_en <= '1';
										char_row <= unsigned(slave_i.ADR(8 downto 6));
										char_col <= unsigned(slave_i.ADR(5 downto 2));
										
								end if;
					
					when 3 => slave_o_STALL <= '0';
								raw_data_o <= slave_i.DAT(7 downto 0);
								raw_addr_o <= slave_i.ADR(12 downto 2);
								if(slave_o_stall = '0') then
        								--write display ram
        								  if(slave_i.WE = '1') then -- Raw
        										if(slave_i.SEL(0) = '1') then
        										  raw_wren_o <= '1';
      										end if;   
        										slave_o_ACK <= not slave_o_STALL;
        								  else
        										  --read out display ram. deal with latency by putting fsm in wait til data is ready
        									 	 disp_ram_ack_sh <= '1';
        								  end if;
								end if;
								
					when others => if(unsigned(disp_ram_ack_shreg) = 0) then
											slave_o_STALL <= '0';
										end if;
										slave_o_ACK <= not slave_o_STALL;
										
			  end case;
			  
			  
        
			end if;
		  
		  
      end if;
    end if;
  end process;
end rtl;
