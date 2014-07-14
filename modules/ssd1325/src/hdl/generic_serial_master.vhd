-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- entity
entity generic_serial_master is
    Generic ( g_system_clock : natural := 62500000; -- Hz 
              g_serial_clock : natural := 2000000;  -- Hz
              g_data_width   : natural := 8         -- bit(s)
             ); 
    Port ( clk_sys_i  : in  std_logic;
           rst_n_i    : in  std_logic;
           tx_data_i  : in  std_logic_vector (g_data_width-1 downto 0);
           mosi_o     : out std_logic;                           
           sclk_o     : out std_logic;
           ss_o       : out std_logic;
           dc_o       : out std_logic;
           tx_start_i : in  std_logic;
           tx_done_o  : out std_logic;
           rx_read_o  : out std_logic
         );
end generic_serial_master;

architecture rtl of generic_serial_master is
    
  -- internal signals and variables
  type     t_spi_tx_states is (state_spi_start_tx, stat_spi_active_tx, state_spi_end_tx);
  constant c_clock_delay     : natural := (g_system_clock/(2*g_serial_clock))-1;
  signal   s_delay           : natural range 0 to (g_system_clock/(2*g_serial_clock));
  signal   s_spitxstate      : t_spi_tx_states := state_spi_start_tx;
  signal   s_rx_read_o       : std_logic;
  signal   s_rx_read_o_last  : std_logic;
  signal   s_tx_start_i      : std_logic;
  signal   s_tx_done         : std_logic;
  signal   s_tx_in_process   : std_logic;
  signal   s_tx_delayed_data : std_logic;
  signal   s_spiclk          : std_logic;
  signal   s_spiclklast      : std_logic;
  signal   s_bitcounter      : natural range 0 to g_data_width; 
  signal   s_tx_reg          : std_logic_vector(g_data_width-1 downto 0) := (others=>'0');

begin

  -- assign signals to output
  sclk_o    <= s_spiclk;
  mosi_o    <= s_tx_delayed_data;
  rx_read_o <= s_rx_read_o;
  tx_done_o <= s_tx_done;
  
  -- trigger transmission if tx process id ready (again)
  p_trigger_fifo_read : process(clk_sys_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      s_rx_read_o    <= '0';
      s_tx_start_i   <= '0';
    elsif (rising_edge(clk_sys_i)) then
      if (s_tx_in_process='0' and s_rx_read_o='0' and not(tx_start_i='1')) then
        s_rx_read_o  <= '1';
        s_tx_start_i <= '1';
      else
        s_rx_read_o  <= '0';
        s_tx_start_i <= '0';
      end if;
    end if;
  end process;
  
  p_transmit_state_machine : process(clk_sys_i, rst_n_i)
  begin
    -- perform reset
    if (rst_n_i = '0') then
      ss_o            <= '1';
      s_tx_done       <= '0';
      s_spiclk        <= '0';
      s_spitxstate    <= state_spi_start_tx; 
      s_spiclklast    <= '0';
      s_tx_in_process <= '0';
      -- perform normal logic flow
    elsif (rising_edge(clk_sys_i)) then
      -- generate delay
      if (s_delay>0) then
        s_delay <= s_delay-1;
      else
        s_delay <= c_clock_delay;  
      end if;
        -- prepare next transfer and wait until tx start signal rises
      case s_spitxstate is
        -- wait or for transfer request
        when state_spi_start_tx =>
          ss_o              <= '1'; 
          s_tx_done         <= '0';
          s_bitcounter      <= g_data_width;
          s_spiclk          <= '1'; 
          if(s_tx_start_i = '1') then 
            s_tx_in_process <= '1';
            s_spitxstate    <= stat_spi_active_tx; 
            ss_o            <= '0';
            s_delay         <= c_clock_delay; 
          end if;

        -- process transfer
        when stat_spi_active_tx =>
          if (s_delay=0) then
            s_spiclk <= not s_spiclk;
            -- set spi clock to high if all bits are transmitted
            if (s_bitcounter=0) then 
              s_spiclk       <= '1';
              s_spitxstate   <= state_spi_end_tx;
            end if;
            -- next bit
            if(s_spiclk='0') then
                s_bitcounter <= s_bitcounter-1;  
            end if; 
          end if;
          
          -- end transfer and disable slave/chip select    
          when state_spi_end_tx =>
            ss_o            <= '1'; 
            s_tx_done       <= '1';
            s_tx_in_process <= '0';
            -- return to start transfer
            if (s_tx_start_i = '0') then
              s_spitxstate  <= state_spi_start_tx;
            end if;
    
        end case;
        
        -- latch last clock state
        s_spiclklast <= s_spiclk;
        
    end if; -- end check rising edge
  end process;   

  -- propergate data from register to single signal/pin
  p_propergate_data : process(clk_sys_i, rst_n_i)
  begin
    if (rst_n_i = '0') then 
      s_tx_reg   <= (others => '0');
    elsif (rising_edge(clk_sys_i)) then
      s_rx_read_o_last <= s_rx_read_o;
      if ( s_rx_read_o_last='1' and s_tx_start_i ='0') then
        s_tx_reg <= tx_data_i;
      end if; 
      if (s_spiclk='1' and s_spiclklast='0') then
        s_tx_reg <= s_tx_reg(s_tx_reg'left-1 downto 0) & s_tx_reg(0);
      end if;
    end if;
  end process;   
  
  -- delay data (data must change at falling edge of the spi clock)
  p_delay_tx_data : process(clk_sys_i, rst_n_i)
  begin
    if (rst_n_i = '0') then 
      s_tx_delayed_data   <= '0';
    elsif (rising_edge(clk_sys_i)) then
      if(s_delay>0) then
        null;
      else            
        s_tx_delayed_data <= s_tx_reg(s_tx_reg'left);
      end if;
    end if;
  end process;
  
end rtl;
