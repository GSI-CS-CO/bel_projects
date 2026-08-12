library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;


entity xwb_2port_fifo_wrapper is
  generic (
    g_fifo_size  : natural := 64;
    g_show_ahead : boolean := true;
    g_with_almost_empty  : boolean := true;
    g_with_almost_full   : boolean := true;
    g_almost_empty_thres : integer := 4; 
    g_almost_full_thres  : integer := 60
  );
  port (
    clk_i        : in  std_logic;
    rstn_i       : in  std_logic;
    slave0_i      : in  t_wishbone_slave_in;
    slave0_o      : out t_wishbone_slave_out;
    slave1_i      : in  t_wishbone_slave_in;
    slave1_o      : out t_wishbone_slave_out
  );
end entity xwb_2port_fifo_wrapper;

architecture rtl of xwb_2port_fifo_wrapper is

  signal fifo_din              : std_logic_vector(31 downto 0) := (others => '0');
  signal fifo_dout             : std_logic_vector(31 downto 0);
  signal we_flag               : std_logic := '0';
  signal rd_flag               : std_logic := '0';
  signal empty_flag            : std_logic := '1';
  signal full_flag             : std_logic := '0';
  signal fifo_count            : std_logic_vector(f_log2_size(g_fifo_size)-1 downto 0);
  signal empty_thres           : std_logic ;
  signal full_thres            : std_logic ;
  signal almost_empty_flag     : std_logic := '0';
  signal almost_full_flag      : std_logic := '0';

  signal wb0_ack               : std_logic := '0';
  signal wb0_stall             : std_logic := '0';
  signal wb0_err               : std_logic := '0';
  signal wb1_ack               : std_logic := '0';
  signal wb1_stall             : std_logic := '0';
  signal wb1_err               : std_logic := '0';

begin

  wrapped_fifo : generic_sync_fifo
    generic map (
      g_data_width             => 32,
      g_size                   => g_fifo_size,      --passing value from outside entity to here
      g_show_ahead             => g_show_ahead,
      g_with_empty             => true,
      g_with_full              => true,    --enable full flag
      g_with_almost_empty      => g_with_almost_empty,
      g_with_almost_full       => g_with_almost_full,
      g_with_count             => true,
      g_almost_empty_threshold => g_almost_empty_thres, 
      g_almost_full_threshold  => g_almost_full_thres
    )
    port map (
      rst_n_i         => rstn_i,
      clk_i           => clk_i,
      d_i             => fifo_din,
      we_i            => we_flag,
      q_o             => fifo_dout,
      rd_i            => rd_flag,
      empty_o         => empty_flag,
      full_o          => full_flag,
      almost_empty_o  => almost_empty_flag,
      almost_full_o   => almost_full_flag,
      count_o         => fifo_count
    );

    slave0_o.rty    <= '0';
    slave0_o.ack    <= wb0_ack;
    slave0_o.err    <= wb0_err;
    slave0_o.stall  <= wb0_stall;

    slave1_o.rty    <= '0';
    slave1_o.ack    <= wb1_ack;
    slave1_o.err    <= wb1_err;
    slave1_o.stall  <= wb1_stall;

    p_single_access : process (clk_i, rstn_i)
      variable status_reg : std_logic_vector(31 downto 0);  --32 bit for wishbone protocol
    begin
      if(rstn_i = '0') then
        wb0_ack      <= '0';
        wb0_err      <= '0';
        wb0_stall    <= '0';
        slave0_o.dat <= (others => '0');
        wb1_ack      <= '0';
        wb1_err      <= '0';
        wb1_stall    <= '0';
        slave1_o.dat <= (others => '0');

        we_flag      <= '0';
        rd_flag      <= '0';

      elsif rising_edge(clk_i) then
        wb0_ack      <= '0';
        wb0_err      <= '0';
        wb0_stall    <= '0';
        wb1_ack      <= '0';
        wb1_err      <= '0';
        wb1_stall    <= '0';
        we_flag      <= '0';
        rd_flag      <= '0';

        if(slave0_i.cyc = '1' and slave0_i.stb = '1' and wb0_ack = '0') then    --slave0: write only
          --access to fifo data register Base Address + Offset 0x00
          if(slave0_i.adr = x"60000000" and slave0_i.we = '1') then     --need to redefine
            if (full_flag = '1') then
              wb0_stall <= '1';
            else
              we_flag  <= '1';
              fifo_din <= slave0_i.dat;
              wb0_ack   <= '1';
            end if;
          end if;
            
        elsif(slave1_i.cyc = '1' and slave1_i.stb = '1' and wb1_ack = '0') then    --slave1: read only
          if(slave1_i.adr = x"65000000" and slave1_i.we = '0') then   
            if (empty_flag = '1') then
              wb1_err      <= '1'; -- Please generate error, check WB spec if you have to raise ACK too
            else
              rd_flag      <= '1';
              slave1_o.dat <= fifo_dout;
              wb1_ack      <= '1';
            end if;
          --access to fifo status register Base Address + Offset 0x04
          elsif(slave1_i.adr = x"65000004" and slave1_i.we = '0') then
            wb1_ack <= '1';
            status_reg := (others => '0');

            status_reg(31 downto 31-fifo_count'length +1) := fifo_count;
            status_reg(5) := almost_empty_flag;
            status_reg(4) := almost_full_flag;
            status_reg(3) := empty_flag;
            status_reg(2) := full_flag;
            status_reg(1) := rd_flag;
            status_reg(0) := we_flag;
            slave1_o.dat <= status_reg;

          end if;
        end if;
      end if;
    end process p_single_access;
  end architecture rtl;

            
             


          
              





    

