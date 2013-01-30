
-- 0 -> DL: Data LSB
-- 1 -> DH: Data MSB
-- 2 -> CS: Command/Status
-- 3 -> CO: Config

-- Write bits, CS:
--
-- START CS[0]:   Start transfer
-- END   CS[1]:   Deselect device after transfer (or immediately if START = '0')
-- IRQEN CS[2]:   Generate IRQ at end of transfer
-- SPIAD CS[6:4]: SPI device address
-- 
-- Read bits, CS:
--
-- BUSY  CS[0]: Currently transmitting data
--
-- Write BITS, CO:
--
-- DIVIDE CO[1:0]: SPI clock divisor, 00=clk/2, 01=clk/4, 10=clk/8, 11=clk/16
-- LENGTH CO[3:2]: Transfer length, 00=4 bits, 01=8 bits, 10=12 bits, 11=16 bits
--

library ieee;
use ieee.std_logic_1164.all;
use IEEE.numeric_std.all;
use ieee.std_logic_unsigned.all;

entity spi_master is
  port (
    clk_i              : in  std_logic;
    nRst_i             : in  std_logic;
    
    -- Control
    load_i		: in std_logic;
	DC_i               : in  std_logic;                   -- 1 Datastream, Commandstream 0
    data_i             : in  std_logic_vector(7 downto 0);-- parallel data in
    stream_len_i       : in std_logic_vector(15 downto 0);           -- length of stream for controlling cs_n
    
    word_done_o          : out std_logic;                   -- ack after each word sent
    stream_done_o        : out std_logic;                   -- ack after complete sream sent
    buf_empty_o              : out std_logic;
    
    
    --SPI
    spi_clk            : out std_logic;
    spi_mosi           : out std_logic;
    spi_miso           : in  std_logic;
    spi_cs_n           : out std_logic;
    
    DC_o               : out std_logic
);
end entity;

architecture rtl of spi_master is



  -- State type of the SPI transfer state machine
  type   state_type is (s_idle, s_running, s_done);
  signal state           : state_type;
  -- Shift register
  signal shift_reg       : std_logic_vector(7 downto 0);
  -- Buffer to hold data to be sent
  signal spi_data_buf    : std_logic_vector(7 downto 0);
  signal dc_buf    : std_logic;
  
  -- Start transmission flag
  signal start           : std_logic;
  -- Number of bits transfered
  signal bcount           : std_logic_vector(3 downto 0);
 -- Number of words transfered
  signal wcount           : std_logic_vector(16 downto 0);
  alias stream_done 	  : std_logic is wcount(16);
  
  signal buf_empty : std_logic;
signal buf_read : std_logic;
	
  -- Buffered SPI clock
  signal spi_clk_buf     : std_logic;
  -- Buffered SPI clock output
  signal spi_clk_out     : std_logic;
  -- Previous SPI clock state
  signal prev_spi_clk    : std_logic;
  -- Number of clk cycles-1 in this SPI clock period
  signal spi_clk_count   : std_logic_vector(2 downto 0);
  -- SPI clock divisor
  signal spi_clk_divide  : std_logic_vector(1 downto 0);
 

  -- Signal to clear IRQ
  signal irq         : std_logic;
  -- Internal chip select signal, will be demultiplexed through the cs_mux
  signal spi_cs          : std_logic;

begin

spi_mosi <= shift_reg(shift_reg'left);
spi_cs_n <= not spi_cs;
word_done_o <= irq;
stream_done_o <= stream_done;
buf_empty_o <= buf_empty;
 
word_counter : process(clk_i, nRst_i)
begin
	 if nRst_i = '0' then
	      wcount <= (others => '1');
   	 elsif rising_edge(clk_i) then

		  if(load_i = '1' AND stream_done = '1') then	
			 wcount <= '0' & (stream_len_i-1);
		  elsif(irq = '1' AND stream_done = '0') then
			 wcount <= wcount -1;
		  end if;
		  
	end if;	
end process;		
  
preload : process(clk_i, nRst_i)
begin
	 if nRst_i = '0' then
	      spi_data_buf <= (others => '0');
	      dc_buf <= '0';
	      buf_empty <= '1'; 	
   	 elsif rising_edge(clk_i) then
		if(load_i = '1') then	
			spi_data_buf 	<= data_i;
	      		dc_buf 		<= DC_i;
	      		buf_empty <= '0';
		end if;
		if(buf_read = '1') then
		  buf_empty <= '1';
		end if;  
	end if;	
end process;
  	

  -- SPI transfer state machine
  spi_proc : process(clk_i, nRst_i)
  begin
    if nRst_i = '0' then
      bcount        <= (others => '0');
      shift_reg    <= (others => '0');
      prev_spi_clk <= '0';
      spi_clk_out  <= '0';
      spi_cs       <= '0';
      DC_o         <= '0';
      state        <= s_idle;
      irq          <= '0';
      spi_clk_divide <= "11";
 
    elsif rising_edge(clk_i) then
      prev_spi_clk <= spi_clk_buf;
      irq          <= '0';
      buf_read 		<= '0';	

      case state is
        when s_idle =>
		if stream_done = '1' then
                	spi_cs <= '0';
              	elsif buf_empty = '0' then -- if the buffer is full
            
		    bcount     <= (others => '0');
		    shift_reg <= spi_data_buf;
		    buf_read <= '1';	
		    DC_o      <= dc_buf;	 		
		    spi_cs    <= '1';
		    state     <= s_running;
        	end if;	
       
	 when s_running =>
          if prev_spi_clk = '1' and spi_clk_buf = '0' then
            spi_clk_out <= '0';
            bcount       <= bcount + "0001";
            shift_reg   <= shift_reg(6 downto 0) & spi_miso;
            if (bcount = x"7") then
              --word complete
              irq   <= '1';
	      state <= s_idle;
            end if;
          elsif prev_spi_clk = '0' and spi_clk_buf = '1' then
            spi_clk_out <= '1';
          end if;
       

        when others =>
          state <= s_idle;
      end case;
    end if;
  end process;

  -- Generate SPI clock
  spi_clock_gen : process(clk_i, nRSt_i)
  begin
    if nRst_i = '0' then
      spi_clk_count <= (others => '0');
      spi_clk_buf   <= '0';
    elsif falling_edge(clk_i) then
      if state = s_running then
        if ((spi_clk_divide = "00")
            or (spi_clk_divide = "01" and spi_clk_count = "001")
            or (spi_clk_divide = "10" and spi_clk_count = "011")
            or (spi_clk_divide = "11" and spi_clk_count = "111")) then
          spi_clk_buf <= not spi_clk_buf;
          spi_clk_count <= (others => '0');
        else
          spi_clk_count <= spi_clk_count + "001";
        end if;
      else
        spi_clk_buf <= '0';
      end if;
    end if;
  end process;

  spi_clk  <= spi_clk_out;

end rtl;


