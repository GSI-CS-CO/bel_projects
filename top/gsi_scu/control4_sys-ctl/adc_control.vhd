----------------------------------------------------------------------------------
-- Engineer:    Kai Lüghausen
--
----------------------------------------------------------------------------------


library IEEE;
 use IEEE.STD_LOGIC_1164.ALL;
 use IEEE.NUMERIC_STD.ALL;
 use ieee.std_logic_unsigned.all;


entity adc_control is
    Port ( clk : in STD_LOGIC;
           clk_pll : in std_logic;
           pll_locked :in std_logic;
           nreset : in STD_LOGIC;
           channel_0: out STD_LOGIC_VECTOR (11 downto 0);  
           channel_1: out STD_LOGIC_VECTOR (11 downto 0); 
           channel_2: out STD_LOGIC_VECTOR (11 downto 0);  
           channel_3: out STD_LOGIC_VECTOR (11 downto 0); 
           channel_4: out STD_LOGIC_VECTOR (11 downto 0);  
           channel_5: out STD_LOGIC_VECTOR (11 downto 0);
           channel_6: out STD_LOGIC_VECTOR (11 downto 0); 
           channel_7: out STD_LOGIC_VECTOR (11 downto 0);  
           channel_8: out STD_LOGIC_VECTOR (11 downto 0); 
           tsd: out STD_LOGIC_VECTOR  (11 downto 0));  
end adc_control;

architecture Behavioral of adc_control is


    component adc is
        port (
            adc_pll_clock_clk      : in  std_logic                     := 'X';             -- clk
            adc_pll_locked_export  : in  std_logic                     := 'X';             -- export
            clock_clk              : in  std_logic                     := 'X';             -- clk
            command_valid          : in  std_logic                     := 'X';             -- valid
            command_channel        : in  std_logic_vector(4 downto 0)  := (others => 'X'); -- channel
            command_startofpacket  : in  std_logic                     := 'X';             -- startofpacket
            command_endofpacket    : in  std_logic                     := 'X';             -- endofpacket
            command_ready          : out std_logic;                                        -- ready
            reset_sink_reset_n     : in  std_logic                     := 'X';             -- reset_n
            response_valid         : out std_logic;                                        -- valid
            response_channel       : out std_logic_vector(4 downto 0);                     -- channel
            response_data          : out std_logic_vector(11 downto 0);                    -- data
            response_startofpacket : out std_logic;                                        -- startofpacket
            response_endofpacket   : out std_logic                                         -- endofpacket
        );
    end component adc;


    type states is (s_0,s_1,s_2,s_3,s_4);
    signal state, next_state    : states;
    TYPE   t_channel_array         is array (0 to 8) of std_logic_vector(11 downto 0);
    signal channel_buf          : t_channel_array;    
    signal counter              : std_logic_vector(4 downto 0) :="00000";
	signal counter_reset		: std_logic;
    signal addr_buf             : std_logic_vector(4 downto 0) :="00000";
    signal count_en             : std_logic;
    signal den                  : std_logic;
    signal channel              : std_logic_vector(11 downto 0);
    signal tsd_buf              : std_logic_vector(11 downto 0);
    signal adc_data             : std_logic_vector(11 downto 0);
    signal adc_dvalid           : std_logic;
	signal rchannel             : std_logic_vector(4 downto 0) :="00000";
	signal cready               : std_logic;


begin



pr_statemachine: process (nreset, clk_pll)
    begin
        if (nreset = '0') then
            state <= s_0;
        elsif (rising_edge(clk_pll)) then
            state <= next_state;
        end if;
end process;

pr_statemachine_decode: process (state,adc_dvalid)
    begin
        case state is
            when s_0 =>
                count_en                <= '0';
					 counter_reset <= '0';
                 next_state             <= s_1;
            when s_1 =>
                count_en                <= '0';
					 counter_reset <= '0';
                next_state              <= s_2;
            when s_2 =>
                count_en                <= '0';
					 counter_reset <= '0';
					 if ( cready = '1' and cready = '1')  then 
							next_state         <= s_4;
                elsif ( cready = '1' and cready = '0') then
                    next_state          <= s_3;
                else
                    next_state          <= s_2;
                end if;
            when s_3 =>
                count_en                <= '0';
					 counter_reset <= '0';
                if (adc_dvalid = '1') then
                    next_state          <= s_4;
                else
                    next_state          <= s_3;
                end if;
            when s_4 =>
					 if counter = "10001" then
						counter_reset <= '1';
						count_en                <= '0';
						else
						count_en                <= '1';
						counter_reset <= '0';
						end if;
                next_state              <= s_0;
            when others =>
                next_state    <= s_0;
                count_en                <= '0';
					 counter_reset <= '0';
        end case;
end process;


process (clk_pll)
    begin
    if rising_edge(clk_pll) then
        if nreset = '0' then
        counter <= (others => '0');
		  elsif counter_reset = '1' then
		  counter <= (others => '0');
        elsif count_en = '1' then
        counter <=  counter + '1';
        end if;
    end if;
end process;


process (clk_pll)
   begin
        if rising_edge(clk_pll) then
               if (adc_dvalid= '1') and (rchannel = "00000") then
                channel_buf(0) <= adc_data(11 downto 0);    
               elsif (adc_dvalid= '1') and (rchannel = "00001") then
                channel_buf(1) <= adc_data(11 downto 0);  
               elsif (adc_dvalid= '1') and (rchannel = "00010") then
                channel_buf(2) <= adc_data(11 downto 0);  
               elsif (adc_dvalid= '1') and (rchannel = "00011") then
                channel_buf(3) <= adc_data(11 downto 0);
               elsif (adc_dvalid= '1') and (rchannel = "00100") then
                channel_buf(4) <= adc_data(11 downto 0);  
               elsif (adc_dvalid= '1') and (rchannel = "00101") then
                channel_buf(5) <= adc_data(11 downto 0);  
               elsif (adc_dvalid= '1') and (rchannel = "00110") then
                channel_buf(6) <= adc_data(11 downto 0);
               elsif (adc_dvalid= '1') and (rchannel = "00111") then
                channel_buf(7) <= adc_data(11 downto 0);  
               elsif (adc_dvalid= '1') and (rchannel = "01000") then
                channel_buf(8) <= adc_data(11 downto 0);   
               elsif (adc_dvalid= '1') and (rchannel = "10001") then
                tsd_buf <= adc_data(11 downto 0);   
			   end if;
        end if;
end process;

  --ADC - Analog Digital-Wandler
  adc0 : component adc
  port map (
      adc_pll_clock_clk      => clk_pll,        -- adc_pll_clock.clk
      adc_pll_locked_export  => pll_locked,     -- adc_pll_locked.export
      clock_clk              => clk_pll,            -- clock.clk
      command_valid          => nreset,            -- command.valid
      command_channel        =>  counter,        -- .channel
      command_startofpacket  => 'X',            -- .startofpacket
      command_endofpacket    => 'X',            -- .endofpacket
      command_ready          => cready,           -- .ready
      reset_sink_reset_n     => nreset,          --  reset_sink.reset_n
      response_valid         => adc_dvalid,     --  response.valid
      response_channel       => rchannel,           -- .channel
      response_data          => adc_data,       -- .data
      response_startofpacket => open,           -- .startofpacket
      response_endofpacket   => open            -- .endofpacket
  );

 process (clk)
   begin
        if rising_edge(clk) then
			channel_0<= channel_buf(0);
			channel_1<= channel_buf(1);
			channel_2<= channel_buf(2);
			channel_3<= channel_buf(3);
			channel_4<= channel_buf(4);
			channel_5<= channel_buf(5);
			channel_6<= channel_buf(6);
			channel_7<= channel_buf(7);
			channel_8<= channel_buf(8);
			tsd <= tsd_buf;
        end if;
end process;


end Behavioral;
