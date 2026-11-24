library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;


entity xwb_fifo_wrapper is
  generic (
    g_fifo_size  : natural := 64;
    g_show_ahead : boolean := true;
    g_with_almost_empty  : boolean := true;
    g_with_almost_full   : boolean := true;
    g_almost_empty_thres : integer := 1; 
    g_almost_full_thres  : integer := 60
  );
  port (
    clk_i        : in  std_logic;
    rstn_i       : in  std_logic;
    slave_i      : in  t_wishbone_slave_in;
    slave_o      : out t_wishbone_slave_out
  );
end entity xwb_fifo_wrapper;

architecture rtl of xwb_fifo_wrapper is

  signal fifo_din       : std_logic_vector(31 downto 0) := (others => '0');
  signal fifo_dout      : std_logic_vector(31 downto 0);
  signal we_flag        : std_logic := '0';
  signal rd_flag        : std_logic := '0';
  signal empty_flag     : std_logic := '1';
  signal full_flag      : std_logic := '0';
  signal fifo_count     : std_logic_vector(f_log2_size(g_fifo_size)-1 downto 0);
  signal empty_thres    : std_logic ;
  signal full_thres     : std_logic ;
  signal wb_ack         : std_logic := '0';
  signal wb_stall       : std_logic := '0';
  signal almost_empty_flag      : std_logic := '0';
  signal almost_full_flag      : std_logic := '1';

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

    slave_o.err    <= '0';
    slave_o.rty    <= '0';
    slave_o.ack    <= wb_ack;
    slave_o.stall  <= wb_stall;

    p_single_access : process (clk_i, rstn_i)
      variable status_reg : std_logic_vector(31 downto 0);  --32 bit for wishbone protocol
    begin
      if(rstn_i = '0') then
        wb_ack      <= '0';
        wb_stall    <= '0';
        slave_o.dat <= (others => '0');
        we_flag     <= '0';
        rd_flag     <= '0';

      elsif rising_edge(clk_i) then
        wb_ack      <= '0';
        wb_stall    <= '0';
        we_flag     <= '0';
        rd_flag     <= '0';

        if(slave_i.cyc = '1' and slave_i.stb = '1' and wb_ack = '0') then
          --access to fifo data register Base Address + Offset 0x00
          if(slave_i.adr(2) = '0') then     --need to redefine
            --write, data in
            if (slave_i.we = '1') then
              if (full_flag = '1') then
                wb_stall <= '1';
              else
                we_flag  <= '1';
                fifo_din <= slave_i.dat;
                wb_ack   <= '1';
              end if;
            --read, data out
            else
              if (empty_flag = '1') then
                slave_o.dat <= x"DEADBEEF";
                wb_ack      <= '1'; -- Please generate error, check WB spec if you have to raise ACK too
              else
                rd_flag     <= '1';
                slave_o.dat <= fifo_dout;
                wb_ack      <= '1';
              end if;
            end if;
          --access to fifo status register Base Address + Offset 0x04
          elsif(slave_i.adr(2) = '1' and slave_i.we = '0') then
            wb_ack <= '1';
            status_reg := (others => '0');
            status_reg(31 downto 31-fifo_count'length +1) := fifo_count;
            status_reg(3) := empty_flag;
            status_reg(2) := full_flag;
            status_reg(1) := rd_flag;
            status_reg(0) := we_flag;
            slave_o.dat <= status_reg;

          end if;
        end if;
      end if;
    end process p_single_access;
  end architecture rtl;

            
             


          
              





    

