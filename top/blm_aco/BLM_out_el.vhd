LIBRARY IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
use work.scu_diob_pkg.all;
 
entity BLM_out_el is
    
port (
  CLK              : in std_logic;      -- Clock
  nRST             : in std_logic;      -- Reset
 -- BLM_ctrl_reg     : in std_logic_vector(11 downto 0); -- bit 5-0 = up_in_counter select, bit 11-6 = down_in_counter select
                                                       
  BLM_out_ena_Reg :  in t_BLM_out_reg_Array;             -- 192 x 16 bits register for the output selection of the 256 counters and comparators results 
  UP_OVERFLOW      : in std_logic_vector(255 downto 0);
  DOWN_OVERFLOW    : in std_logic_vector(255 downto 0);
  BLM_out_mux_Reg  : in t_BLM_mux_reg_Array;  -- 6 16 bits registers for the selection of gate errors, watchdog errors and inputs to the last OR computation
      --  For each register: bit 15 free, bit 14-10: 5 bit for the last or, bit 9-4: 6 bits for the watchdog errors, bit 3-0: 4 bits for gate errors

  wd_out           : in std_logic_vector(53 downto 0); 
  gate_in          : in std_logic_vector(11 downto 0);
  gate_out        : in std_logic_vector (11 downto 0);
     
  BLM_Output      : out std_logic_vector(5 downto 0);
  BLM_status_Reg : out t_IO_Reg_0_to_7_Array
  );

end BLM_out_el;


architecture rtl of BLM_out_el is

signal sel_tot: std_logic_vector(3071 downto 0);
type t_sel is array (0 to 5) of std_logic_vector(511 downto 0);
signal sel: t_sel;
type t_int_sel is array (0 to 5) of integer;
signal int_sel: t_int_sel;

signal BLM_out_signal, Out_to_or: std_logic_vector(5 downto 0);
signal OVERFLOW : std_logic_vector(511 downto 0);
signal BLM_gate_sel: t_BLM_12_to_6_mux_sel_array;
signal BLM_wd_sel: t_BLM_wd_mux_sel_array;
signal BLM_or_sel: t_BLM_or_mux_sel_array;
signal Gate_error_out_signal: std_logic_vector(5 downto 0);
signal wd_error_out_signal: std_logic_vector(5 downto 0);
signal in_or_signal: std_logic_vector(17 downto 0);

component BLM_gate_err_mux is

    port (
        CLK             : in std_logic;      -- Clock
        nRST            : in std_logic;      -- Reset
        sel             : in t_BLM_12_to_6_mux_sel_array ;
        in_s            : in std_logic_vector(11 downto 0);
        out_s           : out std_logic_vector(5 downto 0)
      
    );
  end component BLM_gate_err_mux;

  component BLM_wd_err_mux is

    port (
     
        CLK               : in std_logic;      -- Clock
        nRST              : in std_logic;      -- Reset
        mux_sel           : in t_BLM_wd_mux_sel_array ;
        in_mux            : in std_logic_vector(53 downto 0);
        out_mux           : out std_logic_vector(5 downto 0)
      
    );
end component BLM_wd_err_mux;

component BLM_or_mux is

    port (
        CLK               : in std_logic;      -- Clock
        nRST              : in std_logic;      -- Reset
        mux_sel           : in t_BLM_or_mux_sel_array ; -- 0 to 5 of (std_logic_vector(4 downto 0)
        in_mux            : in std_logic_vector(17 downto 0);
        out_mux           : out std_logic_vector(5 downto 0)
      
    );
end component BLM_or_mux;

begin
        OVERFLOW <=UP_OVERFLOW & DOWN_OVERFLOW;
     control_sig_process: process (BLM_out_mux_Reg)
     begin   
        for i in 0 to 5 loop
            BLM_gate_sel(i) <= BLM_out_mux_Reg(i)(3 downto 0);
            BLM_wd_sel(i) <= BLM_out_mux_Reg(i)(9 downto 4);
            BLM_or_sel(i) <= BLM_out_mux_Reg(i)(14 downto 10);
        end loop;
     end process;

        up_down_signals_to_counter_proc: process (clk,nRST)
        begin
            if not nRST='1' then 
            for i in 0 to 5 loop
                int_sel(i) <=0;
            end loop;
           BLM_out_signal <=( others =>'0');
           Out_to_or <=(others =>'0');
         

              
       elsif (clk'EVENT AND clk= '1') then 
          for i in 0 to 191 loop
            
                sel_tot((i*16+15) downto i*16)<=  BLM_out_ena_Reg(i);

        end loop;

        for j in 0 to 5 loop
              sel(j) <= sel_tot(511*(j+1) downto 511*j);



              int_sel(j) <= to_integer(unsigned (sel(j)));
               out_to_or(j)<= OVERFLOW(int_sel(j));
             
        end loop;
        

end if;

BLM_out_signal <= out_to_or;

    end process;


    gate_Error_to_or: BLM_gate_err_mux

        port map(
            CLK   => CLK,
            nRST  => nRST,
            sel => BLM_gate_sel,
            
            in_s     => gate_out,
            out_s   => Gate_error_out_signal
          
        );
      
    
 wd_Error_to_or: BLM_wd_err_mux 
    
        port map (
            CLK   => CLK,
            nRST  => nRST,
            mux_sel => BLM_wd_sel,
            in_mux  =>wd_out,
            out_mux  => wd_error_out_signal
          
        );

in_or_signal <= BLM_out_signal & Gate_error_out_signal & wd_error_out_signal;

output_selection: BLM_or_mux 
    
        port map (
            CLK   => CLK,
            nRST  => nRST,
            mux_sel => BLM_or_sel,
            in_mux   =>in_or_signal,
            out_mux  => BLM_Output
          
            );


      
    --------------------------------------------------------------------------------------------------
     -----                         BLM_STATUS_REGISTERS               
     --------------------------------------------------------------------------------------------------


            BLM_status_reg(0)<=  "00" & BLM_out_signal & "00" &  BLM_Output; --BLM_out_signal and BLM output
            BLM_status_reg(1)<= "00"& gate_out(11 downto 6) & "00" & gate_out(5 downto 0);         -- gate error
            BLM_status_reg(2)<= "00"& wd_out(11 downto 6) & "00" & wd_out(5 downto 0);    -- interlock board 1 and board 2
            BLM_status_reg(3)<= "00"& wd_out(23 downto 18) & "00" & wd_out(17 downto 12); -- interlock board 3 and board 4
            BLM_status_reg(4)<= "00" & wd_out(35 downto 30)&"00"& wd_out(29 downto 24);   -- interlock board 5 and board 6
            BLM_status_reg(5)<= "00"& wd_out(47 downto 42)&"00"& wd_out(41 downto 36);    -- interlock board 7 and board 8
            BLM_status_reg(6)<= "0000000000"& wd_out(53 downto 48);                             -- interlock board 9
            BLM_status_reg(7) <= "00"& gate_in(11 downto 6) & "00" & gate_in(5 downto 0);
          

end architecture;
