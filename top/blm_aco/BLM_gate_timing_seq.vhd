library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity BLM_gate_timing_seq is

generic (
  
  n       : integer range 0 TO 12 :=12

);

port(

  clk_i : in std_logic;          -- chip-internal pulsed clk signal
  rstn_i : in std_logic;        -- reset signal
  gate_in : in std_logic_vector(n-1 downto 0);        -- input signal
  gate_seq_ena : in std_logic_vector(n-1 downto 0);     -- enable '1' for input connected to the counter
  hold_time : in std_logic_vector(7 downto 0);
  timeout_error : out std_logic_vector(n-1 downto 0);  -- gate doesn't start within the given timeout
  gate_out: out std_logic_vector(n-1 downto 0)        -- out gate signal

);
end BLM_gate_timing_seq;

architecture rtl of BLM_gate_timing_seq is

  signal    timeout_er:         std_logic_vector(n-1 downto 0):= (others =>'0');
  signal    Gate_In_Mtx:        std_logic_vector (n-1 downto 0):= (others =>'0');
  

component BLM_gate_timing_seq_elem is

  
  port(
    clk_i : in std_logic;          -- chip-internal pulsed clk signal
    rstn_i : in std_logic;        -- reset signal
    gate_in : in std_logic;        -- input signal
    gate_in_ena : in std_logic;     -- enable '1' for input connected to the counter
    hold: in std_logic_vector(7 downto 0);
    timeout_error : out std_logic;  -- gate doesn't start within the given timeout
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
              gate_in_ena => gate_seq_ena(i),    -- enable '1' for input connected to the counter
              hold => hold_time,
              timeout_error => timeout_er(i), -- gate doesn't start within the given timeout
              gate_out => Gate_In_Mtx(i)    -- out gate signal
            );
           end generate BLM_gate_timing;
    
        
        timeout_error <= timeout_er;
        gate_out <= Gate_In_Mtx;

 end rtl;          		 
	
