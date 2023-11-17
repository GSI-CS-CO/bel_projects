LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;
 
entity BLM_out_el is
    
port (
  CLK              : in std_logic;      -- Clock
  nRST             : in std_logic;      -- Reset
 -- BLM_ctrl_reg     : in std_logic_vector(11 downto 0); -- bit 5-0 = up_in_counter select, bit 11-6 = down_in_counter select
     BLM_out_sel_reg : in t_BLM_out_sel_reg_Array;  --128 x 16 bits = Reg120 -0:  "0000" and 6 x (54 watchdog errors  + 12 gate errors + 256 counter outputs) 
                                                    -- Reg 128-121  = 128 bits used to write the counters overflows outputs to the UP/DOWN overflows to be read                                           

  UP_OVERFLOW      : in std_logic_vector(127 downto 0);
  DOWN_OVERFLOW    : in std_logic_vector(127 downto 0);

  wd_out           : in std_logic_vector(53 downto 0); 
  gate_in          : in std_logic_vector(11 downto 0); -- to be sent to the status registers
  gate_out        : in std_logic_vector (11 downto 0); 
  counter_reg: in t_BLM_counter_Array;
 
  BLM_Output      : out std_logic_vector(5 downto 0);
  BLM_status_Reg : out t_IO_Reg_0_to_23_Array 

  );

end BLM_out_el;


architecture rtl of BLM_out_el is
--TYPE    t_BLM_reg_Array           is array (0 to 127) of std_logic_vector(15 downto 0);
signal sel_tot: std_logic_vector(1931 downto 0);
type t_sel is array (0 to 5) of std_logic_vector(321 downto 0);
signal sel: t_sel;
type t_int_sel is array (0 to 5) of integer;
signal int_sel: t_int_sel;

signal BLM_out_signal, Out_to_or: std_logic_vector(5 downto 0);

signal OVERFLOW : std_logic_vector(321 downto 0);


signal gate_input: std_logic_vector(11 downto 0);
signal out_cnt_wr : std_logic; 
signal up_down_counter_val: t_BLM_counter_Array;
signal cnt_backout: std_logic_vector(19 downto 0):=(others =>'0');

signal read_cnt: integer range 0 to 127;
begin


        OVERFLOW <= wd_out& gate_out & UP_OVERFLOW & DOWN_OVERFLOW;
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
            for i in 0 to 119 loop 
                sel_tot((i*16+15) downto i*16)<=  BLM_out_sel_Reg(i);
                sel_tot(1931 downto 1920) <= BLM_out_sel_Reg(120)(11 downto 0);
        end loop;
            
        for j in 0 to 5 loop
              sel(j) <= sel_tot((322*(j+1)-1) downto 322*j); 

              int_sel(j) <= to_integer(unsigned (sel(j)));

             

               out_to_or(j)<= OVERFLOW(int_sel(j));

       
        end loop;
        

       end if;

       BLM_out_signal <= out_to_or;

    end process;



 BLM_Output <= BLM_out_signal;

      

----------------------------------------------------------------------------------------
out_counter_buffer_proc: process (clk, nRST)
begin
    if not nRST='1' then 
        for i in 0 to 127 loop
            up_down_counter_val(i) <= (others =>'0');
        end loop;
         
           out_cnt_wr <='0';
  
        elsif (clk'EVENT AND clk= '1') then 
           
                out_cnt_wr<=  BLM_out_sel_reg(121)(15);
      

               
                if out_cnt_wr='1' then  

                for i in 0 to 127 loop
                    up_down_counter_val(i) <= counter_reg(i);
            
                    end loop;
                end if;
    
        end if;        
end process;

    status_reg_counter_value_process: process (up_down_counter_val,  BLM_out_sel_reg(121))
        begin

            read_cnt <= to_integer(unsigned(BLM_out_sel_Reg(121)(7 downto 0)));
            cnt_backout <= up_down_counter_val(read_cnt);
      

        end process;
    --------------------------------------------------------------------------------------------------
     -----                         BLM_STATUS_REGISTERS               
     --------------------------------------------------------------------------------------------------
     status_reg_overflow_proc: process (OVERFLOW)
     begin
        for i in 0 to 19 loop
            BLM_status_reg(i) <= OVERFLOW((i*16+15) downto i*16);
        end loop;
    end process;
    
    BLM_status_reg(20) <= "00" & gate_input & OVERFLOW(321 downto 320); -- bits 321-320 = wd_out (53 downto 52)
    BLM_status_reg(21)(5 downto 0) <= BLM_out_signal; -- phyfical outputs
    BLM_status_reg(21)(15 downto 6)  <= (others =>'0');    
    BLM_status_reg(22)<= cnt_backout(15 downto 0);
    BLM_status_reg(23) <= "000000000000"& cnt_backout(19 downto 16);

end architecture;