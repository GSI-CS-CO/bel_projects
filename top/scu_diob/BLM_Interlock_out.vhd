LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;

entity BLM_Interlock_out is

        
port (
        CLK              : in std_logic;      -- Clock
        nRST             : in std_logic;      -- Reset
        out_mux_sel      : in std_logic_vector(31 downto 0);
        UP_OVERFLOW      : in t_counter_in_Array ; 
        DOWN_OVERFLOW    : in t_counter_in_Array  ; 
        gate_UP_OVERFLOW  : out t_gate_counter_in_Array;
        gate_DOWN_OVERFLOW: out t_gate_counter_in_Array;
        gate_error       : in std_logic_vector(11 downto 0);
        Interlock_IN     : in std_logic_vector(53 downto 0);
        INTL_Output      : out std_logic_vector(5 downto 0);
        BLM_status_Reg : out t_IO_Reg_0_to_7_Array
);
end BLM_Interlock_out;

architecture rtl of BLM_Interlock_out is
--out_mux_sel 0 for gate error
--out_mux_sel 4..1 for interlock_in
--out_mux_sel 12..6 for input:
-- if out_mux_sel 5 ='0' up_overflow_control
-- if out_mux_sel 5 ='1' down_overflow_control




      signal in_overflow: std_logic_vector(1023 downto 0);
     -- signal status_Reg: std_logic_vector(15 downto 0);
      signal out_mux_res: std_logic_vector(5 downto 0);
      signal gate_err_res: std_logic_vector(5 downto 0);
      signal intl_wd_res: std_logic_vector(5 downto 0);
      signal overflow: std_logic_vector(5 downto 0);
      signal gate_overflow: std_logic_vector(143 downto 0);
      signal tot_overflow: std_logic_vector(1167 downto 0);

      signal m: integer range 0 to 195 :=0;
      begin
     
         
         mux_in_process:process (nRST, CLK)
          begin
          if not nRST='1' then 
                in_overflow <=   (OTHERS =>  '0');
                gate_err_res <=   (OTHERS =>  '0');
                intl_wd_res <=   (OTHERS =>  '0');
                gate_overflow <= (others =>'0');
                tot_overflow <= (others =>'0');
                overflow <= (OTHERS =>'0');
                out_mux_res <= (OTHERS =>'0');
       
            
         elsif (CLK'EVENT AND CLK = '1') then  

            m<= to_integer(unsigned(out_mux_sel(13 downto 6)));

                --gate_Error_control
                   if out_mux_sel(0) ='0' then 
                        gate_err_res <= gate_error(5 downto 0);
                   else                        
                        gate_err_res <= gate_error(11 downto 6);
                   end if;

                   case out_mux_sel(4 downto 1) is

                        when "0000" => intl_wd_res <= Interlock_IN(5 downto 0);
                        when "0001" => intl_wd_res <= Interlock_IN(11 downto 6);
                        when "0010" => intl_wd_res <= Interlock_IN(17 downto 12);
                        when "0011" => intl_wd_res <= Interlock_IN(23 downto 18);
                        when "0100" => intl_wd_res <= Interlock_IN(29 downto 24);
                        when "0101" => intl_wd_res <= Interlock_IN(35 downto 30);
                        when "0110" => intl_wd_res <= Interlock_IN(41 downto 36);
                        when "0111" => intl_wd_res <= Interlock_IN(47 downto 42);
                        when "1000" => intl_wd_res <= Interlock_IN(53 downto 48);

                        when others => NULL;
                  end case;
                  
                if gate_err_res ="000000" AND intl_wd_res = "000000"  then

                  if out_mux_sel(5)='1' then 

                        in_overflow <= UP_OVERFLOW(7)&UP_OVERFLOW(6)&UP_OVERFLOW(5)&UP_OVERFLOW(4)&UP_OVERFLOW(3)&UP_OVERFLOW(2)&UP_OVERFLOW(1)&UP_OVERFLOW(0)& 
                                       DOWN_OVERFLOW(7)& DOWN_OVERFLOW(6)& DOWN_OVERFLOW(5)& DOWN_OVERFLOW(4)& DOWN_OVERFLOW(3)& DOWN_OVERFLOW(2)& DOWN_OVERFLOW(1) & DOWN_OVERFLOW(0);
                        
                        gate_overflow <= gate_UP_OVERFLOW(5) & gate_UP_OVERFLOW(4) & gate_UP_OVERFLOW(3) & gate_UP_OVERFLOW(2) & gate_UP_OVERFLOW(1) & gate_UP_OVERFLOW(0) &
                                        gate_DOWN_OVERFLOW(5) & gate_DOWN_OVERFLOW(4) & gate_DOWN_OVERFLOW(3) & gate_DOWN_OVERFLOW(2) & gate_DOWN_OVERFLOW(1) & gate_DOWN_OVERFLOW(0); 

                        tot_overflow <= gate_overflow & in_overflow;
                 end if;  
  --------------------- 
                                
                for i in 0 to 194 loop
                 if m = i then 
                  overflow <= tot_overflow((6*m +5) downto (6*m));
                end if;
                end loop;
                if m = 194 then 
                  overflow <= "00" & tot_overflow(1167 downto 1164);
                end if;
                
  --------------------     
                  out_mux_res <= overflow;
                else
                  if gate_err_res = "000000" then

                  out_mux_res <= intl_wd_res;
                 else
  
                 end if;
                end if;
         end if;

        end process;



        INTL_Output <= out_mux_res;

     --------------------------------------------------------------------------------------------------
     -----                         BLM_STATUS_REGISTERS               
     --------------------------------------------------------------------------------------------------

        BLM_status_reg(0)<= "0000000000" & INTL_Output;
        BLM_status_reg(1)<= "00"&gate_error(11 downto 6) & "00" & gate_error(5 downto 0);
        BLM_status_reg(2)<= "00"& interlock_IN(11 downto 6) & "00" & Interlock_IN(5 downto 0);
        BLM_status_reg(3)<= "00"& interlock_IN(23 downto 18) & "00" & Interlock_IN(17 downto 12);
        BLM_status_reg(4)<= "00" & interlock_IN(35 downto 30)&"00"& Interlock_IN(29 downto 24);
        BLM_status_reg(5)<= "00"& interlock_IN(47 downto 42)&"00"& Interlock_IN(41 downto 36);
        BLM_status_reg(6)<= "0000000000"& interlock_IN(53 downto 48);
        BLM_status_reg(7)<= "0000000000"& overflow;

      
end rtl;
