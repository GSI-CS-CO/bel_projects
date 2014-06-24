library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.RandomPkg.all;
use work.genram_pkg.all;
use work.eb_internals_pkg.all;

-- WB time and flowcontrol independent op stream comparator
entity wb_op_diff is
   generic(
      g_fifo_depth   : natural := 500;
      g_max_lag  : natural := 50);
   port(
      -- Slave in
      clk_i          : in  std_logic;
      rst_n_i        : in  std_logic;
      
      full_o          : out std_logic;
      empty_o         : out std_logic;
      diff_o          : out std_logic;
      
      slave_A_i       : in t_wishbone_slave_in;
      slave_A_o       : in t_wishbone_slave_out;  -- pure listening device without own signalling
      
      slave_B_i       : in t_wishbone_slave_in;
      slave_B_o       : in t_wishbone_slave_out); --  pure listening device without own signalling
end wb_op_diff;

architecture rtl of wb_op_diff is
   
   subtype t_wb_ackerr is std_logic_vector(0 downto 0);
   type t_wb_ackerr_array is array(1 downto 0) of t_wb_ackerr;
   
   subtype t_wb_flat is std_logic_vector(1+1+1+4+32+32 -1 downto 0);
   type t_wb_flat_array is array(1 downto 0) of t_wb_flat; 
   
   
   signal fifo_push,
          fifo_empty,
          fifo_full,
          
          lag_data_push,
          lag_data_empty,
          lag_data_full,
          
          lag_ackerr_push,
          lag_ackerr_empty,
          lag_ackerr_full,
          lag_pop,
          lag_abort,
          lag_commit,
          r_err,
          r_cyc,
          err,
          cyc          : std_logic_vector(1 downto 0);
   
   signal fifo_q,
          fifo_d,
          lag_data_q,
          lag_data_d       : t_wb_flat_array; 
          
   signal lag_ackerr_q,
          lag_ackerr_d     : t_wb_ackerr_array;
               
   signal s_empty,
          fifo_pop         : std_logic;
   
   signal s_elemA,
          s_elemB : natural;
  
begin
  
  
  g: for I in 0 to 1 generate 
  
  lag_pop(i) <= not (lag_data_empty(i) or lag_ackerr_empty(i));
   
   
   
   
   
   
  lag_data : generic_sync_fifo
    generic map(
      g_data_width             => lag_data_d(i)'length, -- cyc, stb, we, sel, dat, adr
      g_size                   => g_max_lag,
      g_show_ahead             => true,
      g_with_empty             => true,
      g_with_full              => true,
      g_with_almost_full       => false,
      g_almost_full_threshold  => 6)
    port map(
      rst_n_i        => rst_n_i,
      clk_i          => clk_i,
      d_i            => lag_data_d(i),
      we_i           => lag_data_push(i),
      q_o            => lag_data_q(i),
      rd_i           => lag_pop(i),
      empty_o        => lag_data_empty(i),
      full_o         => lag_data_full(i),
      count_o        => open
      );
  
  lag_ackerr : generic_sync_fifo
    generic map(
      g_data_width             => 1, -- cyc, stb, we, sel, dat, adr
      g_size                   => g_max_lag,
      g_show_ahead             => true,
      g_with_empty             => true,
      g_with_full              => true,
      g_with_almost_full       => false,
      g_almost_full_threshold  => 6)
    port map(
      rst_n_i        => rst_n_i,
      clk_i          => clk_i,
      d_i            => lag_ackerr_d(i),
      we_i           => lag_ackerr_push(i),
      q_o            => lag_ackerr_q(i),
      rd_i           => lag_pop(i),
      empty_o        => lag_ackerr_empty(i),
      full_o         => lag_ackerr_full(i),
      count_o        => open
      );
  
  fifo_d(i)    <= lag_data_q(i);
  fifo_push(i) <= lag_ackerr_q(i)(0) when lag_ackerr_empty(i) = '0'
                  else '0';
  
  fifo_in : generic_sync_fifo
    generic map(
      g_data_width             => fifo_d(i)'length, -- cyc, stb, we, sel, dat, adr
      g_size                   => g_fifo_depth,
      g_show_ahead             => true,
      g_with_empty             => true,
      g_with_full              => true,
      g_with_almost_full       => false,
      g_almost_full_threshold  => 6)
    port map(
      rst_n_i        => rst_n_i,
      clk_i          => clk_i,
      d_i            => fifo_d(i),
      we_i           => fifo_push(i),
      q_o            => fifo_q(i),
      rd_i           => fifo_pop,
      empty_o        => fifo_empty(i),
      full_o         => fifo_full(i),
      count_o        => open
      );
      
  end generate;
          
  cyc(0) <= slave_A_i.cyc;
  cyc(1) <= slave_B_i.cyc;
  err(0) <= slave_A_o.err;
  err(1) <= slave_B_o.err;
  
  lag_data_d(0)       <= slave_A_i.cyc & slave_A_i.stb & slave_A_i.we & slave_A_i.sel & slave_A_i.dat & slave_A_i.adr;
  lag_data_push(0)    <= slave_A_i.cyc and slave_A_i.stb and not slave_A_o.stall;
  lag_ackerr_d(0)(0)  <= slave_A_o.ack;
  lag_ackerr_push(0)  <= slave_A_o.ack or slave_A_o.err;

  lag_data_d(1)       <= slave_B_i.cyc & slave_B_i.stb & slave_B_i.we & slave_B_i.sel & slave_B_i.dat & slave_B_i.adr;
  lag_data_push(1)    <= slave_B_i.cyc and slave_B_i.stb and not slave_B_o.stall;
  lag_ackerr_d(1)(0)  <= slave_B_o.ack;
  lag_ackerr_push(1)  <= slave_B_o.ack or slave_B_o.err;
  
  full_o <= lag_data_full(0) or lag_data_full(1) or lag_ackerr_full(0) or lag_ackerr_full(1) or fifo_full(0) or fifo_full(1);
  
  s_empty <= fifo_empty(0) or fifo_empty(1);
  empty_o <= fifo_empty(0) and fifo_empty(1);
  fifo_pop <= not s_empty;
  
  verify : process (clk_i, rst_n_i) is
   variable we_mask : std_logic_vector(fifo_d(0)'length-1 downto 0);
   variable push_A, push_B, pop, elem_A, elem_B : natural;
   
  begin
    if rst_n_i = '0' then
       diff_o <= '0';
       push_A := 0;
       push_B := 0;
       pop := 0;
       
    elsif rising_edge(clk_i) then
         if(fifo_push(0) = '1') then
            push_A := push_A + 1; 
         end if;
         
         if(fifo_push(1) = '1') then
            push_B := push_B + 1; 
         end if;
         
         if(fifo_pop = '1') then
            pop := pop + 1; 
         end if;
         s_elemA <= push_A - pop;
         s_elemB <= push_B - pop;
         
         diff_o <= '0';
         we_mask := (others => '1');
         we_mask(63 downto 32) := (others => ( fifo_q(0)(fifo_q(0)'left-2) and fifo_q(1)(fifo_q(1)'left-2) ) ); 
         if(fifo_empty(0) = '0' and fifo_empty(1) = '0')   then
            if( (fifo_q(0)(fifo_q(0)'left-2) /= fifo_q(1)(fifo_q(1)'left-2)) 
              or ((fifo_q(0) and we_mask) /=  (fifo_q(1) and we_mask))) then
               report "****** CORE DIFF IN/OUT!! @A" & integer'image(push_A - pop) & " @B" & integer'image(push_B - pop)  severity failure;
               diff_o <= '1';
            else
              -- report "****** CORE DIFF Packet OK" severity note;
            end if;
         end if;
      end if;
  end process; 
  
   
end;
   






