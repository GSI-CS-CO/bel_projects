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

   -- IN BLM 
    BLM_data_in       : in std_logic_vector(53 downto 0);
    BLM_gate_in       : in std_logic_vector(11 downto 0);
    BLM_tst_ck_sig    : in std_logic_vector (13 downto 0); 
   
    --IN registers
    pos_threshold           : in t_BLM_th_Array; --t_BLM_th_Array is array (0 to 255) of std_logic_vector(31 downto 0);
    neg_threshold           : in t_BLM_th_Array ;
    BLM_wdog_hold_time_Reg  : in std_logic_vector(15 downto 0);
    BLM_gate_hold_time_Reg  : in std_logic_vector(15 downto 0);
    BLM_ctrl_Reg            : in std_logic_vector(15 downto 0);
    BLM_gate_seq_ck_sel_Reg : in t_IO_Reg_0_to_2_Array;
    BLM_gate_seq_in_ena_Reg : in std_logic_vector(15 downto 0); --"00"& ena for gate board1 &"00" & ena for gate board2
    BLM_in_ena_Reg          : in t_BLM_reg_Array; --256 x (4 bit for gate ena & 6 bit for up signal ena & 6 for down signal ena)
    BLM_out_ena_Reg :  in t_BLM_out_reg_Array;             -- 192 16 bits register for the output selection of the 256 counters and comparators results 

    BLM_out_mux_Reg  : in t_BLM_mux_reg_Array;    
    -- OUT register
    BLM_status_Reg    : out t_IO_Reg_0_to_7_Array;
      -- OUT BLM
      BLM_Out           : out std_logic_vector(5 downto 0) 
);

end Beam_Loss_check;

architecture rtl of Beam_Loss_check is
  --
  --TYPE    t_BLM_out_Array           is array (0 to 255) of std_logic_vector(5 downto 0);
  --
  signal BLM_test_signal   : std_logic_vector(9 downto 0);  --Test signals + ground  --to reference  Test_In_Mtx
  signal BLM_gate_seq_clk  : std_logic_vector(7 downto 0);  --
  TYPE gate_clock_type is array  (0 to 2) of std_logic_vector(3 downto 0);
  signal gate_clock : gate_clock_type;
  signal g_clock: std_logic_vector(11 downto 0); 
  signal VALUE_IN:  std_logic_vector(63 downto 0);
 
  signal out_wd: std_logic_vector(53 downto 0);

  signal INT_out: std_logic_vector(53 downto 0);
  signal gate_error: std_logic_vector(11 downto 0);
  signal Gate_In_Mtx: std_logic_vector (11 downto 0):= (OTHERS => '0');  -- gate outputs from the gate timing sequence control
  signal UP_OVERFLOW: std_logic_vector(255 downto 0);     
  signal DOWN_OVERFLOW: std_logic_vector(255 downto 0);  
  signal gate_sel: integer range 0 to 255;
  signal cnt_enable: std_logic_vector(255 downto 0);
  signal BLM_Output_mtx: t_BLM_out_Array;
  
  component BLM_watchdog is
  
    port(
        clk_i : in std_logic;     -- chip-internal pulsed clk signal
        rstn_i : in std_logic;   -- reset signal
        hold: in std_logic_vector(15 downto 0);
        in_watchdog : in std_logic;     -- input signal
        ena_i : in std_logic;     -- enable '1' for input connected to the counter
        INTL_out: out std_logic   -- interlock output for signal that doesn't change for a given time 
    
    );
end component BLM_watchdog;

component BLM_gate_timing_seq is

    generic (
 

      n       : integer range 0 TO 12 := 12
    );
    port(
      clk_i : in std_logic_vector(n-1 downto 0);
      rstn_i : in std_logic;        -- reset signal
      gate_in : in std_logic_vector(n-1 downto 0);        -- input signal
      gate_seq_ena : in std_logic_vector(11 downto 0);     -- enable '1' for input connected to the counter
      hold_time : in std_logic_vector(15 downto 0);
      timeout_error : out std_logic_vector(n-1 downto 0); -- gate doesn't start within the given timeout
      gate_out: out std_logic_vector(n-1 downto 0)        -- out gate signal
    );
    end component BLM_gate_timing_seq;

  component  BLM_ena_in_mux is
      port (
          CLK               : in std_logic;      -- Clock
          nRST              : in std_logic;      -- Reset
          mux_sel           : in t_BLM_reg_Array;
          in_mux            : in std_logic_vector(11 downto 0);
          cnt_enable        : out std_logic_vector(255 downto 0)
      );
  end component BLM_ena_in_mux;


 component BLM_counter_pool_el is

    generic (      
        WIDTH        : integer := 20      -- Counter width
            
    );
    port (
        CLK               : in std_logic;      -- Clock
        nRST              : in std_logic;      -- Reset
        RESET             : in std_logic;      -- global counter reset
        LOAD              : in std_logic;
        ENABLE            : in std_logic;      -- Enable count operation (gate signals)
        pos_threshold     : in std_logic_vector(31 downto 0);
        neg_threshold     : in std_logic_vector(31 downto 0);
        in_counter        : in std_logic_vector(63 downto 0);
        BLM_cnt_Reg     : in std_logic_vector(15 downto 0);  -- bit 5-0 = up_in_counter select, bit 11-6 = down_in_counter select, 15..13 in_ena
        UP_OVERFLOW       : out std_logic;     -- UP_Counter overflow for the input signals
        DOWN_OVERFLOW     : out std_logic    -- DOWN_Counter overflow for the input signals

    );
  end component BLM_counter_pool_el;
  

  component  BLM_out_el is
     
    port (
      CLK              : in std_logic;      -- Clock
      nRST             : in std_logic;      -- Reset
--
      -- Bit 15 downto 12 selection, for each output to the or, between up_output(i) and down_output(i)
      BLM_out_ena_Reg :  in t_BLM_out_reg_Array;           -- 192 16 bits register for the output selection of the 256 counters and comparators results 
      UP_OVERFLOW      : in std_logic_vector(255 downto 0);
      DOWN_OVERFLOW    : in std_logic_vector(255 downto 0);
      BLM_out_mux_Reg  : in t_BLM_mux_reg_Array;  -- 6 16 bits registers for the selection of gate errors, watchdog errors and inputs to the last OR computation
      --  For each register: bit 15 free, bit 14-10: 5 bit for the last or, bit 9-4: 6 bits for the watchdog errors, bit 3-0: 4 bits for gate errors
      wd_out           : in std_logic_vector(53 downto 0);
      gate_in         : in std_logic_vector(11 downto 0); 
      gate_out        : in std_logic_vector (11 downto 0);
 
      BLM_Output      : out std_logic_vector(5 downto 0);
      BLM_status_Reg : out t_IO_Reg_0_to_7_Array
      );
    end component BLM_out_el;

   
---######################################################################################

begin

VALUE_IN <= BLM_test_signal & BLM_data_in;


  gate_timing_clock_sel_proc: process( BLM_gate_seq_ck_sel_Reg)
    begin
    
     
     for j in 0 to 2 loop

      for i in 0 to 3 loop

        case BLM_gate_seq_ck_sel_Reg(J)((3*i+2) downto 3*i) is

          when "000" 
           =>  gate_clock(j)(i) <= BLM_tst_ck_sig(0);  --clk_1_25MHz
          when "001" 
           =>  gate_clock(j)(i) <= BLM_tst_ck_sig(1);  --clk_2MHz
          when "010" 
           =>  gate_clock(j)(i) <= BLM_tst_ck_sig(2);  --clk_4MHz
          when "011" 
           =>  gate_clock(j)(i) <= BLM_tst_ck_sig(4);  --clk_8MHz
          when "100" 
           =>  gate_clock(j)(i) <= BLM_tst_ck_sig(6);  --clk_12_5MHz
          when "101" 
           =>  gate_clock(j)(i) <= BLM_tst_ck_sig(8);  --clk_25MHz
          when "110" 
           =>  gate_clock(j)(i) <= BLM_tst_ck_sig(10); --clk_50MHz
          when "111" 
           =>  gate_clock(j)(i) <= BLM_tst_ck_sig(11); --clk_100MHz
          when others 
           =>  gate_clock(j)(i) <= Null;
        end case;
      end loop;
    end loop;

    end process;

g_clock <= gate_clock(2) & gate_clock(1)& gate_clock(0);

BLM_test_signal <=  BLM_tst_ck_sig(10) & -- clk_25MHz
                    BLM_tst_ck_sig(9)  & -- clk_20MHz
                    BLM_tst_ck_sig(8)  & -- clk_16MHz
                    BLM_tst_ck_sig(7)  & -- clk_12_5MHz
                    BLM_tst_ck_sig(6)  & -- clk_10MHz
                    BLM_tst_ck_sig(5)  & -- clk_8MHz
                    BLM_tst_ck_sig(4)  & -- clk6_25MHz
                    BLM_tst_ck_sig(1)  & -- clk_2MHz
                    BLM_tst_ck_sig(0)  & -- clk_1_25MHz
                    '0';                 -- GND
                    
                    
  gate_board: BLM_gate_timing_seq

    generic map (
      n        => 12
    )
    port map(
      clk_i => g_clock,         -- chip-internal pulsed clk signal
      rstn_i => rstn_sys,         -- reset signal
      gate_in => BLM_gate_in,       -- gate input signals
      gate_seq_ena => BLM_gate_seq_in_ena_Reg(13 downto 8) & BLM_gate_seq_in_ena_Reg(5 downto 0), --"00"& ena for gate board1 &"00" & ena for gate board2
      hold_time => BLM_gate_hold_time_Reg,
      timeout_error => gate_error, -- gate error
      gate_out => gate_In_Mtx       -- out gate signal
    );
   

       wd_elem_gen: for i in 0 to 53 generate

        input_Watchdog: BLM_watchdog
         
          port map(
            clk_i => clk_sys,  
            rstn_i => rstn_sys,   -- reset signal
            hold => BLM_wdog_hold_time_Reg,
            in_watchdog => BLM_data_in(i),
            ena_i =>    '1',  -- enable for input connected to the counter
            INTL_out =>   out_wd(i));
      end generate wd_elem_gen;




---- counter ena mux ------------------------------------------------------------------------------
BLM_counter_ena_block:  BLM_ena_in_mux 
  port  map(
    CLK            => clk_sys,  
    nRST           => rstn_sys,
    mux_sel        => BLM_in_ena_Reg,
    in_mux         => gate_In_Mtx,
    cnt_enable     => cnt_enable
  );


  ---- counter pool ------------------------------------------------------------------------------

BLM_counter_pool: for i in 0 to 255 generate

BLM_counter_pool_elem: BLM_counter_pool_el
generic map (      
      WIDTH  => 20)
port map (
  CLK         => clk_sys,  
  nRST         => rstn_sys,
  RESET          => BLM_ctrl_Reg(0),
  LOAD           => BLM_ctrl_Reg(1),
  ENABLE         => cnt_enable(i),
  pos_threshold  => pos_threshold(i),
  neg_threshold  => neg_threshold(i),
  in_counter     => VALUE_IN,
  BLM_cnt_Reg  => BLM_in_ena_Reg(i),
  UP_OVERFLOW    => UP_OVERFLOW(i),
  DOWN_OVERFLOW  => DOWN_OVERFLOW(i)
  );
      end generate BLM_counter_pool;
-----------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------
-- out section

BLM_out_section: BLM_out_el 
     
  port map(
    CLK          => clk_sys,
    nRST           => rstn_sys,   -- Reset
  
    BLM_out_ena_Reg => BLM_out_ena_Reg,          -- 256 16 bits register for the output selection of the 256 counters and comparators results 
    UP_OVERFLOW     => UP_OVERFLOW,
    DOWN_OVERFLOW   => DOWN_OVERFLOW,
    BLM_out_mux_Reg => BLM_out_mux_Reg,  -- 6 16 bits registers for the selection of gate errors, watchdog errors and inputs to the last OR computation
    --  For each register: bit 15 free, bit 14-10: 5 bit for the last or, bit 9-4: 6 bits for the watchdog errors, bit 3-0: 4 bits for gate errors
    wd_out           => out_wd,
    gate_in         => BLM_gate_in,
    gate_out        => gate_error,
    BLM_Output      => BLM_out,
    BLM_status_Reg  => BLM_status_Reg
    );

  end architecture;

