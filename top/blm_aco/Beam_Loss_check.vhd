LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use IEEE.std_logic_misc.all;
use work.scu_diob_pkg.all;

entity Beam_Loss_check is
    generic (
    
    WIDTH        : integer := 30     -- Counter width
       
);
port (
    clk_sys           : in std_logic;      -- Clock
    rstn_sys          : in std_logic;      -- Reset
   -- IN BLM 
    BLM_data_in       : in std_logic_vector(53 downto 0);
    BLM_gate_in       : in std_logic_vector(11 downto 0);
    BLM_tst_ck_sig    : in std_logic_vector (10 downto 0);
    IOBP_LED_nr       : in std_logic_vector(3 downto 0);
    --IN registers
    pos_threshold           : in t_BLM_th_Array; 
    neg_threshold           : in t_BLM_th_Array ;
    BLM_wdog_hold_time_Reg  : in std_logic_vector(15 downto 0);
    BLM_wd_reset            : in std_logic_vector(53 downto 0);
    BLM_gate_hold_time_Reg  : in  t_BLM_gate_hold_Time_Array;
    BLM_ctrl_Reg            : in std_logic_vector(15 downto 0);  
    BLM_gate_sync_rcv : in std_logic_vector(15 downto 0);                                                        
    BLM_counters_Reg: in std_logic_vector(15 downto 0); 
    BLM_gate_recover_Reg : in std_logic_vector(15 downto 0); 
    BLM_gate_prep_Reg: in std_logic_vector(15 downto 0);     
    BLM_in_sel_Reg          : in t_BLM_reg_Array; 
    BLM_out_sel_reg : in t_BLM_out_sel_reg_Array;   

    ev_prepare_reg : in std_logic_vector(11 downto 0);
    ev_recover_reg: in std_logic_vector(11 downto 0);
    ev_counter_reset: in std_logic;

 -- OUT register
   BLM_status_Reg : out t_IO_Reg_0_to_29_Array ;
   counter_readout_reg: out t_BLM_th_Array  ;
  -- OUT BLM
   BLM_Out           : out std_logic_vector(11 downto 0) 
);

end Beam_Loss_check;

architecture rtl of Beam_Loss_check is
  
signal BLM_test_signal   : std_logic_vector(9 downto 0);  
signal VALUE_IN: std_logic_vector(63 downto 0);
signal out_1wd: std_logic_vector(53 downto 0);
signal INT_out: std_logic_vector(53 downto 0);
signal gate_error: std_logic_vector(11 downto 0);
signal Gate_In_Mtx: std_logic_vector (11 downto 0):= (OTHERS => '0');  
signal UP_OVERFLOW: std_logic_vector(127 downto 0);     
signal DOWN_OVERFLOW: std_logic_vector(127 downto 0);  
signal gate_sel: integer range 0 to 127;
signal cnt_enable: std_logic_vector(127 downto 0);
signal BLM_Output_mtx: t_BLM_out_Array;
signal gate_output: std_logic_vector (11 downto 0);
signal wd_reset: std_logic_vector(53 downto 0);
signal BLM_gate_recover: std_logic_vector(11 downto 0);
signal BLM_gate_seq_clk_sel: std_logic_vector(2 downto 0);
signal BLM_gate_prepare : std_logic_vector(11 downto 0);
signal counter_value: t_BLM_counter_Array;
signal gate_test_value: std_logic_vector(11 downto 0);
signal gate_sm_error: std_logic_vector(11 downto 0);
signal gate_sm_output: std_logic_vector(11 downto 0);
signal gate_recover: std_logic_vector(11 downto 0);
signal gate_prepare: std_logic_vector(11 downto 0);
signal gate_hold_time: t_BLM_gate_hold_Time_Array;
signal gate_state: std_logic_vector(47 downto 0);
signal gate_sm_state :t_gate_state_nr;
signal UP_OVERFLOW_OUT: std_logic_vector(127 downto 0):=(others => '0');
signal DOWN_OVERFLOW_OUT: std_logic_vector(127 downto 0):=(others => '0');
signal direct_gate: std_logic_vector(11 downto 0);
signal LED_ID_state:  std_logic_vector(3 downto 0);
signal pos_th, neg_th: t_BLM_th_Array;
signal ev_pos_neg_thr: std_logic_vector(63 downto 0):=(others => '0');
signal cnt_nr : integer range 0 to 127;
signal AUTO_RESET: std_logic; 
signal RESET: std_logic;


  component BLM_watchdog is
  
    port(
        clk_i : in std_logic;    
        rst_i : in std_logic;  
        wd_reset: in std_logic; 
        hold: in std_logic_vector(15 downto 0);
        in_watchdog : in std_logic;    
        INTL_out: out std_logic       
    );
end component BLM_watchdog;

component BLM_gate_timing_seq is

    generic (
      n       : integer range 0 TO 12 := 12
    );
    port(
      clk_i : in std_logic;
      rstn_i : in std_logic;      
      gate_in : in std_logic_vector(n-1 downto 0);      
      direct_gate : in std_logic_vector(n-1 downto 0);
      BLM_gate_recover: in std_logic_vector(11 downto 0); 
      gate_sync_rcv : in std_logic_vector(11 downto 0);
      BLM_gate_prepare : in std_logic_vector(11 downto 0); 
      hold_time : in  t_BLM_gate_hold_Time_Array;
      gate_error : out std_logic_vector(n-1 downto 0); 
      state_nr: out t_gate_state_nr;
      gate_out: out std_logic_vector(n-1 downto 0)      
    );
    end component BLM_gate_timing_seq;

  component  BLM_ena_in_mux is
      port (
          CLK               : in std_logic;      
          nRST              : in std_logic;      
          mux_sel           : in t_BLM_reg_Array;
          in_mux            : in std_logic_vector(11 downto 0);
          cnt_enable        : out std_logic_vector(127 downto 0)
      );
  end component BLM_ena_in_mux;


 component BLM_counter_pool_el is

    generic (      
        WIDTH        : integer := 32      -- Counter width
            
    );
    port (
        CLK               : in std_logic;     
        nRST              : in std_logic;      
        gate_reset_ena    : in std_logic;
        RESET      : in std_logic;        -- global counter reset
        ENABLE            : in std_logic; -- Enable count operation (gate signals)
        pos_threshold     : in std_logic_vector(31 downto 0);
        neg_threshold     : in std_logic_vector(31 downto 0);
        in_counter        : in std_logic_vector(63 downto 0);
        BLM_cnt_Reg     : in std_logic_vector(15 downto 0); 
        cnt    : out std_logic_vector (WIDTH-1 downto 0);    
        UP_OVERFLOW       : out std_logic;    
        DOWN_OVERFLOW     : out std_logic   
    );
  end component BLM_counter_pool_el;
  

  component  BLM_out_el is
    
      port (
        CLK              : in std_logic;      
        nRST             : in std_logic;      
        BLM_out_sel_reg : in t_BLM_out_sel_reg_Array;   
        UP_OVERFLOW      : in std_logic_vector(127 downto 0);
        DOWN_OVERFLOW    : in std_logic_vector(127 downto 0);   
        wd_out           : in std_logic_vector(53 downto 0); 
        gate_in          : in std_logic_vector(11 downto 0); 
        gate_error        : in std_logic_vector (11 downto 0); 
        gate_out        : in std_logic_vector (11 downto 0);    
        gate_state: in std_logic_vector(47 downto 0);
        led_id_state : in std_logic_vector(3 downto 0);
        BLM_Output      : out std_logic_vector(11 downto 0);
      BLM_status_Reg : out t_IO_Reg_0_to_29_Array   
        );
     
    end component BLM_out_el;


---######################################################################################

begin


BLM_test_signal <=  BLM_tst_ck_sig(9) & -- 25 MHz
                    BLM_tst_ck_sig(8)  & -- 24.9 MHz
                    BLM_tst_ck_sig(7)  & -- 10 MHz
                    BLM_tst_ck_sig(2)  & -- 9.9 MHz
                    BLM_tst_ck_sig(6)  & -- 1 MHz
                    BLM_tst_ck_sig(1)  & -- 0.99 MHz
                    BLM_tst_ck_sig(5)  & -- 100 kHz
                    BLM_tst_ck_sig(0)  & -- 99 kHz
                    BLM_tst_ck_sig(4)  & -- 10 kHz
                    '0';                 -- GND


Values_in_proc: process (clk_sys, rstn_sys)
    begin
      if not rstn_sys='1' then 
        VALUE_IN <= (others =>'0');
        AUTO_RESET <= '0';
        BLM_gate_recover <= (others =>'0');
        BLM_gate_prepare <= (others =>'0');
        direct_gate <= (others =>'0');                    
                               
      elsif (clk_sys'EVENT AND clk_sys= '1') then 
        VALUE_IN <= BLM_test_signal & BLM_data_in;
        RESET <= BLM_counters_Reg(0);
        AUTO_RESET <= BLM_counters_Reg(1);
        BLM_gate_recover <= ev_recover_reg or BLM_gate_recover_Reg(11 downto 0);
        BLM_gate_prepare <= ev_prepare_reg or BLM_gate_prep_Reg(11 downto 0); 
        direct_gate <= BLM_ctrl_Reg(5 downto 0) & BLM_ctrl_Reg(11 downto 6);
      end if;
    end process;
                 
-- for direct Gate operations: if the corresponding BLM_ctrl_Reg (bit +2)='0' then BLM_gate_in signals 
-- are used as input signal to the multiplexer  (BLM_ena_in_mux) which gives the counter enables.

LED_ID_state <= IOBP_LED_nr;

direct_gate_operation: process(BLM_ctrl_Reg, BLM_gate_in, gate_output)

begin

  for i in 0 to 5 loop
    
    if BLM_ctrl_Reg(i) = '1' then   
      gate_In_Mtx(i)<= BLM_gate_in(i+6);
    else 
      gate_IN_Mtx(i) <= gate_output(i+6);
    end if;
  end loop;
  for i in 6 to 11 loop
    
    if BLM_ctrl_Reg(i) = '1' then 
      gate_In_Mtx(i)<= BLM_gate_in(i-6);
    else 
      gate_IN_Mtx(i) <= gate_output(i-6);
    end if;
  end loop;
end process direct_gate_operation;


  gate_board: BLM_gate_timing_seq

    generic map (
      n        => 12
    )
    port map(
      clk_i => clk_sys,        
      rstn_i => rstn_sys,         
      gate_in => BLM_gate_in,      
      direct_gate => direct_gate,
      BLM_gate_recover => BLM_gate_recover(5 downto 0)&BLM_gate_recover(11 downto 6),
      BLM_gate_prepare => BLM_gate_prepare(5 downto 0)&BLM_gate_prepare(11 downto 6),

      gate_sync_rcv  => BLM_gate_sync_rcv(5 downto 0)& BLM_gate_sync_rcv(11 downto 6),
      hold_time => gate_hold_time,

      gate_error => gate_sm_error, 
      state_nr => gate_sm_state,
      gate_out => gate_sm_output 
    );

  gate_error <= gate_sm_error;
  gate_output <= gate_sm_output;

  gate_state <= '0'& gate_sm_state(5) & '0'& gate_sm_state(4)& '0'& gate_sm_state(3)&'0'& gate_sm_state(2) & '0'& gate_sm_state(1)&'0'& gate_sm_state(0)&
  '0'& gate_sm_state(11) & '0'& gate_sm_state(10)& '0'& gate_sm_state(9)&'0'& gate_sm_state(8) & '0'& gate_sm_state(7)&'0'& gate_sm_state(6);


  gate_hold_time_proc: process(BLM_gate_hold_time_Reg)

    begin
      for i in 0 to 5 loop
        gate_hold_time(i) <= BLM_gate_hold_time_Reg(i+6);
      end loop;
      for i in 6 to 11 loop
        gate_hold_time(i) <= BLM_gate_hold_time_Reg(i-6);
      end loop;

    end process gate_hold_time_proc;

       wd_elem_gen: for i in 0 to 47 generate

        input_Watchdog: BLM_watchdog
         
          port map(
            clk_i => clk_sys,  
            rst_i => rstn_sys,  
            wd_reset=>  BLM_wd_reset(i), 
            hold => BLM_wdog_hold_time_Reg,
            in_watchdog => BLM_data_in(i),
            INTL_out =>   out_1wd(i));
     
      end generate wd_elem_gen;

---- counter ena mux ------------------------------------------------------------------------------
BLM_counter_ena_block:  BLM_ena_in_mux 
  port  map(
    CLK            => clk_sys,  
    nRST           => rstn_sys,
    mux_sel        => BLM_in_sel_Reg,
    in_mux         => gate_In_Mtx,
    cnt_enable     => cnt_enable
  );

  ---- counter pool ------------------------------------------------------------------------------

BLM_counter_pool: for i in 0 to 127 generate

BLM_counter_pool_elem: BLM_counter_pool_el
generic map (      
      WIDTH  => 32)  -- Counter width
port map (
  CLK            => clk_sys,  
  nRST           => rstn_sys,
  gate_reset_ena => AUTO_RESET and cnt_enable(i), 
  RESET          => RESET or ev_counter_reset, 
  ENABLE         => cnt_enable(i),
  pos_threshold  => pos_threshold(i),
  neg_threshold  => neg_threshold(i),
  in_counter     => VALUE_IN,
  
  BLM_cnt_Reg    => BLM_in_sel_Reg(i),
  cnt   => counter_value(i),

  UP_OVERFLOW    => UP_OVERFLOW(i),
  DOWN_OVERFLOW  => DOWN_OVERFLOW(i)
  );
      end generate BLM_counter_pool;
-----------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------


BLM_out_section: BLM_out_el 
     
  port map(
    CLK          => clk_sys,
    nRST           => rstn_sys,  
 
    BLM_out_sel_reg => BLM_out_sel_reg, 
  
  UP_OVERFLOW  =>UP_OVERFLOW,
   DOWN_OVERFLOW   => DOWN_OVERFLOW,
    wd_out           => out_1wd, 
    gate_in         => BLM_gate_in(5 downto 0) & BLM_gate_in(11 downto 6),
    gate_error        => gate_error(5 downto 0) & gate_error(11 downto 6),
    gate_out        => gate_output(5 downto 0) & gate_output(11 downto 6),
    gate_state      => gate_state,
    led_id_state    => LED_ID_state,
    BLM_Output      => BLM_out, 
    BLM_status_Reg  => BLM_status_Reg 
    );

    counters_readout_proc: process (counter_value)
    begin
      for i in 0 to 127 loop
        counter_readout_reg(i) <=  counter_value(i);
      end loop;
      end process counters_readout_proc;

     
  end architecture;

