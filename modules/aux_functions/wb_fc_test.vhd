library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.RandomPkg.all;
use work.genram_pkg.all;

entity wb_fc_test is
   generic(
      g_seed          : natural := 14897;
      
      g_max_stall     : natural := 20;
      g_prob_stall    : natural := 35;
      
      g_ack_gen       : boolean := false;
      g_max_lag       : natural := 3;
      g_prob_lag      : natural := 0;
      g_prob_err      : natural := 0);
   port(
      -- Slave in
      clk_i    : in  std_logic;
      rst_n_i  : in  std_logic;
      slave_i        : in  t_wishbone_slave_in;
      slave_o        : out t_wishbone_slave_out;
      -- Master out
      master_i       : in  t_wishbone_master_in;
      master_o       : out t_wishbone_master_out);
end wb_fc_test;

architecture rtl of wb_fc_test is
   
   
   
   signal   r_cnt_stall    : natural;
   signal   s_stall,
            s_rnd_stall    : std_logic;
   constant c_stall_weight : integer_vector( 0 to 1) := ( 100 - g_prob_stall, g_prob_stall );
   
   signal r_cnt_lag        : integer;
   signal s_ackerr, r_err  : std_logic;
   constant c_lag_weight   : integer_vector( 0 to 1) := ( 100 - g_prob_lag, g_prob_lag );
   constant c_err_weight   : integer_vector( 0 to 1) := ( 100 - g_prob_err, g_prob_err );

   signal fifo_push,
          fifo_pop,
          fifo_empty,
          fifo_full        : std_logic;
   signal fifo_q           : std_logic_vector(0 downto 0);       

begin
    
   master_o.cyc   <= slave_i.cyc;
   master_o.we    <= slave_i.we;
   master_o.sel   <= slave_i.sel;
   master_o.adr   <= slave_i.adr;
   master_o.dat   <= slave_i.dat;
   
   s_rnd_stall    <= '1' when r_cnt_stall > 0
                else '0'; 
   
   slave_o.stall  <= s_stall;
  
   rnd_stall : process(clk_i, rst_n_i)
   variable RV : RandomPType;
   begin
      if (rst_n_i = '0') then
         r_cnt_stall <= 0;
      end if;
      if (rising_edge(clk_i)) then
        if r_cnt_stall = 0 then
           r_cnt_stall <= RV.DistInt( c_stall_weight ) * RV.RandInt(1, g_max_stall);
        else
           r_cnt_stall <= r_cnt_stall - 1;
        end if;
      end if;
   end process;
   
   bside: if(g_ack_gen = false) generate
     slave_o.dat    <= master_i.dat;
     slave_o.ack    <= master_i.ack;
     slave_o.err    <= master_i.err;
     s_stall        <= master_i.stall or  s_rnd_stall;
     master_o.stb   <= slave_i.stb and (not s_rnd_stall or master_i.stall);
   end generate;
   
   gen: if(g_ack_gen = true) generate
      
      slave_o.dat    <= x"DEADBEEF";
      slave_o.ack    <= fifo_pop and not r_err;
      slave_o.err    <= fifo_pop and r_err;
      s_stall        <= s_rnd_stall;
      master_o.stb   <= slave_i.stb and not s_rnd_stall;
      
      s_ackerr <= '1' when r_cnt_lag = 0
             else '0';  
      
      
      
      rnd_lag : process(clk_i, rst_n_i)
      variable RV : RandomPType;
      begin
         if (rst_n_i = '0') then
            r_cnt_lag <= 0;
         end if;
         if (rising_edge(clk_i)) then
           if(slave_i.cyc and slave_i.stb and not s_stall) then
            r_cnt_lag <= RV.DistInt( c_lag_weight ) * RV.RandInt(0, g_max_lag);
           elsif(r_cnt_lag > 0) then
              r_cnt_lag <= r_cnt_lag - 1;
           end if;
         end if;
      end process;
      
      A_fifo : generic_sync_fifo
    generic map(
      g_data_width             => 1, 
      g_size                   => g_max_lag,
      g_show_ahead             => false,
      g_with_empty             => true,
      g_with_full              => true,
      g_with_almost_full       => true,
      g_almost_full_threshold  => g_max_lag-1)
    port map(
      rst_n_i        => rst_n_i,
      clk_i          => clk_i,
      d_i            => "1",
      we_i           => fifo_push,
      q_o            => fifo_q,
      rd_i           => fifo_pop,
      empty_o        => fifo_empty,
      almost_full_o  => fifo_full,
      count_o        => open
      );

      fifo_push <= slave_i.cyc and slave_i.stb and not s_stall;
      fifo_pop  <= (s_ackerr and not fifo_empty) or fifo_full;
   
      rnd_err : process(clk_i, rst_n_i)
      variable RV : RandomPType;
      variable v_err : std_logic_vector (0 downto 0);
      begin
         if (rising_edge(clk_i)) then
           v_err := RV.DistSlv( c_err_weight, 1);
           r_err <= v_err(0);
         end if;
      end process;
   end generate;  
   
end;
   






