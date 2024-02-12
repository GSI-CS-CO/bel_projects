LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;
use IEEE.std_logic_misc.all;

 
entity BLM_out_el is
    
port (
  CLK              : in std_logic;      -- Clock
  nRST             : in std_logic;      -- Reset

     BLM_out_sel_reg : in t_BLM_out_sel_reg_Array;  -- 122 x 16 bits = Reg120-0:  "0000" and 6 x (54 watchdog errors  + 12 gate errors + 256 counters overflows outputs) 
                                                     -- + 4 more registers for 6 x 12 input gate (= 72 bits) to be send to the outputs.    
                                                     --=> 126 registers             
                                                    --   REg127             ex Reg121: counter outputs buffering enable (bit 15) and buffered output select (bit 7-0). Bits 14-8 not used     
                                                                            

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

signal sel_tot: std_logic_vector(2003 downto 0); --I need 4 more 16 bits registers 
type t_sel is array (0 to 5) of std_logic_vector(333 downto 0);
signal sel, product: t_sel;
--type t_int_sel is array (0 to 5) of integer;
--signal int_sel: t_int_sel;

signal BLM_out_signal, Out_to_or: std_logic_vector(5 downto 0):=(others =>'0');

signal OVERFLOW : std_logic_vector(333 downto 0);


signal gate_input: std_logic_vector(11 downto 0);
signal out_cnt_wr : std_logic; 
signal up_down_counter_val: t_BLM_counter_Array;
signal cnt_readback: std_logic_vector(19 downto 0):=(others =>'0');
TYPE    t_cnt     is array (0 to 5)  of integer;
--signal read_cnt: integer range 0 to 127;

signal read_cnt: t_cnt:= (others => 0);
--signal flag_cnt: t_cnt;
signal read_counters: integer range 0 to 255;

signal ena_out: std_logic_vector(5 downto 0);

begin


OVERFLOW <= gate_in& wd_out& gate_out & UP_OVERFLOW & DOWN_OVERFLOW;
gate_input <= gate_in;

sel_signal_proc: process (BLM_out_sel_Reg)

    begin

        for i in 0 to 124 loop 
            sel_tot((i*16+15) downto i*16)<=  BLM_out_sel_Reg(i);
            sel_tot(2003 downto 2000) <= BLM_out_sel_Reg(125)(3 downto 0);
        end loop;
        
        for k in 0 to 5 loop
            sel(k) <= sel_tot((334*(k+1)-1) downto 334*k); 
        end loop;
        
   end process;


out_signals_proc: process (OVERFLOW, sel)
begin
    for j in 0 to 5 loop
        for i in 0 to 333 loop
            product(j)(i) <= sel(j)(i) and OVERFLOW(i);
        end loop;
           ena_out(j) <= or_reduce(sel(j));
           -- if ena_out(j) = '1' then
            --    BLM_out_signal(j) <= or_reduce(product(j));
           -- else
            --    BLM_out_signal(j) <= '1';
           -- end if;    
        
    end loop;    
end  process;

      
ena_out_signals_proc: process (clk, nRST)
begin
    if not nRST='1' then 
        for j in 0 to 5 loop
            
            BLM_out_signal(j) <= '1';
        end loop;
    elsif (clk'EVENT AND clk= '1') then 
        for j in 0 to 5 loop
          
           if ena_out(j) = '1' then
               BLM_out_signal(j) <= or_reduce(product(j));
            else
                BLM_out_signal(j) <= '1';
           end if;    
        
    end loop; 
    end if;   
end  process;

      
----------------------------------------------------------------------------------------
out_counter_buffer_proc: process (clk, nRST)
begin
    if not nRST='1' then 
        for i in 0 to 127 loop
            up_down_counter_val(i) <= (others =>'0');
        end loop;
         
           out_cnt_wr <='0';
  
        elsif (clk'EVENT AND clk= '1') then 
           
                out_cnt_wr<=  BLM_out_sel_reg(126)(15);
      

               
                if out_cnt_wr='1' then  

                for i in 0 to 127 loop
                    up_down_counter_val(i) <= counter_reg(i);
            
                    end loop;
                end if;
    
        end if;        
end process;

    status_reg_counter_value_process: process (up_down_counter_val,  BLM_out_sel_reg(121))
        begin

            read_counters <= to_integer(unsigned(BLM_out_sel_Reg(126)(7 downto 0)));
            cnt_readback <= up_down_counter_val(read_counters);
      

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
    
    BLM_status_reg(20) <= "00" & not OVERFLOW(333 downto 320); -- bits 333-321 gate inputs, bits 321-320 = wd_out (53 downto 52)
    BLM_status_reg(21)(5 downto 0) <= not BLM_out_signal; -- physical outputs
    BLM_status_reg(21)(15 downto 6)  <= (others =>'0');    
    BLM_status_reg(22)<= cnt_readback(15 downto 0);
    BLM_status_reg(23) <= "000000000000"& cnt_readback(19 downto 16);


 BLM_Output <= BLM_out_signal;

end architecture;