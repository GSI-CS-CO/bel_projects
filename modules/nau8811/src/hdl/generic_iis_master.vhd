-- libraries and packages
-- ieee
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- entity
entity generic_iis_master is
    Generic ( g_word_width   : natural := 32 -- bit(s)
             ); 
    Port ( clk_sys_i         : in  std_logic;
           rst_n_i           : in  std_logic;
           tx_data_i         : in  std_logic_vector (g_word_width-1 downto 0);
           rx_data_o         : out std_logic_vector (g_word_width-1 downto 0);
           iis_fs_o          : out std_logic;
           iis_bclk_o        : out std_logic;
           iis_adcout_o      : out std_logic;
           iis_dacin_i       : in  std_logic;
           iis_enable_clk_i  : in  std_logic;
           iis_transaction_i : in  std_logic;
           iis_mono_output_i : in  std_logic;
           iis_invert_fs_i   : in  std_logic;
           iis_padding_i     : in  std_logic;
           iis_heartbeat_i   : in  std_logic;
           iis_trigger_pol_i : in  std_logic;
           iis_trigger_i     : in  std_logic;
           tx_fifo_empty_i   : in  std_logic;
           rx_fifo_full_i    : in  std_logic;
           tx_read_o         : out std_logic;
           rx_write_o        : out std_logic
         );
end generic_iis_master;

architecture rtl of generic_iis_master is
  
  -- internal synchronization 
  signal   s_clock_phase_counter : natural range 0 to ((2*g_word_width*4)-1);
  signal   s_bit_counter         : natural range 0 to ((2*g_word_width*4)-1);
  signal   s_frame_sync          : std_logic;
  signal   s_clock_phase         : std_logic;
  signal   s_read_from_fifo      : std_logic;
  signal   s_transaction         : std_logic;
  signal   s_data_to_send        : std_logic_vector(0 to (g_word_width-1));
  signal   s_received_data       : std_logic_vector(0 to (g_word_width-1));
  
  -- heartbeat
  constant c_sample_frequency         : natural := 48000; -- Hz
  constant c_tone_frequency           : natural := 500; -- Hz
  constant c_tone_duration            : natural := 100; -- ms
  constant c_period_length            : natural := c_sample_frequency/c_tone_frequency; -- samples needed to output one period
  constant c_period_tone              : natural := (c_tone_duration*c_tone_frequency)/1000; -- times to play the square-wave sound
  signal   s_heartbeat_counter        : natural range 0 to c_period_length+1;
  signal   s_heartbeat_period_counter : natural range 0 to c_period_tone+1;
  type     t_iis_heartbeat_state is (state_iis_heartbeat_waiting, state_iis_heartbeat_sync, state_iis_heartbeat_active);
  signal   s_iis_heartbeat_state      : t_iis_heartbeat_state := state_iis_heartbeat_waiting;
  signal   s_current_trigger          : std_logic; -- actual trigger signal
  signal   s_prev_trigger             : std_logic; -- value of the previous trigger signal
  signal   s_sync_trigger             : std_logic; -- sync stage for trigger signal
  signal   s_sync_trigger_pre         : std_logic; -- sync pre. stage for trigger signal  
  signal   s_frame_done               : std_logic; -- pulse every new frame sync cycle
  
begin
  
  p_generate_bit_clock : process(clk_sys_i, rst_n_i)
  begin
    -- reset
    if (rst_n_i = '0') then
      s_clock_phase     <= '0';
      iis_bclk_o        <= '0';
    elsif (rising_edge(clk_sys_i)) then
    -- check if the clock generator is enabled
      if (iis_enable_clk_i = '0') then
        s_clock_phase   <= '0';
        iis_bclk_o      <= '0';
      else
        -- phase should be used for clock divider >=2
        if (s_clock_phase = '1') then
          s_clock_phase <= '0';
          iis_bclk_o    <= '0';
        else
          s_clock_phase <= '1';
          iis_bclk_o    <= '1';
        end if;
      end if;
    end if;
  end process;
  
  p_generate_bit_counter : process(clk_sys_i, rst_n_i)
  begin
    -- reset
    if (rst_n_i = '0') then
      -- set the bit counter to zero
      s_bit_counter <= 0;
    elsif (rising_edge(clk_sys_i)) then
      -- check if the clock generator is enabled
      if (iis_enable_clk_i = '0') then
        s_bit_counter <= 0;
      else
        if (s_clock_phase = '0') then
          -- check if two words have been sent
          if (s_bit_counter=(2*g_word_width*4)-1) then
            s_bit_counter <= 0;
          -- otherwise just count up
          else
            s_bit_counter <= s_bit_counter+1;
          end if;
        end if;
      end if;
    end if;
  end process;
  
  p_generate_frame_sync : process(clk_sys_i, rst_n_i)
  begin
    -- reset
    if (rst_n_i = '0') then 
      -- reset all status/values to zero
      s_frame_sync <= '1';
      s_heartbeat_counter <= 0;
      s_frame_done <= '0';
    -- process
    elsif (rising_edge(clk_sys_i)) then
      -- check if the clock generator is enabled
      if (iis_enable_clk_i = '0') then
        s_frame_sync <= '1';
      else
        -- only change the frame sync signal under these circumstances (to be synchronous with the falling bit clock edge)
        if (s_clock_phase = '1') then
          if (s_bit_counter=((g_word_width*2*4)-2)) then
            s_frame_sync <= '0';
            -- signal for heartbeat
            s_frame_done <= '1';
          elsif (s_bit_counter=((g_word_width*4)-2)) then
            s_frame_sync <= '1';
            -- handle heartbeat signal
            if (s_iis_heartbeat_state = state_iis_heartbeat_active) then
              if (s_heartbeat_counter = (c_period_length-1)) then
                s_heartbeat_counter <= 0;
              else
                s_heartbeat_counter <= s_heartbeat_counter + 1;
              end if;
            elsif (s_iis_heartbeat_state = state_iis_heartbeat_waiting) then
                s_heartbeat_counter <= 0;
            end if;
          else
            -- signal for heartbeat
            s_frame_done <= '0';
          end if;
        end if;
      end if;
    end if;
  end process;
  
  -- manage read access to fifo
  p_provide_read_from_fifo : process(clk_sys_i, rst_n_i)
  begin
    -- reset
    if (rst_n_i = '0') then
      tx_read_o <= '0';
      s_transaction <= '0';
      s_data_to_send <= (others => '0');
    -- process
    elsif (rising_edge(clk_sys_i)) then
      -- read data from fifo
        if ((s_bit_counter=((g_word_width*4*2)-2)) and tx_fifo_empty_i='0' and s_clock_phase = '0') then
          tx_read_o <= '1';
          s_transaction <= '1';
          s_data_to_send <= tx_data_i;
        elsif ((s_bit_counter=((g_word_width*4*2)-2)) and s_clock_phase = '1') then
          s_transaction <= '0';
          tx_read_o <= '0';
        else
          tx_read_o <= '0';
        end if; 
      end if;
  end process;
  
  -- send data
  p_send_data : process(clk_sys_i, rst_n_i)
  begin
    -- reset
    if (rst_n_i = '0') then
      iis_adcout_o               <= '0';
      s_current_trigger          <= '0';
      s_prev_trigger             <= '0';
      s_iis_heartbeat_state      <= state_iis_heartbeat_waiting;
      s_heartbeat_period_counter <= 0;
    -- process
    elsif (rising_edge(clk_sys_i)) then
      if (s_clock_phase = '1') then
        -- sync trigger signals and latch them
        s_current_trigger <= s_sync_trigger;
        s_prev_trigger    <= s_current_trigger;
        if (iis_heartbeat_i = '0') then
        -- normal mode
        -- use the next bit (overflow protection)
          if (s_bit_counter=(g_word_width*2*4)-1) then
            iis_adcout_o <= s_data_to_send(0);
          elsif (s_bit_counter<g_word_width-1) then
            iis_adcout_o <= s_data_to_send(s_bit_counter+1);
          elsif (s_bit_counter<g_word_width*4-1) then
            -- unused bit slots (first channel)
            iis_adcout_o <= iis_padding_i;
          elsif (s_bit_counter<g_word_width*5-1) then
            -- send padding value or output to both channels
            if (iis_mono_output_i = '0') then
              iis_adcout_o <= s_data_to_send(s_bit_counter-((g_word_width*4)-1));
            else
              iis_adcout_o <= iis_padding_i;
            end if;
          else 
            -- unused bit slots (second channel)
            iis_adcout_o <= iis_padding_i;
          end if;
        else
          -- heartbeat mode
          -- wait for falling trigger edge
          if (s_iis_heartbeat_state = state_iis_heartbeat_waiting and s_prev_trigger = '1' and s_current_trigger = '0' and iis_trigger_pol_i = '0') then
            s_iis_heartbeat_state <= state_iis_heartbeat_sync;
          -- wait for rising trigger edge
          elsif (s_iis_heartbeat_state = state_iis_heartbeat_waiting and s_prev_trigger = '0' and s_current_trigger = '1' and iis_trigger_pol_i = '1') then
            s_iis_heartbeat_state <= state_iis_heartbeat_sync;
          elsif (s_iis_heartbeat_state = state_iis_heartbeat_sync ) then
            -- sync to frame sync signal
            if (s_bit_counter=(g_word_width*2*4)-2) then
              s_iis_heartbeat_state <= state_iis_heartbeat_active;
            end if;
          elsif (s_iis_heartbeat_state = state_iis_heartbeat_active) then
            -- generate square-wave-signal
            if (s_heartbeat_counter < (c_period_length/2)) then
              -- full positive value (no minus sign)
              if(s_bit_counter<g_word_width-1) then
                iis_adcout_o <= '1';
              else
                iis_adcout_o <= '0';
              end if;
            else
             -- full negative value (minus sign)
              if(s_bit_counter<g_word_width-1 or s_bit_counter=(g_word_width*2*4)-1) then
                iis_adcout_o <= '1';
              else
                iis_adcout_o <= '0';
              end if;
            end if;
            -- increase period counter if a frame sync cycle is done
            if (s_frame_done = '1' and s_heartbeat_counter = (c_period_length-1)) then
              s_heartbeat_period_counter <= s_heartbeat_period_counter + 1;
            end if;
            -- check if heartbeat is done
            if (s_heartbeat_period_counter = c_period_tone) then
              s_iis_heartbeat_state <= state_iis_heartbeat_waiting;
              s_heartbeat_period_counter <= 0;
            end if;
          end if;
        end if;
      end if;
    end if;
  end process;
  
  -- receive data
  p_receive_data : process(clk_sys_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      s_received_data <= (others => '0');
    elsif (rising_edge(clk_sys_i)) then
       if (s_clock_phase = '0') then
        -- sample data
          if (s_bit_counter=(g_word_width*2*4)-1) then
            s_received_data(0) <= iis_dacin_i;
          elsif (s_bit_counter<(g_word_width-1)) then
            s_received_data(s_bit_counter+1) <= iis_dacin_i;
          end if;
        end if;
        -- write data to fifo depending on the transmission setting
        if (s_bit_counter=(g_word_width-1) and rx_fifo_full_i='0' and s_clock_phase = '0') then
          -- always write data to fifo
          if (iis_transaction_i='1') then
            rx_write_o <= '1';
          -- only write data to fifo if a valid transaction (tx and rx) is active
          else
            rx_write_o <= s_transaction;
          end if;
        else 
          rx_write_o <= '0';
        end if;
    end if;
  end process;
    
  -- propagate frame sync
  p_propergate_frame_sync : process(s_frame_sync, iis_invert_fs_i)
  begin
    if (iis_invert_fs_i='0') then
      iis_fs_o <= s_frame_sync;
    else
      iis_fs_o <= not(s_frame_sync);
    end if;
  end process;

  -- provide sampled data
  p_provide_rx_ouput : process(s_received_data)
  begin
    rx_data_o <= s_received_data;
  end process;
  
  -- sync trigger signal
  p_sync_trigger : process(clk_sys_i, rst_n_i)
  begin
    if (rst_n_i = '0') then
      s_sync_trigger_pre <= '0';
      s_sync_trigger     <= '0';
    elsif (rising_edge(clk_sys_i)) then
      s_sync_trigger_pre <= iis_trigger_i;
      s_sync_trigger     <= s_sync_trigger_pre;
    end if;
  end process;
  
end rtl;
