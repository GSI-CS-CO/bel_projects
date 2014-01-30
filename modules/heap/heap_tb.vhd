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
   
   signal time_sys, s_wdat : std_logic_vector(7 downto 0); --x"1555AAAAFFFFFFAA";
   signal s_movkey : t_key;
   
   signal s_data_in, s_data_out : t_data;
   
   signal s_idx, s_last, s_wadr : std_logic_vector(c_width-1 downto 0); --x"1555AAAAFFFFFFAA";
   
   signal s_fedge_push : std_logic_vector(0 downto 0) :=  "0";
   signal s_push : std_logic := '0';
   signal r_push : std_logic := '0';
   signal s_pop, s_dbg  : std_logic := '0'; 

   signal s_we, s_valid, s_flag, s_push_op : std_logic;

begin
  
   s_movkey <= s_data_in(t_skey'range);
    
 
   PF : heap_pathfinder
   generic map (
      g_idx_width    => c_width 
   )            
   port map(clk_sys_i   => clk_sys,
            rst_n_i     => rst_n,
            push_i      => s_push,
            pop_i       => s_pop,
            movkey_i    => s_movkey,
    
            final_o => s_flag,
            idx_o      => s_idx,
            last_o     => s_last,
            valid_o    => s_valid,
            push_op_o  => s_push_op,
                  
            wr_key_i   => s_wdat,
            wr_idx_i   => s_wadr,
            we_i       => s_we
    
    
    );
    
   WR : heap_writer
   generic map (
      g_idx_width    => c_width 
   )            
   port map(clk_sys_i   => clk_sys,
            rst_n_i     => rst_n,
            
            dbg_show_i => s_dbg,
            data_i     => s_data_in,
            data_o     => s_data_out, 
                
            final_i => s_flag,
            idx_i      => s_idx,
            last_i     => s_last,
            push_op_i  => s_push,
            en_i    => s_valid,
    
            wr_key_o   => s_wdat,
            wr_idx_o   => s_wadr,
            we_o       => s_we
    
    
    );
    
    
   -- Clock process definitions( clock with 50% duty cycle is generated here.
   clk_process :process
   begin
        time_sys <= std_logic_vector(unsigned(time_sys) + unsigned(s_fedge_push));
        clk_sys <= '0';
        wait for clk_period/2;  --for 0.5 ns signal is '0'.
        clk_sys <= '1';
        wait for clk_period/2;  --for next 0.5 ns signal is '1'.
        r_push <= s_push;
        
   end process;
   
   s_fedge_push(0) <= not s_push and r_push;
   
   
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
