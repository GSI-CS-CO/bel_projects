LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;

entity Beam_Loss_check is
    generic (
    	  m            : integer := 8;       -- gate input width
        WIDTH        : integer := 20;      -- Counter width
        n            : integer range 0 to 62 :=62;        -- counter pool inputs:  hardware inputs plus test signals
        pos_threshold: integer:= 262144;
        neg_threshold: integer:= -262144
        
    );
    port (
        clk_sys       : in std_logic;      -- Clock
        rstn_sys      : in std_logic;      -- Reset
        CLEAR         : in std_logic;      -- Clear counter register
        LOAD          : in std_logic_vector(n-2 downto 0);     -- Load counter register
        watchdog_ena  : in std_logic_vector( 8 downto 0);      
        gate_in_ena   : in std_logic;
        Gate_Mtx      : in std_logic_vector (m-1 downto 0);
        In_Mtx       : in t_IO_Reg_0_to_7_Array; 
        test_sig_sel : in std_logic_vector(2 downto 0);
        out_mux_sel  : in std_logic_vector(12 downto 0);
        INTL_Output   : out std_logic_vector(5 downto 0) 
    
    );
end Beam_Loss_check;

architecture rtl of Beam_Loss_check is


signal    Interlock_IN:       std_logic_vector(8 downto 0) := (others =>'0');  -- outputs of the input stage logic
signal    gate_error:         std_logic_vector(1 downto 0);
signal    Gate_In_Mtx:        std_logic_vector (7 downto 0):= (OTHERS => '0');  -- gate outputs from the gate timing sequence control

signal    count_enable:       std_logic_vector(n-2 downto 0); 
signal    UP_IN:              std_logic_vector(n-2 downto 0); 
signal    DOWN_IN:            std_logic_vector(n-2 downto 0); 
signal    UP_OVERFLOW:        std_logic_vector((n-8)*(n-2)-1 downto 0);
signal    DOWN_OVERFLOW:      std_logic_vector((n-8)*(n-2)-1 downto 0);
signal    test_signal:        std_logic_vector(5 downto 0);


component BLM_gate_timing_seq is

    generic (
      freq    : natural range 500 TO 125000000 := 125000000;
      hold    : integer range 2 TO 10:= 2;
      n       : integer range 0 TO 11 :=4
    );
    port(
      clk_i : in std_logic;          -- chip-internal pulsed clk signal
      rstn_i : in std_logic;        -- reset signal
      gate_in : in std_logic_vector(n-1 downto 0);        -- input signal
      initialize : in std_logic;     -- enable '1' for input connected to the counter
      timeout_error : out std_logic; -- gate doesn't start within the given timeout
      gate_out: out std_logic_vector(n-1 downto 0)        -- out gate signal
    );
    end component BLM_gate_timing_seq;

    component BLM_watchdog is
      generic (
          freq    : natural range 500 TO 125000000 := 1250000000;
          hold    : integer range 2 TO 10:= 2;
          n       : integer range 0 TO 6 :=6
      );
      port(
          clk_i : in std_logic;     -- chip-internal pulsed clk signal
          rstn_i : in std_logic;   -- reset signal
          in_watchdog : in std_logic_vector(n-1 downto 0);     -- input signal
          ena_i : in std_logic;     -- enable '1' for input connected to the counter
          INTL_out: out std_logic   -- interlock output for signal that doesn't change for a given time (2 clocks)
      
      );
      end component BLM_watchdog;
      
      component BLM_counter_pool is
        generic (
          n            : integer :=62;        -- Counter_input width
            WIDTH        : integer := 20;      -- Counter width
            pos_threshold: integer:= 262144;
            neg_threshold: integer:= -262144       
        );
        port (
            CLK         : in std_logic;      -- Clock
            nRST         : in std_logic;      -- Reset
            CLEAR       : in std_logic;      -- Clear counter register
            LOAD        : in std_logic_vector(n-2 downto 0);      -- Load counter register
            ENABLE      : in std_logic_vector(n-2 downto 0);      -- Enable count operation
            UP_IN       : in std_logic_vector(n-2 downto 0);    -- Load counter register up input
            DOWN_IN     : in std_logic_vector(n-2 downto 0);    -- Load counter register down input
            UP_OVERFLOW    : out std_logic_vector((n-8)*(n-2)-1 downto 0) ;     -- UP_Counter overflow
            DOWN_OVERFLOW    : out std_logic_vector((n-8)*(n-2)-1 downto 0)      -- UP_Counter overflow
        );
        end component BLM_counter_pool;

        component BLM_Interlock_out is

          generic
                  (n : integer range 0 to 62 :=62        -- counter pool inputs:  hardware inputs plus test signals
                  
          );
          port (
                  CLK              : in std_logic;      -- Clock
                  nRST             : in std_logic;      -- Reset
                  out_mux_sel      : in std_logic_vector(12 downto 0);
                  UP_OVERFLOW      : in std_logic_vector((n-8)*(n-2)-1 downto 0) ; 
                  DOWN_OVERFLOW    : in std_logic_vector((n-8)*(n-2) -1 downto 0) ; 
                  gate_error       : in std_logic_vector(1 downto 0);
                  Interlock_IN     : in std_logic_vector(8 downto 0);

                  INTL_Output      : out std_logic_vector(5 downto 0) 
          );
          end component BLM_Interlock_out;
begin

----------------Input watchdog ------------------------------------------------------------------------
input1up_watchdog: BLM_watchdog 
  generic map(
      freq   => 125000000,
      hold   => 2,
      n      => 6
  )
  port map(
      clk_i => clk_sys,  
      rstn_i => rstn_sys,   -- reset signal
      in_watchdog => In_Mtx(0)(5 downto 0),
      ena_i =>    watchdog_ena(0),  -- enable for input connected to the counter
      INTL_out =>   Interlock_IN (0)-- interlock output for signal that doesn't change for a given time (2 clocks)
     
  );

  input1down_watchdog: BLM_watchdog 
  generic map(
      freq   => 125000000,
      hold   => 2,
      n      => 6
  )
  port map(
      clk_i => clk_sys,  
      rstn_i => rstn_sys,   -- reset signal
      in_watchdog => In_Mtx(0)(11 downto 6),
      ena_i =>   watchdog_ena(1),  -- enable for input connected to the counter
      INTL_out =>   Interlock_IN (1)-- interlock output for signal that doesn't change for a given time (2 clocks)

  );

  input2up_watchdog: BLM_watchdog 
  generic map(
      freq   => 125000000,
      hold   => 2,
      n      => 6
  )
  port map(
      clk_i => clk_sys,  
      rstn_i => rstn_sys,   -- reset signal
      in_watchdog => In_Mtx(1)(5 downto 0),
      ena_i =>     watchdog_ena(2),  -- enable for input connected to the counter
      INTL_out =>   Interlock_IN (2)-- interlock output for signal that doesn't change for a given time (2 clocks)
   
  );

  input2down_watchdog: BLM_watchdog 
  generic map(
      freq   => 125000000,
      hold   => 2,
      n      => 6
  )
  port map(
      clk_i => clk_sys,  
      rstn_i => rstn_sys,   -- reset signal
      in_watchdog => In_Mtx(1)(11 downto 6),
      ena_i =>    watchdog_ena(3),  -- enable for input connected to the counter
      INTL_out =>   Interlock_IN (3)-- interlock output for signal that doesn't change for a given time (2 clocks)
    
  );

input3up_watchdog: BLM_watchdog 
  generic map(
      freq   => 125000000,
      hold   => 2,
      n      => 6
  )
  port map(
      clk_i => clk_sys,  
      rstn_i => rstn_sys,   -- reset signal
      in_watchdog => In_Mtx(2)(5 downto 0),
      ena_i =>   watchdog_ena(4),  -- enable for input connected to the counter
      INTL_out =>   Interlock_IN (4)-- interlock output for signal that doesn't change for a given time (2 clocks)
     
  );

  input3down_watchdog: BLM_watchdog 
  generic map (
      freq   => 125000000,
      hold   => 2,
      n      => 6
  )
  port map(
      clk_i => clk_sys,  
      rstn_i => rstn_sys,   -- reset signal
      in_watchdog =>In_Mtx(2)(11 downto 6),
      ena_i =>   watchdog_ena(5),  -- enable for input connected to the counter
      INTL_out =>   Interlock_IN (5)-- interlock output for signal that doesn't change for a given time (2 clocks)
      
  );

  input4up_watchdog: BLM_watchdog 
  generic map(
      freq   => 125000000,
      hold   => 2,
      n      => 6
  )
  port map(
      clk_i => clk_sys,  
      rstn_i => rstn_sys,   -- reset signal
      in_watchdog => In_Mtx(3)(5 downto 0),
      ena_i =>    watchdog_ena(6),  -- enable for input connected to the counter
      INTL_out =>   Interlock_IN (6)-- interlock output for signal that doesn't change for a given time (2 clocks)
     
  );

  input4down_watchdog: BLM_watchdog 
  generic map(
      freq   => 125000000,
      hold   => 2,
      n      => 6
  )
  port map(
      clk_i => clk_sys,  
      rstn_i => rstn_sys,   -- reset signal
      in_watchdog => In_Mtx(3)(11 downto 6),
      ena_i =>    watchdog_ena(7),  -- enable for input connected to the counter
      INTL_out =>   Interlock_IN (7)-- interlock output for signal that doesn't change for a given time (2 clocks)
     
  );

 input5up_watchdog: BLM_watchdog 
  generic map(
      freq   => 125000000,
      hold   => 2,
      n      => 6
  )
  port map(
      clk_i => clk_sys,  
      rstn_i => rstn_sys,   -- reset signal
      in_watchdog => In_Mtx(4)(5 downto 0),
      ena_i =>   watchdog_ena(8),  -- enable for input connected to the counter
      INTL_out =>   Interlock_IN (8)-- interlock output for signal that doesn't change for a given time (2 clocks)
     
  );

  gate_board1: BLM_gate_timing_seq

    generic map (
      freq     => 125000000,
      hold     => 2,
      n        => 4
    )
    port map(
      clk_i => clk_sys,         -- chip-internal pulsed clk signal
      rstn_i => rstn_sys,         -- reset signal
      gate_in => Gate_Mtx(3 downto 0),       -- input signal
      initialize => gate_in_ena,  -- enable '1' for input connected to the counter
      timeout_error => gate_error(0), -- gate doesn't start within the given timeout
      gate_out => gate_In_Mtx(3 downto 0)       -- out gate signal
    );
   

gate_board2: BLM_gate_timing_seq

  generic map (
    freq     => 125000000,
    hold     => 2,
    n        => 4
  )
  port map(
    clk_i => clk_sys,         -- chip-internal pulsed clk signal
    rstn_i => rstn_sys,         -- reset signal
    gate_in => Gate_Mtx (7 downto 4),       -- input signal
    initialize => gate_in_ena,  -- enable '1' for input connected to the counter
    timeout_error => gate_error(1), -- gate doesn't start within the given timeout
    gate_out => gate_In_Mtx(7 downto 4)       -- out gate signal
  );
  

---- counter pool ------------------------------------------------------------------------------

BLM_counter_pool_inputs: process (rstn_sys, clk_sys)  --54 Inputs + 1 Test signal pro time (I use Test_in_Mtx(5 downto 0)... the MSB have been defined as "00")
begin
           if not rstn_sys='1' then 
       count_enable <= (others =>'0');
 --          UP_IN <= ( OTHERS => '0');
 --          DOWN_IN <= ( OTHERS => '0');
            test_signal <= (others =>'0');
           
       elsif (clk_sys'EVENT AND clk_sys = '1') then
            
            case test_sig_sel is 

               when "001" => test_signal <=  In_Mtx(4)(11 downto 6);
                when "010" => test_signal <= In_Mtx(5)(5 downto 0);
                when "011" => test_signal <=  In_Mtx(5)(11 downto 6);
                when "100" => test_signal <= In_Mtx(6)(5 downto 0);
                when "101" => test_signal <= In_Mtx(6)(11 downto 6);
                when others => null;

            end case;
            for i in 0 to (n-2) loop
                count_enable(i) <='1';
            end loop;
        end if;
    end process;
            UP_IN <=   "0"&test_signal & In_Mtx(4)(5 downto 0)& In_Mtx(3)(11 downto 0)&In_Mtx(2)(11 downto 0)&In_Mtx(1)(11 downto 0)&In_Mtx(0)(11 downto 0);
            DOWN_IN <="0"& test_signal & In_Mtx(4)(5 downto 0)& In_Mtx(3)(11 downto 0)&In_Mtx(2)(11 downto 0)&In_Mtx(1)(11 downto 0)&In_Mtx(0)(11 downto 0);
      --  end if;

--end process;


    BLM_Counter_pool_elem: BLM_counter_pool
      generic map (
          n        => 62,        -- Counter_input width
          WIDTH     =>  20,      -- Counter width
          pos_threshold => 262144,
          neg_threshold => -262144       
      )
      port map(
          CLK         => clk_sys,      -- Clock
          nRST        => rstn_sys,      -- Reset
          CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
          LOAD        => LOAD,      -- Load counter register
          ENABLE      => count_enable,     -- Enable count operation
          UP_IN       => UP_IN,    -- Load counter register up input
          DOWN_IN     => DOWN_IN,   -- Load counter register down input
          UP_OVERFLOW    => UP_OVERFLOW,    -- UP_Counter overflow
          DOWN_OVERFLOW  => DOWN_OVERFLOW    -- UP_Counter overflow
      );


Interlock_output: BLM_Interlock_out 

  generic map
          (n => 62       -- counter pool inputs:  hardware inputs plus test signals
          
  )
  port map(
          CLK          => clk_sys,
          nRST         => rstn_sys,
          out_mux_sel  => out_mux_sel,
          UP_OVERFLOW    => UP_OVERFLOW,    
          DOWN_OVERFLOW  => DOWN_OVERFLOW,   
          gate_error     => gate_error,
          Interlock_IN   => Interlock_IN,
          INTL_Output    => INTL_Output
  );

  
  end architecture;
