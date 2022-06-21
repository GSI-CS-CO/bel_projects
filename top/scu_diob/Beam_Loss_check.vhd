LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;

entity Beam_Loss_check is
    generic (
    	m            : integer := 8;       -- gate input width
        WIDTH        : integer := 20;      -- Counter width
        pos_threshold: integer:= 262144;
        neg_threshold: integer:= -262144
        
    );
    port (
        clk_sys       : in std_logic;      -- Clock
        rstn_sys      : in std_logic;      -- Reset
        CLEAR         : in std_logic;      -- Clear counter register
        LOAD          : in std_logic;      -- Load counter register
        watchdog_ena  : in std_logic_vector( 8 downto 0);      
        gate_in_ena   : in std_logic;
        Gate_Mtx      : in std_logic_vector (m-1 downto 0);
        In_Mtx       : in t_IO_Reg_0_to_7_Array; 
        INTL_Output   : out std_logic_vector(5 downto 0) 
    
    );
end Beam_Loss_check;

architecture rtl of Beam_Loss_check is


signal    Interlock_IN:       std_logic_vector(8 downto 0) := (others =>'0');  -- outputs of the input stage logic
signal    gate_error:         std_logic_vector(1 downto 0);
signal    Gate_In_Mtx:        std_logic_vector (7 downto 0):= (OTHERS => '0');  -- gate outputs from the gate timing sequence control #
signal    out_up_IL:             std_logic_vector (47 downto 0);  -- outputs of the Magnitude comparators of the up_down counters
signal    out_down_IL:             std_logic_vector (47 downto 0);
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
      
      component up_down_counter is
        generic (
          n            : integer :=6;        -- Counter_input width
          WIDTH        : integer := 20;      -- Counter width
          pos_threshold: integer:= 262144;
          neg_threshold: integer:= -262144
            
        );
        port (
            CLK         : in std_logic;      -- Clock
            nRST         : in std_logic;      -- Reset
            CLEAR       : in std_logic;      -- Clear counter register
            LOAD        : in std_logic;      -- Load counter register
            ENABLE      : in std_logic;      -- Enable count operation
            UP_IN       : in std_logic_vector(n-1 downto 0);    -- Load counter register up input
            DOWN_IN     : in std_logic_vector(n-1 downto 0);    -- Load counter register down input
            UP_OVERFLOW    : out std_logic ;     -- UP_Counter overflow
            DOWN_OVERFLOW    : out std_logic      -- UP_Counter overflow
        
        );
        end component up_down_counter;
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
  

---------------------------------------------------------------------------------------------------------
  ---- counter pool ----------------------------------------------------------------------------------------
  ----------------------------------------------------------------------------------------------------------

  counter0_4: for i in 0 to 4 generate
  counter_0_4: up_down_counter 
    generic map(
    	n             => 6,        -- Counter_input width
      WIDTH         => 20,    -- Counter width
      pos_threshold => 262144,
      neg_threshold =>-262144
        
    )
    port map (
        CLK     => clk_sys,      -- Clock
        nRST         =>rstn_sys,      -- Reset
        CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
        LOAD        => LOAD,     -- Load counter register to be defined by the control register
        ENABLE      => gate_in_Mtx(i),      -- Enable count operation
        UP_IN       => In_Mtx(i)(5 downto 0),   -- Load counter register up input
        DOWN_IN    => In_Mtx(i)(11 downto 6),  -- Load counter register down input
        UP_OVERFLOW    => out_up_IL(i),    -- UP_Counter overflow
        DOWN_OVERFLOW  => out_down_IL(i)    -- Down_Counter overflow
    
    );
    end generate counter0_4;


  counter5_12:for i in 0 to 7 generate
  counter_5_12: up_down_counter 
    generic map(
    	n             => 6,        -- Counter_input width
      WIDTH         => 20,    -- Counter width
      pos_threshold => 262144,
      neg_threshold =>-262144
        
    )
    port map (
        CLK     => clk_sys,      -- Clock
        nRST         =>rstn_sys,      -- Reset
        CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
        LOAD        => LOAD, -- Load counter register to be defined by the control register
        ENABLE      => gate_in_Mtx(i),      -- Enable count operation
        UP_IN       => In_Mtx(7-i)(5 downto 0),   -- Load counter register up input
        DOWN_IN    => In_Mtx(i)(11 downto 6),  -- Load counter register down input
        UP_OVERFLOW    => out_up_IL(i+5),    -- UP_Counter overflow
        DOWN_OVERFLOW  => out_down_IL(i+5)   -- Down_Counter overflow
    
    );
    end generate counter5_12;


    counter_13: up_down_counter 
      generic map(
        n             => 6,        -- Counter_input width
        WIDTH         => 20,    -- Counter width
        pos_threshold => 262144,
        neg_threshold =>-262144
          
      )
      port map (
          CLK     => clk_sys,      -- Clock
          nRST         =>rstn_sys,      -- Reset
          CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
          LOAD        => LOAD, -- Load counter register to be defined by the control register
          ENABLE      => gate_in_Mtx(0),      -- Enable count operation
          UP_IN       => In_Mtx(0)(5 downto 0),   -- Load counter register up input
          DOWN_IN    => In_Mtx(5)(5 downto 0),  -- Load counter register down input
          UP_OVERFLOW    => out_up_IL(13),    -- UP_Counter overflow
          DOWN_OVERFLOW  => out_down_IL(13)    -- Down_Counter overflow
      
      );
 

   counter14_16: for i in 1 to 3 generate
  counter_14_16: up_down_counter 
    generic map(
    	n             => 6,        -- Counter_input width
      WIDTH         => 20,    -- Counter width
      pos_threshold => 262144,
      neg_threshold =>-262144
        
    )
    port map (
        CLK     => clk_sys,      -- Clock
        nRST         =>rstn_sys,      -- Reset
        CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
        LOAD        => LOAD,   -- Load counter register to be defined by the control register
        ENABLE      => gate_in_Mtx(i),      -- Enable count operation
        UP_IN       => In_Mtx(i)(5 downto 0),   -- Load counter register up input
        DOWN_IN    => In_Mtx(i-1)(11 downto 6),  -- Load counter register down input
        UP_OVERFLOW    => out_up_IL(i+13),    -- UP_Counter overflow
        DOWN_OVERFLOW  => out_down_IL(i+13)     -- Down_Counter overflow
    
    );
    end generate counter14_16;

    counter_17: up_down_counter 
      generic map(
        n             => 6,        -- Counter_input width
        WIDTH         => 20,    -- Counter width
        pos_threshold => 262144,
        neg_threshold =>-262144
          
      )
      port map (
          CLK     => clk_sys,      -- Clock
          nRST         =>rstn_sys,      -- Reset
          CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
          LOAD        => LOAD,  -- Load counter register to be defined by the control register
          ENABLE      => gate_in_Mtx(0),      -- Enable count operation
          UP_IN       => In_Mtx(3)(11 downto 6),   -- Load counter register up input
          DOWN_IN    => In_Mtx(5)(11 downto 6),  -- Load counter register down input
          UP_OVERFLOW    => out_up_IL(17),    -- UP_Counter overflow
          DOWN_OVERFLOW  => out_down_IL(17)    -- Down_Counter overflow
      
      );

    counter_18_20: for i in 2 to 4 generate
    counter18_20: up_down_counter 
      generic map(
        n             => 6,        -- Counter_input width
        WIDTH         => 20,    -- Counter width
        pos_threshold => 262144,
        neg_threshold =>-262144
          
      )
      port map (
          CLK     => clk_sys,      -- Clock
          nRST         =>rstn_sys,      -- Reset
          CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
          LOAD        => LOAD,   -- Load counter register to be defined by the control register
          ENABLE      => gate_in_Mtx(i),      -- Enable count operation
          UP_IN       => In_Mtx(i-2)(5 downto 0),   -- Load counter register up input
          DOWN_IN    => In_Mtx(i)(5 downto 0),  -- Load counter register down input
          UP_OVERFLOW    => out_up_IL(i+16),    -- UP_Counter overflow
          DOWN_OVERFLOW  => out_down_IL(i+16)     -- Down_Counter overflow
      
      );
      end generate counter_18_20;

      counter_21_22: for i in 2 to 3 generate
    counter21_22: up_down_counter 
      generic map(
        n             => 6,        -- Counter_input width
        WIDTH         => 20,    -- Counter width
        pos_threshold => 262144,
        neg_threshold =>-262144
          
      )
      port map (
          CLK     => clk_sys,      -- Clock
          nRST         =>rstn_sys,      -- Reset
          CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
          LOAD        => LOAD,    -- Load counter register to be defined by the control register
          ENABLE      => gate_in_Mtx(i),      -- Enable count operation
          UP_IN       => In_Mtx(i-2)(11 downto 6),   -- Load counter register up input
          DOWN_IN    => In_Mtx(i)(11 downto 6),  -- Load counter register down input
          UP_OVERFLOW    => out_up_IL(i+19),    -- UP_Counter overflow
          DOWN_OVERFLOW  => out_down_IL(i+19)     -- Down_Counter overflow
      
      );
      end generate counter_21_22;
      
      counter_23_25: for i in 0 to 2 generate
      counter23_25: up_down_counter 
        generic map(
          n             => 6,        -- Counter_input width
          WIDTH         => 20,    -- Counter width
          pos_threshold => 262144,
          neg_threshold =>-262144
            
        )
        port map (
            CLK     => clk_sys,      -- Clock
            nRST         =>rstn_sys,      -- Reset
            CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
            LOAD        => LOAD,            -- Load counter register to be defined by the control register
            ENABLE      => gate_in_Mtx(i),      -- Enable count operation
            UP_IN       => In_Mtx(i)(5 downto 0),   -- Load counter register up input
            DOWN_IN    => In_Mtx(i+1)(5 downto 0),  -- Load counter register down input
            UP_OVERFLOW    => out_up_IL(i+23),    -- UP_Counter overflow
            DOWN_OVERFLOW  => out_down_IL(i+23)     -- Down_Counter overflow
        
        );
        end generate counter_23_25;

        counter_26_27: for i in 0 to 1 generate
      counter26_27: up_down_counter 
        generic map(
          n             => 6,        -- Counter_input width
          WIDTH         => 20,    -- Counter width
          pos_threshold => 262144,
          neg_threshold =>-262144
            
        )
        port map (
            CLK     => clk_sys,      -- Clock
            nRST         =>rstn_sys,      -- Reset
            CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
            LOAD        => LOAD,    -- Load counter register to be defined by the control register
            ENABLE      => gate_in_Mtx(i),      -- Enable count operation
            UP_IN       => In_Mtx(i)(11 downto 6),   -- Load counter register up input
            DOWN_IN    => In_Mtx(i+1)(11 downto 6),  -- Load counter register down input
            UP_OVERFLOW    => out_up_IL(i+26),    -- UP_Counter overflow
            DOWN_OVERFLOW  => out_down_IL(i+26)     -- Down_Counter overflow
        
        );
        end generate counter_26_27;

        counter_28_30: for i in 0 to 2 generate
        counter28_30: up_down_counter 
          generic map(
            n             => 6,        -- Counter_input width
            WIDTH         => 20,    -- Counter width
            pos_threshold => 262144,
            neg_threshold =>-262144
              
          )
          port map (
              CLK     => clk_sys,      -- Clock
              nRST         =>rstn_sys,      -- Reset
              CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
              LOAD        => LOAD,   -- Load counter register to be defined by the control register
              ENABLE      => gate_in_Mtx(i),      -- Enable count operation
              UP_IN       => In_Mtx(i)(5 downto 0),   -- Load counter register up input
              DOWN_IN    => In_Mtx(i+1)(11 downto 6),  -- Load counter register down input
              UP_OVERFLOW    => out_up_IL(i+28),    -- UP_Counter overflow
              DOWN_OVERFLOW  => out_down_IL(i+28)     -- Down_Counter overflow
          
          );
          end generate counter_28_30;

          counter_31_33: for i in 0 to 2 generate
      
        counter31_33: up_down_counter 
          generic map(
            n             => 6,        -- Counter_input width
            WIDTH         => 20,    -- Counter width
            pos_threshold => 262144,
            neg_threshold =>-262144
              
          )
          port map (
              CLK     => clk_sys,      -- Clock
              nRST         =>rstn_sys,      -- Reset
              CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
              LOAD        => LOAD,    -- Load counter register to be defined by the control register
              ENABLE      => gate_in_Mtx(i),      -- Enable count operation
              UP_IN       => In_Mtx(i)(11 downto 6),   -- Load counter register up input
              DOWN_IN    => In_Mtx(i+2)(5 downto 0),  -- Load counter register down input
              UP_OVERFLOW    => out_up_IL(i+31),    -- UP_Counter overflow
              DOWN_OVERFLOW  => out_down_IL(i+31)     -- Down_Counter overflow
          
          );

          end generate counter_31_33; 

          counter_34_35: for i in 0 to 1 generate
      
          counter34_35: up_down_counter 
            generic map(
              n             => 6,        -- Counter_input width
              WIDTH         => 20,    -- Counter width
              pos_threshold => 262144,
              neg_threshold =>-262144
                
            )
            port map (
                CLK     => clk_sys,      -- Clock
                nRST         =>rstn_sys,      -- Reset
                CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
                LOAD        => LOAD,    -- Load counter register to be defined by the control register
                ENABLE      => gate_in_Mtx(i),      -- Enable count operation
                UP_IN       => In_Mtx(i)(5 downto 0),   -- Load counter register up input
                DOWN_IN    => In_Mtx(i+3)(5 downto 0),  -- Load counter register down input
                UP_OVERFLOW    => out_up_IL(i+34),    -- UP_Counter overflow
                DOWN_OVERFLOW  => out_down_IL(i+34)     -- Down_Counter overflow
            
            );
  
            end generate counter_34_35; 


          counter_36_37: for i in 0 to 1 generate
      
          counter36_37: up_down_counter 
            generic map(
              n             => 6,        -- Counter_input width
              WIDTH         => 20,    -- Counter width
              pos_threshold => 262144,
              neg_threshold =>-262144
                
            )
            port map (
                CLK     => clk_sys,      -- Clock
                nRST         =>rstn_sys,      -- Reset
                CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
                LOAD        => LOAD,   -- Load counter register to be defined by the control register
                ENABLE      => gate_in_Mtx(i),      -- Enable count operation
                UP_IN       => In_Mtx(i)(11 downto 6),   -- Load counter register up input
                DOWN_IN    => In_Mtx(i+3)(11 downto 6),  -- Load counter register down input
                UP_OVERFLOW    => out_up_IL(i+36),    -- UP_Counter overflow
                DOWN_OVERFLOW  => out_down_IL(i+36)     -- Down_Counter overflow
            
            );
  
            end generate counter_36_37;
            
            counter_38_39: for i in 0 to 1 generate

            counter38_39: up_down_counter 
            generic map(
              n             => 6,        -- Counter_input width
              WIDTH         => 20,    -- Counter width
              pos_threshold => 262144,
              neg_threshold =>-262144
                
            )
            port map (
                CLK     => clk_sys,      -- Clock
                nRST         =>rstn_sys,      -- Reset
                CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
                LOAD        => LOAD,    -- Load counter register to be defined by the control register
                ENABLE      => gate_in_Mtx(i),      -- Enable count operation
                UP_IN       => In_Mtx(i)(5 downto 0),   -- Load counter register up input
                DOWN_IN    => In_Mtx(i+3)(11 downto 6),  -- Load counter register down input
                UP_OVERFLOW    => out_up_IL(i+38),    -- UP_Counter overflow
                DOWN_OVERFLOW  => out_down_IL(i+38)     -- Down_Counter overflow
            
            );
  
            end generate counter_38_39; 

            counter_40_41: for i in 0 to 1 generate

            counter40_41: up_down_counter 
            generic map(
              n             => 6,        -- Counter_input width
              WIDTH         => 20,    -- Counter width
              pos_threshold => 262144,
              neg_threshold =>-262144
                
            )
            port map (
                CLK     => clk_sys,      -- Clock
                nRST         =>rstn_sys,      -- Reset
                CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
                LOAD        => LOAD,    -- Load counter register to be defined by the control register
                ENABLE      => gate_in_Mtx(i),      -- Enable count operation
                UP_IN       => In_Mtx(i)(11 downto 6),   -- Load counter register up input
                DOWN_IN    => In_Mtx(i+4)(5 downto 0),  -- Load counter register down input
                UP_OVERFLOW    => out_up_IL(i+40),    -- UP_Counter overflow
                DOWN_OVERFLOW  => out_down_IL(i+40)     -- Down_Counter overflow
            
            );
  
            end generate counter_40_41; 

            counter_42_43: for i in 0 to 1 generate

            counter42_43: up_down_counter 
            generic map(
              n             => 6,        -- Counter_input width
              WIDTH         => 20,    -- Counter width
              pos_threshold => 262144,
              neg_threshold =>-262144
                
            )
            port map (
                CLK     => clk_sys,      -- Clock
                nRST         =>rstn_sys,      -- Reset
                CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
                LOAD        => LOAD,    -- Load counter register to be defined by the control register
                ENABLE      => gate_in_Mtx(i),      -- Enable count operation
                UP_IN       => In_Mtx(i)(5 downto 0),   -- Load counter register up input
                DOWN_IN    => In_Mtx(i+4)(5 downto 0),  -- Load counter register down input
                UP_OVERFLOW    => out_up_IL(i+42),    -- UP_Counter overflow
                DOWN_OVERFLOW  => out_down_IL(i+42)     -- Down_Counter overflow
            
            );
  
            end generate counter_42_43; 

            counter_44_45: for i in 0 to 1 generate

            counter44_45: up_down_counter 
            generic map(
              n             => 6,        -- Counter_input width
              WIDTH         => 20,    -- Counter width
              pos_threshold => 262144,
              neg_threshold =>-262144
                
            )
            port map (
                CLK     => clk_sys,      -- Clock
                nRST         =>rstn_sys,      -- Reset
                CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
                LOAD        => LOAD,     -- Load counter register to be defined by the control register
                ENABLE      => gate_in_Mtx(i),      -- Enable count operation
                UP_IN       => In_Mtx(i)(5 downto 0),   -- Load counter register up input
                DOWN_IN    => In_Mtx(i+2)(11 downto 6),  -- Load counter register down input
                UP_OVERFLOW    => out_up_IL(i+44),    -- UP_Counter overflow
                DOWN_OVERFLOW  => out_down_IL(i+44)     -- Down_Counter overflow
            
            );
  
            end generate counter_44_45; 

            counter_46_47: for i in 0 to 1 generate

            counter46_47: up_down_counter 
            generic map(
              n             => 6,        -- Counter_input width
              WIDTH         => 20,    -- Counter width
              pos_threshold => 262144,
              neg_threshold =>-262144
                
            )
            port map (
                CLK     => clk_sys,      -- Clock
                nRST         => rstn_sys,     -- Reset
                CLEAR       => CLEAR,       -- Clear counter register to be defined by the control register
                LOAD        => LOAD,  -- Load counter register to be defined by the control register
                ENABLE      => gate_in_Mtx(i),      -- Enable count operation
                UP_IN       => In_Mtx(i)(11 downto 6),   -- Load counter register up input
                DOWN_IN    => In_Mtx(i+3)(5 downto 0),  -- Load counter register down input
                UP_OVERFLOW    => out_up_IL(i+46),    -- UP_Counter overflow
                DOWN_OVERFLOW  => out_down_IL(i+46)     -- Down_Counter overflow
            
            );
  
            end generate counter_46_47; 
        
Interlock_output_process: process(rstn_sys, clk_sys)


  begin
    if  ( not rstn_sys    = '1') then
      for i in 0 to 5 loop
        INTL_Output(i) <= '0';
        end loop;
    elsif (rising_edge(clk_sys)) then
      
         INTL_Output(0) <= not(Interlock_IN(0) or Interlock_IN(1) or Interlock_IN(2) or Interlock_IN(3) or Interlock_IN(4) or Interlock_IN(5) or Interlock_IN(6) or Interlock_IN(7) or Interlock_IN(8));
         INTL_Output(1) <= gate_error(0) nor gate_error(1);
         INTL_Output(2) <= not(out_up_IL(0) or out_up_IL(1) or out_up_IL(2) or out_up_IL(3) or out_up_IL(4) or out_up_IL(5) or out_up_IL(6) or out_up_IL(7) or out_up_IL(8) or out_up_IL(9) or out_up_IL(10) 
                             or  out_up_IL(11) or out_up_IL(12) or out_up_IL(13)or out_up_IL(14) or out_up_IL(15) or out_up_IL(16) or out_up_IL(17) or out_up_IL(18) or out_up_IL(19) or out_up_IL(20) or 
                             out_up_IL(21)or out_up_IL(22) or out_up_IL(23) or out_up_IL(24)); 
         INTL_Output(3) <= not(  out_up_IL(25) or out_up_IL(26) or out_up_IL(27) or out_up_IL(28) or out_up_IL(29) or out_up_IL(30) or out_up_IL(31)or out_up_IL(32)or out_up_IL(33) or out_up_IL(34)or 
                                out_up_IL(35)or out_up_IL(36)or out_up_IL(37)or out_up_IL(38)or out_up_IL(39)or out_up_IL(40)or out_up_IL(41)or out_up_IL(42)or out_up_IL(43)or out_up_IL(44)or out_up_IL(45)or 
                                out_up_IL(46)or out_up_IL(47));
         INTL_Output(4) <= not(out_down_IL(0) or out_down_IL(1) or out_down_IL(2) or out_down_IL(3) or out_down_IL(4) or out_down_IL(5) or out_down_IL(6) or out_down_IL(7) or out_down_IL(8) or out_down_IL(9) 
                                or out_down_IL(10) or out_down_IL(11) or out_down_IL(12) or out_down_IL(13) or out_down_IL(14) or out_down_IL(15) or out_down_IL(16) or out_down_IL(17) or out_down_IL(18) or 
                                out_down_IL(19) or out_down_IL(20) or out_down_IL(21)or out_down_IL(22) or  out_down_IL(23)or out_down_IL(24));
         INTL_Output(5) <= not(  out_down_IL(25)or out_down_IL(26)or out_down_IL(27) or out_down_IL(28) or out_down_IL(29) or out_down_IL(30) or out_down_IL(31)or 
                                out_down_IL(32)or out_down_IL(33) or out_down_IL(34)or out_down_IL(35)or out_down_IL(36)or out_down_IL(37)or out_down_IL(38)or out_down_IL(39)or out_down_IL(40)or out_down_IL(41)or 
                                out_down_IL(42)or out_down_IL(43)or out_down_IL(44)or out_down_IL(45)or out_down_IL(46)or out_down_IL(47)); 
     
    end if;
  end process;

  
  end architecture;
