library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use work.scu_diob_pkg.all;

entity BLM_gate_timing_seq is

generic (
  
  n       : integer range 0 TO 12 :=12

);

port(

  clk_i : in std_logic;         
  rstn_i : in std_logic;        -- reset signal
  gate_in : in std_logic_vector(n-1 downto 0);        -- input signal
  direct_gate : in std_logic_vector(n-1 downto 0);
 -- gate_seq_ena : in std_logic_vector(n-1 downto 0);     -- enable '1' for input connected to the counter
  BLM_gate_recover: in std_logic_vector(11 downto 0); 
  BLM_gate_prepare : in std_logic_vector(11 downto 0); 
  hold_time : in  t_BLM_gate_hold_Time_Array;
  gate_sync_rcv : in std_logic_vector(11 downto 0);
  gate_error : out std_logic_vector(n-1 downto 0);  -- gate doesn't start within the given timeout
  state_nr: out t_gate_state_nr;
  gate_out: out std_logic_vector(n-1 downto 0)        -- out gate signal

);
end BLM_gate_timing_seq;

architecture rtl of BLM_gate_timing_seq is

  signal    gate_er:         std_logic_vector(n-1 downto 0):= (others =>'0');
  signal    Gate_In_Mtx:        std_logic_vector (n-1 downto 0):= (others =>'0');
  signal    gate_state: t_gate_state_nr;

component BLM_gate_timing_seq_elem is

  
  port(
    clk_i : in std_logic;          --
    rstn_i : in std_logic;        -- reset signal
    gate_in : in std_logic;        -- input signal
    direct_gate : in std_logic;
    prepare : in std_logic;
    recover : in std_logic;
    sync_rcv : in std_logic;
    hold: in std_logic_vector(15 downto 0);
--
    gate_error : out std_logic;  -- gate doesn't start within the given timeout
   gate_state_nr : out std_logic_vector (2 downto 0); --for tests
    gate_out: out std_logic      -- out gate signal
  );
  end component BLM_gate_timing_seq_elem;


begin

  

         BLM_gate_timing: for i in 0 to (n-1) generate

         begin
    
          gate_elem: BLM_gate_timing_seq_elem 

       
            port map(
              clk_i=> clk_i,
              rstn_i => rstn_i,
              gate_in => gate_in(i),
              direct_gate => direct_gate(i),
              prepare => BLM_gate_prepare(i),
              recover => BLM_gate_recover(i),
              sync_rcv =>gate_sync_rcv(i),
              hold => hold_time(i),

              gate_error => gate_er(i), 
              gate_state_nr => gate_state(i),
              gate_out => Gate_In_Mtx(i)    -- out gate signal
            );
           end generate BLM_gate_timing;
    
        
        gate_error <= gate_er;
        gate_out <= Gate_In_Mtx;
         state_nr <= gate_state;
         
 end rtl;          		 
	
