LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;

entity Beam_Loss_check is
    generic (
    n            : integer range 0 to 110 :=64;        -- counter pool inputs:  hardware inputs plus test signals      
    WIDTH        : integer := 20     -- Counter width
       
);
port (
    clk_sys           : in std_logic;      -- Clock
    rstn_sys          : in std_logic;      -- Reset
    pos_threshold     : in std_logic_vector(31 downto 0);
    neg_threshold     : in std_logic_vector(31 downto 0);
    counter_maske_Reg : in std_logic_vector(31 downto 0);
    out_maske_Reg     : in std_logic_vector(31 downto 0);
    BLM_ena          : in std_logic_vector(31 downto 0);      
    Test_In_Mtx   : in t_Test_Data; 
    AW_IOBP_Input_Reg:  in  t_IO_Reg_1_to_7_Array;
    INTL_Output   : out std_logic_vector(5 downto 0);
    BLM_status_Reg : out std_logic_vector(15 downto 0)
    
);

end Beam_Loss_check;

architecture rtl of Beam_Loss_check is
    

signal    gate_error:         std_logic_vector(11 downto 0);
signal    Gate_In_Mtx:        std_logic_vector (11 downto 0):= (OTHERS => '0');  -- gate outputs from the gate timing sequence control


signal    count_enable:       std_logic_vector(7 downto 0); 
signal    UP_IN:              std_logic_vector(n-2 downto 0); 
signal    DOWN_IN:            std_logic_vector(n-2 downto 0); 
signal    UP_OVERFLOW:        t_counter_in_Array ; 
signal    DOWN_OVERFLOW:      t_counter_in_Array ; 
signal    test_signal:        std_logic_vector(9 downto 0);
signal    in_mux:             t_in_array;
signal    test_signal_sel:    std_logic_vector(2 downto 0); 
signal    Interlock_wd:       t_in_array;
signal    watchdog_warn:      std_logic_vector(53 downto 0);
signal    VALUE_IN:            std_logic_vector(63 downto 0);

component BLM_In_Multiplexer is

    port(
      clk_i : in std_logic;          -- chip-internal pulsed clk signal
      rstn_i : in std_logic;        -- reset signal
      AW_IOBP_Input_Reg:  in  t_IO_Reg_1_to_7_Array;
      watchdog_ena  : in std_logic_vector( 8 downto 0);
      In_Mtx        : out t_in_array;
      INTL_out      : out t_in_array
    );
    end component BLM_In_Multiplexer;

component BLM_gate_timing_seq is

    generic (
 
      hold    : integer range 2 TO 10:= 2;
      n       : integer range 0 TO 12 := 12
    );
    port(
      clk_i : in std_logic;          -- chip-internal pulsed clk signal
      rstn_i : in std_logic;        -- reset signal
      gate_in : in std_logic_vector(n-1 downto 0);        -- input signal
      initialize : in std_logic_vector(11 downto 0);     -- enable '1' for input connected to the counter
      timeout_error : out std_logic_vector(n-1 downto 0); -- gate doesn't start within the given timeout
      gate_out: out std_logic_vector(n-1 downto 0)        -- out gate signal
    );
    end component BLM_gate_timing_seq;

   
      component BLM_counter_pool is
    
        generic (      
            WIDTH        : integer := 20      -- Counter width
                
        );
        port (
            CLK         : in std_logic;      -- Clock
            nRST         : in std_logic;      -- Reset
            CLEAR       : in std_logic;      -- Clear counter register
            LOAD        : in std_logic_vector(7 downto 0);      -- Load counter register
            ENABLE      : in std_logic_vector(7 downto 0);      -- Enable count operation
            pos_threshold : in std_logic_vector(31 downto 0);
            neg_threshold : in std_logic_vector(31 downto 0);
            VALUE_IN       : in std_logic_vector(63 downto 0);    -- Load counter register input
            UP_OVERFLOW    : out t_counter_in_Array ;     -- UP_Counter overflow
            DOWN_OVERFLOW    : out t_counter_in_Array      -- UP_Counter overflow
        );

        end component BLM_counter_pool;

        component BLM_Interlock_out is

      
        
          port (
                  CLK              : in std_logic;      -- Clock
                  nRST             : in std_logic;      -- Reset
                  out_mux_sel      : in std_logic_vector(31 downto 0);
                  UP_OVERFLOW      : in t_counter_in_Array ; 
                  DOWN_OVERFLOW    : in t_counter_in_Array  ; 
                  gate_error       : in std_logic_vector(11 downto 0);
                  Interlock_IN     : in std_logic_vector(53 downto 0);

                  INTL_Output      : out std_logic_vector(5 downto 0);
                  BLM_status_Reg : out std_logic_vector(15 downto 0)
          );
          end component BLM_Interlock_out;

---######################################################################################

begin



  gate_board1: BLM_gate_timing_seq

    generic map (
      hold     => 2,
      n        => 12
    )
    port map(
      clk_i => clk_sys,         -- chip-internal pulsed clk signal
      rstn_i => rstn_sys,         -- reset signal
      gate_in => AW_IOBP_Input_Reg(6)(5 downto 0) & AW_IOBP_Input_Reg(5)(11 downto 6),       -- input signal
      initialize => BLM_ena(26 downto 15),  -- enable '1' for input connected to the counter
      timeout_error => gate_error, -- gate doesn't start within the given timeout
      gate_out => gate_In_Mtx(11 downto 0)       -- out gate signal
    );
   

  
 Input_multiplexer: BLM_In_Multiplexer 

    port map(
      clk_i                => clk_sys,
      rstn_i               => rstn_sys,
      AW_IOBP_Input_Reg    => AW_IOBP_Input_Reg,
      watchdog_ena         => BLM_ena(8 downto 0),
      In_Mtx               => in_mux,
      INTL_out             =>Interlock_wd
    );


---- counter pool ------------------------------------------------------------------------------

BLM_counter_pool_inputs: process (rstn_sys, clk_sys)  --54 Inputs + 1 Test signal pro time (I use Test_in_Mtx(5 downto 0)... the MSB have been defined as "00")
begin
           if not rstn_sys='1' then 
            count_enable <= (others =>'0');
 --  
              test_signal <= (others =>'0');
              
              watchdog_warn <= (others =>'0');
              
       elsif (clk_sys'EVENT AND clk_sys = '1') then

            
            case BLM_ena(14 downto 12) is 

                when "000" => test_signal <=  Test_In_Mtx(0);
                when "001" => test_signal <=  Test_In_Mtx(1);
                when "010" => test_signal <=  Test_In_Mtx(2);
                when "011" => test_signal <=  Test_In_Mtx(3);
                when "100" => test_signal <=  Test_In_Mtx(4);
                when "101" => test_signal <=  Test_In_Mtx(5);
                when "110" => test_signal <=  Test_In_Mtx(6);
                when "111" => test_signal <=  Test_In_Mtx(7);  
                when others => null;

            end case;

            watchdog_warn <= Interlock_wd(8) & Interlock_wd(7) & Interlock_wd(6) & Interlock_wd(5) & Interlock_wd(4) & Interlock_wd(3) & Interlock_wd(2) & Interlock_wd(1) & Interlock_wd(0);
           
                if (watchdog_warn = ("000000000000000000000000000000000000000000000000000000")) or (gate_error = "000000000000") then 
                count_enable <=counter_maske_Reg(7 downto 0);
                 
                else
                count_enable <="00000000";
                end if;

            
        end if;
    end process;
            VALUE_IN   <= test_signal  & in_mux(8) & in_mux(7) & in_mux(6) & in_mux(5) & in_mux(4) & in_mux(3) & in_mux(2) & in_mux(1) & in_mux(0);

    




    BLM_Counter_pool_elem: BLM_counter_pool
      generic map (

          WIDTH     =>  20      -- Counter width
      
      )
      port map(
          CLK         => clk_sys,      -- Clock
          nRST        => rstn_sys,      -- Reset
          CLEAR       => counter_maske_Reg(8),       -- Clear counter register 
          LOAD        => counter_maske_Reg(16 downto 9),      -- Load counter register
          ENABLE      => count_enable,     -- Enable count operation
          pos_threshold =>  pos_threshold,
          neg_threshold =>  neg_threshold,  
          VALUE_IN       => VALUE_IN,    -- Load counter register input
          UP_OVERFLOW    => UP_OVERFLOW,    -- UP_Counter overflow
          DOWN_OVERFLOW  => DOWN_OVERFLOW    -- UP_Counter overflow
      );


Interlock_output: BLM_Interlock_out 

  port map(
          CLK          => clk_sys,
          nRST         => rstn_sys,
          out_mux_sel  => out_maske_reg,
          UP_OVERFLOW    => UP_OVERFLOW,    
          DOWN_OVERFLOW  => DOWN_OVERFLOW,   
          gate_error     => gate_error,
          Interlock_IN   => watchdog_warn,
          INTL_Output    => INTL_Output,
          BLM_status_Reg => BLM_status_Reg
  );

  
  end architecture;
