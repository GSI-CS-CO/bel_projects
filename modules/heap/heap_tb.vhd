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

   constant c_dummy_slave_in     : t_wishbone_slave_in   := ('0', '0', x"00000000", x"F", '0', x"00000000");
   constant c_dummy_slave_out    : t_wishbone_slave_out  := ('0', '0', '0', '0', '0', x"00000000");
   constant c_dummy_master_out   : t_wishbone_master_out := c_dummy_slave_in;
   constant c_dummy_master_in    : t_wishbone_master_in  := c_dummy_slave_out;

   constant c_key_width : natural := 64;
   constant c_val_width : natural := 192;

   constant c_data_width : natural := c_key_width + c_val_width;
   
   constant c_words_per_entry : natural := (c_data_width + t_wishbone_data'length -1) / t_wishbone_data'length;
   
   subtype t_val   is std_logic_vector(c_val_width  -1 downto 0);
   subtype t_key   is std_logic_vector(c_key_width  -1 downto 0);
   subtype t_data  is std_logic_vector(c_data_width -1 downto 0);
   subtype t_skey  is std_logic_vector(c_data_width -1 downto c_val_width);
   subtype t_sval  is t_val;

  
   signal clk_sys 			            : std_logic := '0';
	signal rst_n,rst_lm32_n             : std_logic := '0';
	
   -- Clock period definitions
   constant clk_period     : time      := 8 ns;
   constant c_width        : natural   := 5;
   
   signal s_data_in, s_data_out, sample_in, sample_out : t_data;
   
   signal s_count : std_logic_vector(c_width-1 downto 0); --x"1555AAAAFFFFFFAA";
   signal s_push, s_pop, s_dbg, s_busy, s_full, s_out, s_empty, s_dbg_ok, s_dbg_err : std_logic := '0';
   signal r_push : std_logic := '0';
   
   signal s_ctrl_i : t_wishbone_slave_in;
   signal s_ctrl_o : t_wishbone_slave_out;
   
   signal s_snk_i : t_wishbone_slave_in;
   signal s_snk_o : t_wishbone_slave_out;
   
   signal s_src_i : t_wishbone_master_in;
   signal s_src_o : t_wishbone_master_out;
   
   
   
   

begin
  
   heap : heap_top
   generic map (
      g_idx_width    => c_width,
      g_val_width    => c_val_width,
      g_key_width    => c_key_width 
   )            
   port map(clk_sys_i   => '0',
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
   
  
   wb_heap : xwb_heap
   generic map(
      g_is_ftm       => false,  
      g_idx_width    => c_width,
      g_key_width    => 64,
      g_val_width    => 56
       
   )           
   port map(
      clk_sys_i   => clk_sys,
      rst_n_i     => rst_n,

      ctrl_i      => s_ctrl_i,
      ctrl_o      => open,
      
      snk_i       => s_snk_i,
      snk_o       => open,
      
      src_o       => s_src_o,
      src_i       => c_dummy_master_in
     
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
  rst: process
  begin        
        
        rst_n  <= '0';
        wait until rising_edge(clk_sys);
        wait for clk_period*5;
        rst_n <= '1';
        wait for clk_period*10;
        wait until rst_n = '0';
   end process;
   

   
    -- Stimulus process
  stim_WB_proc: process
  variable i, j : natural;
  variable v_key : std_logic_vector(c_key_width-1 downto 0);
  variable v_val : std_logic_vector(c_val_width-1 downto 0);
  variable v_data : std_logic_vector(c_words_per_entry * t_wishbone_data'length-1 downto 0); 
   begin        
        
        i := 50;
        s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
        s_snk_i  <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
        wait until rst_n = '1';
        wait until rising_edge(clk_sys);
       
        
        report "+++++++++++++++ +++++++++++++++ +++++++++++++ Start INSERT" severity warning;
        
        while (i > 0) 
        loop
        
        v_key := std_logic_vector(to_unsigned(i*2, v_key'length));
        v_val := (others => '1');
        v_data := v_key & v_val & std_logic_vector( to_unsigned(0, (c_words_per_entry * t_wishbone_data'length - c_data_width)));
        
        for j in 0 to c_words_per_entry-1 loop 
           s_snk_i <= ('1', '1', x"00000000", x"F", '1', v_data(v_data'left - j*t_wishbone_data'length downto  v_data'length - (j+1)*t_wishbone_data'length));
           wait for clk_period*1;
        end loop;
        s_snk_i  <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
        wait for clk_period*1;
        wait for clk_period*20;
        i := i -1;
        s_ctrl_i <= ('1', '1', x"00000008", x"F", '1', x"00000001"); -- dbg set
        wait for clk_period;
        s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
        wait for clk_period*150;
        
        end loop;
          
        
        wait for clk_period*10;
        
        -- i := 50;
        -- report "+++++++++++++++ +++++++++++++++ +++++++++++++ Start REPLACE" severity warning;
        -- while (i > 0) 
        -- loop
        -- s_data_in <= std_logic_vector(to_unsigned(i*2, t_key'length)) & s_data_in(t_sval'range);
        -- wait for clk_period*3;
        -- s_push <= '1';
        -- s_pop <= '1';
        -- wait for clk_period;
        -- s_push <= '0';
        -- s_pop <= '0'; 
        -- wait for clk_period*20;
        
        -- s_dbg <= '1';
        -- wait for clk_period;
        -- s_dbg <= '0';
        -- wait for clk_period*150;
        -- i := i -1;
        -- end loop;
        
        
        i := 50;
        report "+++++++++++++++ +++++++++++++++ +++++++++++++ Start REMOVE" severity warning;
        while (i > 0) 
        loop
        
           s_ctrl_i <= ('1', '1', x"00000004", x"F", '1', x"00000001"); -- force-pop
           wait for clk_period;
           s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
           wait for clk_period*30;
           i := i -1;
           s_ctrl_i <= ('1', '1', x"00000008", x"F", '1', x"00000001"); -- dbg set
           wait for clk_period;
           s_ctrl_i <= ('0', '0', x"00000000", x"F", '0', x"00000000"); 
           wait for clk_period*150;
        end loop;
         
        wait until rst_n = '0';
  end process;

   
   
   -- Stimulus process
  stim_proc: process
  variable i : natural;
   begin        
        
        i := 50;
        wait until rising_edge(clk_sys);  
        s_push <= '0';
        s_pop <= '0';
        s_dbg <= '0';
        s_data_in <= std_logic_vector(to_unsigned(i*2+1, t_key'length)) & std_logic_vector(to_unsigned(0, t_val'length));  
        wait until rst_n = '1';
        
        
       
        
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
