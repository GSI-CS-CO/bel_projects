LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;
 
entity BLM_out_el is
    
port (
  CLK              : in std_logic;      -- Clock
  nRST             : in std_logic;      -- Reset
 -- BLM_ctrl_reg     : in std_logic_vector(11 downto 0); -- bit 5-0 = up_in_counter select, bit 11-6 = down_in_counter select
     BLM_out_sel_reg : in t_BLM_out_sel_reg_Array;  --217 x 16 bits = "0000" and 6 x (54 watchdog errors  + 12 gate errors + 512 counter outputs)                                                

  UP_OVERFLOW      : in std_logic_vector(255 downto 0);
  DOWN_OVERFLOW    : in std_logic_vector(255 downto 0);

  wd_out           : in std_logic_vector(53 downto 0); 
  gate_in          : in std_logic_vector(11 downto 0); -- to be sent to the status registers
  gate_out        : in std_logic_vector (11 downto 0); 
     
  BLM_Output      : out std_logic_vector(5 downto 0);
  BLM_status_Reg : out t_IO_Reg_0_to_36_Array
  );

end BLM_out_el;


architecture rtl of BLM_out_el is

signal sel_tot: std_logic_vector(3471 downto 0);
type t_sel is array (0 to 5) of std_logic_vector(577 downto 0);
signal sel: t_sel;
type t_int_sel is array (0 to 5) of integer;
signal int_sel: t_int_sel;

signal BLM_out_signal, Out_to_or: std_logic_vector(5 downto 0);

signal OVERFLOW : std_logic_vector(577 downto 0);


signal gate_input: std_logic_vector(11 downto 0);



begin
        OVERFLOW <= wd_out& gate_out & UP_OVERFLOW & DOWN_OVERFLOW ;
        gate_input <= gate_in;


    up_down_signals_to_counter_proc: process (clk,nRST)
        begin
            if not nRST='1' then 
            for i in 0 to 5 loop
                int_sel(i) <=0;
            end loop;
           BLM_out_signal <=( others =>'0');
           Out_to_or <=(others =>'0');
         

              
       elsif (clk'EVENT AND clk= '1') then 
         -- for i in 0 to 191 loop
            for i in 0 to 216 loop 
                sel_tot((i*16+15) downto i*16)<=  BLM_out_sel_Reg(i);

        end loop;
            
        for j in 0 to 5 loop
              sel(j) <= sel_tot((578*(j+1)-1) downto 578*j); 



              int_sel(j) <= to_integer(unsigned (sel(j)));
               out_to_or(j)<= OVERFLOW(int_sel(j));
             
        end loop;
        

       end if;

       BLM_out_signal <= out_to_or;

    end process;



 BLM_Output <= BLM_out_signal;

      
    --------------------------------------------------------------------------------------------------
     -----                         BLM_STATUS_REGISTERS               
     --------------------------------------------------------------------------------------------------
     status_reg_proc: process (OVERFLOW)
     begin
        for i in 0 to 35 loop
            BLM_status_reg(i) <= OVERFLOW((i*16+15) downto i*16);
        end loop;
    end process;
    
    BLM_status_reg(36) <= "00" & gate_input & OVERFLOW(577 downto 576); -- bits 577-576 = wd_out (53 downto 52)
            
          

end architecture;

