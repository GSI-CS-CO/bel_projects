library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.heap_pkg.all;

entity heap_tb is
end heap_tb;

architecture rtl of heap_tb is 

 

  
signal clk_sys 						: std_logic := '0';
	signal rst_n,rst_lm32_n 					: std_logic := '0';
	
   -- Clock period definitions
   constant clk_period : time := 8 ns;
   constant c_width : natural := 5;
   
   signal s_data_in, s_data_out, sample_in, sample_out : t_data;
   
   signal s_count : std_logic_vector(c_width-1 downto 0); --x"1555AAAAFFFFFFAA";
   signal s_push, s_pop, s_dbg, s_busy, s_full, s_out, s_empty, s_dbg_ok, s_dbg_err : std_logic := '0';
   signal r_push : std_logic := '0';

begin
  
   heap : heap_top
   generic map (
      g_idx_width    => c_width 
   )            
   port map(clk_sys_i   => clk_sys,
            rst_n_i     => rst_n,
            
            dbg_show_i  => s_dbg,
            dbg_ok_o    => s_dbg_ok,       
            dbg_err_o   => s_dbg_err,
    
          
            push_i      => s_push,
            pop_i       => s_pop,
            busy_o      => s_busy,
            full_o      => s_full,
            empty_o     => s_empty,
            count_o     => s_count,
           
            
            data_i     => s_data_in,
            data_o     => s_data_out, 
            out_o      => s_out 
    
    );
   
  

    
 
    
    
   -- Clock process definitions( clock with 50% duty cycle is generated here.
   clk_process :process
   begin
        clk_sys <= '0';
        wait for clk_period/2;  --for 0.5 ns signal is '0'.
        clk_sys <= '1';
        wait for clk_period/2;  --for next 0.5 ns signal is '1'.
        
   end process;
   
   sample :process(clk_sys)
   begin
   if(rising_edge(clk_sys)) then
   
   if(s_push = '1') then
    sample_in <= s_data_in;
   end if;
   if(s_out = '1') then
      sample_out <= s_data_out;
   end if;
   end if;  
   end process;  
     
   
   
   -- Stimulus process
  stim_proc: process
  variable i : natural;
   begin        
        i := 50;
        rst_n  <= '0';
        s_push <= '0';
        s_pop <= '0';
        s_dbg <= '0';
        s_data_in <= std_logic_vector(to_unsigned(i*2+1, t_key'length)) & std_logic_vector(to_unsigned(0, t_val'length));  
        wait for clk_period*5;
        rst_n <= '1';
        wait until rising_edge(clk_sys);  
        wait for clk_period*10;
        
        report "Fill 6" severity note;
        
        report "+++++++++++++++ +++++++++++++++ +++++++++++++ Start INSERT" severity warning;
        
        while (i > 0) 
        loop
        
        s_push <= '1';
        
        wait for clk_period;
        s_data_in <= std_logic_vector(to_unsigned(i*2, t_key'length)) & s_data_in(t_sval'range); 
        s_push <= '0'; 
        wait for clk_period*20;
        i := i -1;
        s_dbg <= '1';
        wait for clk_period;
        s_dbg <= '0';
        wait for clk_period*150;
        
        end loop;
        s_data_in <= std_logic_vector(to_unsigned(64, t_key'length)) & std_logic_vector(to_unsigned(0, t_val'length));  
        
        wait for clk_period*10;
        
        i := 50;
        report "+++++++++++++++ +++++++++++++++ +++++++++++++ Start REPLACE" severity warning;
        while (i > 0) 
        loop
        s_data_in <= std_logic_vector(to_unsigned(i*2, t_key'length)) & s_data_in(t_sval'range);
        wait for clk_period*3;
        s_push <= '1';
        s_pop <= '1';
        wait for clk_period;
        s_push <= '0';
        s_pop <= '0'; 
        wait for clk_period*20;
        
        s_dbg <= '1';
        wait for clk_period;
        s_dbg <= '0';
        wait for clk_period*150;
        i := i -1;
        end loop;
        
        
        i := 50;
        report "+++++++++++++++ +++++++++++++++ +++++++++++++ Start REMOVE" severity warning;
        while (i > 0) 
        loop
        s_data_in <= std_logic_vector(unsigned(s_data_in(t_skey'range)) -1) & s_data_in(t_sval'range);
        s_push <= '0';
        s_pop <= '1';
        wait for clk_period;
        s_push <= '0';
        s_pop <= '0'; 
        wait for clk_period*30;
        
        s_dbg <= '1';
        wait for clk_period;
        s_dbg <= '0';
        wait for clk_period*200;
        i := i -1;
        end loop;
        
         
        wait until rst_n = '0';
  end process;



end rtl;  
