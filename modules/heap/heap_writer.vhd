
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library work;
use work.genram_pkg.all;
use work.wishbone_pkg.all;
use work.gencores_pkg.all;
use work.heap_pkg.all;

entity heap_writer is
  generic(
    g_idx_width    : natural := 8
  );            
  port(
    clk_sys_i  : in  std_logic;
    rst_n_i    : in  std_logic;

    dbg_show_i : in std_logic;

    data_i    : in  t_data;
    data_o    : out  t_data;
    
    idx_i     : in std_logic_vector(g_idx_width-1 downto 0);
    last_i    : in std_logic_vector(g_idx_width-1 downto 0);
    final_i   : in std_logic;
    push_i    : in std_logic;
    en_i      : in std_logic;
    
    
    wr_key_o   : out  t_key;     -- writes
    wr_idx_o   : out  std_logic_vector(g_idx_width-1 downto 0);
    we_o       : out  std_logic
    );
end heap_writer;

architecture behavioral of heap_writer is

   constant c_elements  : natural := 2**g_idx_width;
   constant c_first     : std_logic_vector(g_idx_width-1 downto 0) := std_logic_vector(to_unsigned(1, g_idx_width)); 
   constant c_INP       : std_logic_vector (1 downto 0)  := "00";
   constant c_OUTPUT    : std_logic_vector (1 downto 0)  := "01";
   constant c_UPDATE    : std_logic_vector (1 downto 0)  := "10";
   constant c_DBG       : std_logic_vector (1 downto 0)  := "11";
     
  
   type t_op_state is (e_IDLE, e_SETUP, e_COPY, e_SET_ADR_1ST, e_SET_ADR_LAST_CPY_1ST, e_CPY_LAST, e_DBG_RD_C, e_DBG_RD_P, e_DBG_VER, e_DBG_WAIT);
   signal r_state : t_op_state;
   
   signal r_dbg_show : std_logic;
   
   signal r_mov : t_data;
   signal r_rd_adr_mode : std_logic_vector (1 downto 0);
   
   signal r_wr_idx1, r_wr_idx0 : std_logic_vector(g_idx_width-1 downto 0);
   signal s_rd_idx, r_dbg_ptr, r_dbg_adr : std_logic_vector(g_idx_width-1 downto 0);
   signal r_last : std_logic_vector(g_idx_width-1 downto 0);
   
   signal s_rd_val, s_wr_val, r_dbg_cmp : t_data;
   signal s_we : std_logic;
   signal r_fin0, r_fin1, r_en0, r_en1, r_push : std_logic; 
  
begin
  
   VAL_DPRAM : generic_dpram
    generic map(
      -- standard parameters
      g_data_width               => t_data'length,
      g_size                     => c_elements,
      g_with_byte_enable         => false,
      --g_addr_conflict_resolution => "dont_care",
      g_init_file                => "",
      g_dual_clock               => false
      )
    port map(
      rst_n_i => rst_n_i,
      -- Port A
      clka_i  => clk_sys_i,
      wea_i   => '0',
      aa_i    => s_rd_idx,
      da_i    => (others => '0'),
      qa_o    => s_rd_val,
      -- Port B
      clkb_i  => clk_sys_i,
      web_i   => s_we,
      ab_i    => r_wr_idx1,
      db_i    => s_wr_val,
      qb_o    => open
      );
  
   
   -- read first element, last element or index list, 
   inpmux0 : with r_rd_adr_mode select
      s_rd_idx <= c_first  when c_OUTPUT,
                  r_last   when c_UPDATE,
                  r_dbg_adr when c_DBG,
                  idx_i    when others;
   
  
   
   inpmux1 : with r_fin1 select
      s_wr_val <= s_rd_val when '0',
                  r_mov when others;
   
   s_we <= r_en1;
   
   -- write to key rams
   wr_key_o <= s_wr_val(t_skey'range);
   wr_idx_o <= r_wr_idx1;
   we_o     <= s_we;
   
   
   pipeline: process(clk_sys_i)
      variable v_state  : t_op_state;
   
   begin
    if(rising_edge(clk_sys_i)) then
      v_state  := r_state;
      
      if(rst_n_i = '0') then
       
      else
         r_fin0      <= final_i;
         r_fin1      <= r_fin0;
         r_en0       <= en_i;
         r_en1       <= r_en0;
         r_wr_idx0   <= idx_i;
         r_wr_idx1   <= r_wr_idx0;
      end if;
    end if;
    end process pipeline; 
    
     
      
   main: process(clk_sys_i)
      variable v_state  : t_op_state;
   
   begin
    if(rising_edge(clk_sys_i)) then
      v_state  := r_state;
      
      if(rst_n_i = '0') then
         r_state  <= e_IDLE;
         r_push <= '0';
         r_dbg_ptr <= c_first;
      else
         
         r_push <= r_push or push_i ;
         r_last   <= last_i;
         case r_state is
            when e_IDLE    => 
                              if(dbg_show_i = '1') then
                                 r_dbg_ptr <= last_i;
                                 r_dbg_adr <= last_i;
                                 v_state  := e_DBG_RD_C;   
                              else
                              
                                 if(en_i = '1') then
                                    if(r_push = '1') then
                                       r_mov    <= data_i;
                                    end if;
                                    
                                    v_state  := e_COPY;   
                                 end if;   
                              end if;
  
            when e_COPY    => if(r_fin1 = '1') then
                                 r_push <= '0';
                                 v_state  := e_SET_ADR_1ST;   
                              end if;              

            -- copy 1st to output and last to r_mov
            when e_SET_ADR_1ST            => v_state  := e_SET_ADR_LAST_CPY_1ST; 
            when e_SET_ADR_LAST_CPY_1ST   => r_mov    <= s_rd_val;
                                             v_state  := e_CPY_LAST;
            when e_CPY_LAST               => data_o   <= s_rd_val;
                                             v_state  := e_IDLE; 
            
            
            
            
                                    
            
            when e_DBG_RD_C  =>  v_state := e_DBG_RD_P;

            when e_DBG_RD_P  =>  r_dbg_adr <= std_logic_vector(f_get_parent(unsigned(r_dbg_ptr)));
                                 v_state := e_DBG_WAIT;
            
             when e_DBG_WAIT  => r_dbg_cmp <= s_rd_val;
                                 v_state := e_DBG_VER;
                              
            when e_DBG_VER => r_dbg_ptr <= std_logic_vector(unsigned(r_dbg_ptr) -1);
                              r_dbg_adr <= std_logic_vector(unsigned(r_dbg_ptr) -1);
                                report "*** C "   & integer'image(to_integer(unsigned(r_dbg_cmp(t_skey'range))))  
                                       & " @"     & integer'image(to_integer(unsigned(r_dbg_ptr)))
                                       & " >= P "   & integer'image(to_integer(unsigned(s_rd_val(t_skey'range))))
                                       & " @"     & integer'image(to_integer(unsigned(r_dbg_adr))) 
                                 severity note;
                              
                              assert (unsigned(r_dbg_cmp(t_skey'range)) >= unsigned(s_rd_val(t_skey'range)))
                                 report "####### C "   & integer'image(to_integer(unsigned(r_dbg_cmp(t_skey'range))))  
                                       & "@"     & integer'image(to_integer(unsigned(r_dbg_ptr)))
                                       & " < P "   & integer'image(to_integer(unsigned(s_rd_val(t_skey'range))))
                                       & "@"     & integer'image(to_integer(unsigned(r_dbg_adr))) 
                                 severity failure;
                                 
                              if(r_dbg_ptr = c_first) then 
                                 v_state := e_IDLE;
                              else
                                 v_state := e_DBG_RD_C;
                              end if;  
                                  
            when others    => v_state := e_IDLE;                   
                                 
         end case;  
         
         case v_state is
            when e_SET_ADR_1ST => r_rd_adr_mode <= c_UPDATE;
            when e_SET_ADR_LAST_CPY_1ST => r_rd_adr_mode <= c_OUTPUT;
            when e_DBG_RD_C | 
                 e_DBG_RD_P |
                 e_DBG_WAIT |
                 e_DBG_VER => r_rd_adr_mode <= c_DBG;
            when others   => r_rd_adr_mode <= c_INP;                         
         end case;
         
         r_state  <= v_state;
  end if;
end if;     
   end process main;

  



end behavioral;      
      
      
