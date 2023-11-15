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
  up_counter_reg: in t_BLM_reg_Array;  
  down_counter_reg: in t_BLM_reg_Array; 
  BLM_cnt_read_Reg: in std_logic_vector(15 downto 0);
  BLM_Output      : out std_logic_vector(5 downto 0);
  BLM_status_Reg : out t_IO_Reg_0_to_23_Array 

  );

end BLM_out_el;


architecture rtl of BLM_out_el is

signal sel_tot: std_logic_vector(1931 downto 0);
type t_sel is array (0 to 5) of std_logic_vector(321 downto 0);
signal sel: t_sel;
type t_int_sel is array (0 to 5) of integer;
signal int_sel: t_int_sel;

signal BLM_out_signal, Out_to_or: std_logic_vector(5 downto 0);

signal OVERFLOW : std_logic_vector(321 downto 0);
signal UP_OVFL, DOWN_OVFL: std_logic_vector(127 downto 0); 

signal gate_input: std_logic_vector(11 downto 0);
signal out_cnt_wr : std_logic_vector(127 downto 0); 
signal up_backout, down_backout: std_logic_vector(15 downto 0);
signal read_up_cnt, read_down_cnt: integer range 0 to 127;
signal read_cnt: integer range 0 to 127;
begin

    out_counter_buffer_proc: process (clk, nRST)
    begin
        if not nRST='1' then 
            for i in 0 to 127 loop
               UP_OVFL(i) <= '0';
               DOWN_OVFL(i) <= '0';
               out_cnt_wr(i) <='0';
            end loop;
            elsif (clk'EVENT AND clk= '1') then 
                for i in 0 to 7 loop
                    out_cnt_wr((16*i+15)downto (16*i)) <=  BLM_out_sel_reg(121 + i);
                end loop;

                for i in 0 to 127 loop
                   
                    if out_cnt_wr(i) ='1' then  
                        UP_OVFL(i) <= UP_OVERFLOW(i);
                        DOWN_OVFL(i) <= DOWN_OVERFLOW(i);    
                    end if;
                end loop;
            end if;        
    end process;


        OVERFLOW <= wd_out& gate_out & UP_OVFL & DOWN_OVFL;
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
--
    status_reg_counter_value_process: process (up_counter_reg, down_counter_reg, BLM_cnt_read_Reg)
        begin
            read_up_cnt <=  to_integer(unsigned(BLM_cnt_read_Reg(3 downto 0)));
            read_down_cnt <=  to_integer(unsigned(BLM_cnt_read_Reg(7 downto 4)));
            up_backout <= up_counter_reg(read_up_cnt);
            down_backout <= down_counter_reg(read_down_cnt);
            --read_cnt <= to_integer(unsigned(BLM_cnt_read_Reg(3 downto 0)));
            --up_backout <= up_counter_reg(read_cnt);
            --down_backout <= down_counter_reg(read_cnt);
            BLM_status_reg(22)<= up_backout;
            BLM_status_reg(23)<= down_backout;
        end process;
       
end architecture;