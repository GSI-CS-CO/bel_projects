library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;


entity xwb_2port_fifo is
  generic (
    g_fifo_size              : natural := 64;
    g_show_ahead             : boolean := true;

    g_with_rd_empty          : boolean := true;
    g_with_rd_almost_empty   : boolean := true;
    g_with_rd_count          : boolean := true;

    g_with_wr_full           : boolean := true;       --enable full flag
    g_with_wr_almost_full    : boolean := true;
    g_with_wr_count          : boolean := true;

    g_almost_empty_thres     : integer := 4;
    g_almost_full_thres      : integer := 60;
    g_async_adapter          : boolean := false
  );
  port (
    clk_w_i        : in  std_logic;
    rstn_i         : in  std_logic;
    slave_w_i      : in  t_wishbone_slave_in;
    slave_w_o      : out t_wishbone_slave_out;

    clk_r_i        : in  std_logic := '0';
    slave_r_i      : in  t_wishbone_slave_in;
    slave_r_o      : out t_wishbone_slave_out
  );
end entity xwb_2port_fifo;

architecture rtl of xwb_2port_fifo is

  --global signal for shared w/r process
  signal rd_clk                : std_logic;
  signal fifo_din              : std_logic_vector(31 downto 0) := (others => '0');
  signal fifo_dout             : std_logic_vector(31 downto 0);
  signal we_flag               : std_logic := '0';
  signal rd_flag               : std_logic := '0';
  signal empty_flag            : std_logic := '1';
  signal full_flag             : std_logic := '0';

  signal wr_status_reg         : std_logic_vector(31 downto 0);
  signal rd_status_reg         : std_logic_vector(31 downto 0);
  signal boot_status_reg       : std_logic_vector(31 downto 0) := (others => '0');

  signal wb_w_ack              : std_logic := '0';
  signal wb_w_stall            : std_logic := '0';
  signal wb_w_err              : std_logic := '0';

  signal wb_r_ack              : std_logic := '0';
  signal wb_r_stall            : std_logic := '0';
  signal wb_r_err              : std_logic := '0';

begin

  slave_w_o.rty    <= '0';
  slave_w_o.ack    <= wb_w_ack;
  slave_w_o.err    <= wb_w_err;
  slave_w_o.stall  <= wb_w_stall;

  slave_r_o.rty    <= '0';
  slave_r_o.ack    <= wb_r_ack;
  slave_r_o.err    <= wb_r_err;
  slave_r_o.stall  <= wb_r_stall;


  gen_sync : if not g_async_adapter generate
    signal fifo_count            : std_logic_vector(f_log2_size(g_fifo_size)-1 downto 0);
    signal almost_empty_flag     : std_logic := '0';
    signal almost_full_flag      : std_logic := '0';
    signal s_empty_flag          : std_logic := '1';
    signal s_full_flag           : std_logic := '0';

  begin
    --local signal should connect to global signal, global <= local
    rd_clk     <= clk_w_i;
    full_flag  <= s_full_flag;
    empty_flag <= s_empty_flag;

    wrapped_fifo : generic_sync_fifo
      generic map (
        g_data_width             => 32,
        g_size                   => g_fifo_size,      --passing value from outside entity to here
        g_show_ahead             => g_show_ahead,
        g_with_empty             => g_with_rd_empty,
        g_with_full              => g_with_wr_full,             --enable full flag
        g_with_almost_empty      => g_with_rd_almost_empty,
        g_with_almost_full       => g_with_wr_almost_full,
        g_with_count             => true,
        g_almost_empty_threshold => g_almost_empty_thres,
        g_almost_full_threshold  => g_almost_full_thres
      )
      port map (
        rst_n_i         => rstn_i,
        clk_i           => clk_w_i,
        d_i             => fifo_din,
        we_i            => we_flag,
        q_o             => fifo_dout,
        rd_i            => rd_flag,
        empty_o         => s_empty_flag,
        full_o          => s_full_flag,
        almost_empty_o  => almost_empty_flag,
        almost_full_o   => almost_full_flag,
        count_o         => fifo_count
      );

      
    p_status : process(fifo_count, almost_empty_flag, almost_full_flag, s_empty_flag, s_full_flag, we_flag, rd_flag)
    begin
      wr_status_reg <= (others => '0');
      wr_status_reg(8+fifo_count'length -1 downto 8) <= fifo_count;
      wr_status_reg(2) <= almost_full_flag;
      wr_status_reg(1) <= s_full_flag;
      wr_status_reg(0) <= we_flag;
      rd_status_reg <= (others => '0');
      rd_status_reg(8+fifo_count'length -1 downto 8) <= fifo_count;
      rd_status_reg(2) <= almost_empty_flag;
      rd_status_reg(1) <= s_empty_flag;
      rd_status_reg(0) <= rd_flag;
    end process;
  end generate gen_sync;

  gen_async : if g_async_adapter generate
    signal wr_full_flag              : std_logic := '0';
    signal wr_almost_full_flag       : std_logic := '0';
    signal rd_empty_flag             : std_logic := '1';
    signal rd_almost_empty_flag      : std_logic := '1';
    signal wr_fifo_count             : std_logic_vector(f_log2_size(g_fifo_size)-1 downto 0);
    signal rd_fifo_count             : std_logic_vector(f_log2_size(g_fifo_size)-1 downto 0);
  begin
    --global <= local
    rd_clk     <= clk_r_i;
    empty_flag <= rd_empty_flag;
    full_flag  <= wr_full_flag; 

    wrapped_async_fifo : generic_async_fifo
      generic map (
        g_data_width                => 32,
        g_size                      => g_fifo_size,      --passed value from entity to here
        g_show_ahead                => g_show_ahead,
        
        g_with_rd_empty             => g_with_rd_empty,
        g_with_rd_almost_empty      => g_with_rd_almost_empty,
        g_with_rd_count             => g_with_rd_count,

        g_with_wr_full              => g_with_wr_full,              --enable full flag
        g_with_wr_almost_full       => g_with_wr_almost_full,
        g_with_wr_count             => g_with_wr_count,

        g_almost_empty_threshold    => g_almost_empty_thres, 
        g_almost_full_threshold     => g_almost_full_thres
      )
      port map (
        rst_n_i            => rstn_i,

        clk_wr_i           => clk_w_i,
        d_i                => fifo_din,
        we_i               => we_flag,
        wr_full_o          => wr_full_flag,
        wr_almost_full_o   => wr_almost_full_flag,
        wr_count_o         => wr_fifo_count,
        
        clk_rd_i           => clk_r_i,
        q_o                => fifo_dout,
        rd_i               => rd_flag,
        rd_empty_o         => rd_empty_flag,
        rd_almost_empty_o  => rd_almost_empty_flag,
        rd_count_o         => rd_fifo_count
      );

    p_wr_status : process(wr_fifo_count, wr_almost_full_flag, wr_full_flag, we_flag)
    begin
      wr_status_reg <= (others => '0');
      --byte2
      wr_status_reg(8+wr_fifo_count'length -1 downto 8) <= wr_fifo_count; 
      --byte1
      wr_status_reg(2)  <= wr_almost_full_flag;
      wr_status_reg(1)  <= wr_full_flag;
      wr_status_reg(0)  <= we_flag;
    end process;
    
    p_rd_status : process(rd_fifo_count, rd_almost_empty_flag, rd_empty_flag, rd_flag)
    begin
      rd_status_reg <= (others => '0');
      --byte2
      rd_status_reg(8+rd_fifo_count'length -1 downto 8) <= rd_fifo_count; 
      --byte1
      rd_status_reg(2)  <= rd_almost_empty_flag;
      rd_status_reg(1)  <= rd_empty_flag;
      rd_status_reg(0)  <= rd_flag;
    end process;

  end generate gen_async;


  w_single_access : process (clk_w_i, rstn_i)
  begin
    if(rstn_i = '0') then
      wb_w_ack       <= '0';
      wb_w_err       <= '0';
      wb_w_stall     <= '0';
      slave_w_o.dat  <= (others => '0');
      we_flag        <= '0';

    elsif rising_edge(clk_w_i) then
      wb_w_ack       <= '0';
      wb_w_err       <= '0';
      wb_w_stall     <= '0';
      we_flag        <= '0';

      if(slave_w_i.cyc = '1' and slave_w_i.stb = '1' and wb_w_ack = '0') then
        --access to fifo data register Base Address + Offset 0x00
        if(slave_w_i.adr(3 downto 0) = x"0" and slave_w_i.we = '1') then
          if (full_flag = '1') then
            wb_w_stall <= '1';
          else
            we_flag    <= '1';
            fifo_din   <= slave_w_i.dat;
            wb_w_ack   <= '1';
          end if;
        elsif(slave_w_i.adr(3 downto 0) = x"4" and slave_w_i.we = '0') then
          wb_w_ack <= '1';
          slave_w_o.dat <= wr_status_reg;
        elsif(slave_w_i.adr(3 downto 0) = x"8" and slave_w_i.we = '1') then
          wb_w_ack <= '1';
          boot_status_reg <= slave_w_i.dat;
        else
          wb_w_err     <= '1';
        end if;
      end if;
    end if;
  end process w_single_access;

  r_single_access : process (rd_clk, rstn_i)
  begin
    if(rstn_i = '0') then
      wb_r_ack      <= '0';
      wb_r_err      <= '0';
      wb_r_stall    <= '0';
      slave_r_o.dat <= (others => '0');
      rd_flag       <= '0';

    elsif rising_edge(rd_clk) then
      wb_r_ack      <= '0';
      wb_r_err      <= '0';
      wb_r_stall    <= '0';
      rd_flag       <= '0';
      if(slave_r_i.cyc = '1' and slave_r_i.stb = '1' and wb_r_ack = '0') then    --slave1: read only
        if(slave_r_i.adr(3 downto 0) = x"0" and slave_r_i.we = '0') then   
          if (empty_flag = '1') then
            wb_r_err      <= '1'; -- Please generate error, check WB spec if you have to raise ACK too
          else
            rd_flag       <= '1';
            slave_r_o.dat <= fifo_dout;
            wb_r_ack      <= '1';
          end if;
        --access to fifo status register Base Address + Offset 0x04
        elsif(slave_r_i.adr(3 downto 0) = x"4" and slave_r_i.we = '0') then
          wb_r_ack <= '1';
          slave_r_o.dat <= rd_status_reg;
        --access to boot status register Base Address + Offset 0x08
        elsif(slave_r_i.adr(3 downto 0) = x"8" and slave_r_i.we = '0') then
          wb_r_ack <= '1';
          slave_r_o.dat <= boot_status_reg;
        else
          wb_r_err <= '1';
        end if;
      end if;
    end if;
  end process r_single_access;



  end architecture rtl;