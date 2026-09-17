LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;
use IEEE.std_logic_misc.all;

 
entity BLM_out_el is
    
port (
  CLK              : in std_logic;      -- Clock
  nRST             : in std_logic;      -- Reset
  BLM_out_sel_reg : in t_BLM_out_sel_reg_Array; 
  UP_OVERFLOW      : in std_logic_vector(127 downto 0);
  DOWN_OVERFLOW    : in std_logic_vector(127 downto 0);
  wd_out           : in std_logic_vector(53 downto 0);  -- wd_error
  gate_in          : in std_logic_vector(11 downto 0); -- to be sent to the status registers
  gate_error       : in std_logic_vector(11 downto 0); -- to be sent to the status registers
  gate_out        : in std_logic_vector (11 downto 0); --gate error
  gate_state: in std_logic_vector(47 downto 0);
  led_id_state : in std_logic_vector(3 downto 0);
  BLM_Output      : out std_logic_vector(11 downto 0);
  BLM_status_Reg : out t_IO_Reg_0_to_29_Array 
  );

end BLM_out_el;

architecture rtl of BLM_out_el is

    signal sel_tot: std_logic_vector(4031 downto 0);
    type t_sel is array (0 to 15) of std_logic_vector(327 downto 0);
    signal sel, product: t_sel;
    signal BLM_out_signal: std_logic_vector(15 downto 0):=(others =>'0');
    signal OVERFLOW : std_logic_vector(327 downto 0); 
    signal gate_output : std_logic_vector(11 downto 0);
    signal gate_error_output: std_logic_vector(11 downto 0);
    signal wd_output: std_logic_vector(53 downto 0);
    signal gate_input : std_logic_vector(11 downto 0);

    begin

        OVERFLOW <=(not (gate_in))& gate_error & wd_out(47 downto 0)  & UP_OVERFLOW & DOWN_OVERFLOW;
        gate_output <= gate_out;
        gate_error_output <=gate_error;
        wd_output <= wd_out;
        gate_input <= gate_in; 

    sel_signal_proc: process (clk, nRST)   
    begin
        if not nRST='1' then 
            for j in 0 to 15 loop           
                BLM_out_signal(j) <= '1';
                product(j) <= (others =>'0');
            end loop;
    

        elsif (clk'EVENT AND clk= '1') then 
              for i in 0 to 327 loop   
                for j in 0 to 15 loop
                  
          
                   product(j)(i) <= BLM_out_sel_Reg(i)(j) and OVERFLOW(i);   
                   sel(j)(i) <= BLM_out_sel_Reg(i)(j);   
                  
                     BLM_out_signal(j)<=or_reduce(product(j)) or nor_reduce(sel(j));
            

                end loop;
              end loop;              
        end if;   
    end  process;

--------------------------------------------------------------------------------------------------
-----                         BLM_STATUS_REGISTERS               
--------------------------------------------------------------------------------------------------
    status_reg_overflow_proc: process (OVERFLOW)
        begin
            for i in 0 to 15 loop
                BLM_status_reg(i) <=  OVERFLOW((i*16+15) downto i*16);
            end loop;
        end process;

    BLM_status_reg(16) <= "0000"& gate_error_output;
    BLM_status_reg(17) <= wd_output(15 downto 0);
    BLM_status_reg(18) <= wd_output(31 downto 16);
    BLM_status_reg(19) <= wd_output(47 downto 32);
    BLM_status_reg(20) <= (others =>'0');--reserved "0000000000" & wd_output(53 downto 48);
    BLM_status_reg(21) <=(others =>'0');
    BLM_status_reg(22) <=(others =>'0');
    BLM_status_reg(23) <= "0000" & gate_input;
    BLM_status_reg(24) <= "0000" & gate_output;
    BLM_status_reg(25) <= not BLM_out_signal; --bit 12-15 are virtual outputs
    BLM_status_reg(26) <= gate_state(15 downto 0); -- '0'& gate_sm_state(3)& '0'& gate_sm_state(2) & '0'& gate_sm_state(1)&'0'& gate_sm_state(0);
    BLM_status_reg(27) <= gate_state(31 downto 16); -- '0'& gate_sm_state(7)&'0'& gate_sm_state(6)& '0'& gate_sm_state(5) & '0'& gate_sm_state(4)
    BLM_status_reg(28) <=  gate_state(47 downto 32); --'0'& gate_sm_state(11) & '0'& gate_sm_state(10)& '0'& gate_sm_state(9)&'0'& gate_sm_state(8)
    BLM_status_reg(29)(15 downto 4) <= (others => '0');
    BLM_status_reg(29)(3 downto 0) <= led_id_state;
    BLM_Output <= BLM_out_signal(11 downto 0);
    
end architecture;