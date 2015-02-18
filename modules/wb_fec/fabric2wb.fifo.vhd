library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.wr_fabric_pkg.all;
use work.genram_pkg.all;

entity fabric2wb is
  port(
    clk_i           : in  std_logic;
    rst_n_i         : in  std_logic;
    dec_snk_i       : in  t_wrf_sink_in;
    dec_snk_o       : out t_wrf_sink_out;
    irq_fwb_o       : out std_logic;    
    wb_ram_master_i : in  t_wishbone_master_in;
    wb_ram_master_o : out t_wishbone_master_out;
    wb_fwb_slave_i  : in  t_wishbone_slave_in;
    wb_fwb_slave_o  : out t_wishbone_slave_out);
end fabric2wb;

architecture rtl of fabric2wb is

  signal s_wb_buffer  : t_wishbone_data;
  signal s_cycle      : integer := 0;
  
  constant g_r_data_width  : integer := 32;
  constant g_w_data_width  : integer := 16;
  constant g_size			   : integer  := 8;
  
  signal s_q        : std_logic_vector(g_r_data_width-1 downto 0);
  signal s_d        : std_logic_vector(g_w_data_width-1 downto 0);
  signal s_wr_req   : std_logic;
  signal s_rd_req   : std_logic;
  signal s_wr_empty : std_logic;
  signal s_rd_empty_d0 : std_logic := '0';
  signal s_wr_full  : std_logic;
  signal s_rd_empty : std_logic;
  signal s_rd_full  : std_logic;
  signal s_wr_count : std_logic_vector(f_log2_size(g_size)-1 downto 0);
  signal s_rd_count : std_logic_vector(f_log2_size(g_size)-2 downto 0);
  signal s_cnt      : integer;
  signal s_buff     : std_logic_vector(g_r_data_width-1 downto 0);

begin

   -- this wb slave doesn't supoort them
   dec_snk_o.rty <= '0';
   dec_snk_o.err <= '0';
  
  fabric_process : process(clk_i)
  begin

    if rising_edge(clk_i) then
      if rst_n_i = '0' then
         dec_snk_o.ack   <= '0';
         dec_snk_o.stall <= '1';
      else

        if s_wr_full = '0' then
          dec_snk_o.ack   <= dec_snk_i.cyc and dec_snk_i.stb;
          s_wr_req        <= dec_snk_i.cyc and dec_snk_i.stb;
          dec_snk_o.stall <= '0';
        else -- is full
          dec_snk_o.stall <= '1';
        end if;
        
        s_d <= dec_snk_i.dat;

      end if;
    end if;
  end process;

  ref_fifo: generic_async_fifo_mixed
  generic map (
    g_r_data_width  => 32,
    g_w_data_width  => 16,
    g_size          => 8,
    g_show_ahead    => true)

  port map(
    rst_n_i     => rst_n_i,
    clk_wr_i    => clk_i,
    d_i         => s_d,
    we_i        => s_wr_req,

    wr_empty_o  => s_wr_empty,
    wr_full_o   => s_wr_full,
    wr_count_o  => s_wr_count,

    clk_rd_i    => clk_i,
    q_o         => s_q,
    rd_i        => s_rd_req,

    rd_empty_o  => s_rd_empty,
    rd_full_o   => s_rd_full,
    rd_count_o  => s_rd_count
    );

  -- this wb slave doesn't supoort them
  wb_fwb_slave_o.int <= '0';
  wb_fwb_slave_o.rty <= '0';
  wb_fwb_slave_o.err <= '0';
  
  wb_process : process(clk_i)
  begin

    if rising_edge(clk_i) then
      if rst_n_i = '0' then
        wb_fwb_slave_o.ack   <= '0';
        wb_fwb_slave_o.stall <= '1';
      else
        if s_rd_empty = '0' then
          s_rd_req <= wb_fwb_slave_i.cyc and wb_fwb_slave_i.stb;
          wb_fwb_slave_o.ack <= wb_fwb_slave_i.cyc and wb_fwb_slave_i.stb;
          wb_fwb_slave_o.stall <= '0';
        else -- is empty
          wb_fwb_slave_o.stall <= '1';
        end if;

        wb_fwb_slave_o.dat <= s_q;

      end if;
    end if;
  end process;

  interrupt : process(clk_i)
  begin

    if rising_edge(clk_i) then
      if rst_n_i = '0' then
        s_rd_empty_d0 <= '0';
      else
        s_rd_empty_d0 <= s_rd_empty;
        irq_fwb_o <= s_rd_empty xor s_rd_empty_d0;
      end if;
    end if;
  end process;
end rtl;
