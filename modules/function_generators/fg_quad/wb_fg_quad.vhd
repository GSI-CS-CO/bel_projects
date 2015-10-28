library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.wishbone_pkg.all;
use work.wb_irq_pkg.all;
use work.fg_quad_pkg.all;

entity wb_fg_quad is
  generic (
            Clk_in_hz   : integer := 62_500_000);
  port (
        clk_i         : std_logic;
        rst_n_i       : std_logic;
  
        -- slave wb port to fg_quad
        fg_slave_i    : in t_wishbone_slave_in;
        fg_slave_o    : out t_wishbone_slave_out;
        
        -- master interface for output from fg to the scu_bus
        fg_mst_i      : in t_wishbone_master_in;
        fg_mst_o      : out t_wishbone_master_out;

        -- control interface for msi generator
        ctrl_irq_i    : in t_wishbone_slave_in;
        ctrl_irq_o    : out t_wishbone_slave_out;

        -- master interface for msi generator
        irq_mst_i     : in t_wishbone_master_in;
        irq_mst_o     : out t_wishbone_master_out);
end entity;

architecture wb_fg_quad_arch of wb_fg_quad is
  signal  fg_dreq:            std_logic_vector(0 downto 0);

  constant cntrl_reg_adr:     unsigned(7 downto 0) := x"00";
  constant coeff_a_reg_adr:   unsigned(7 downto 0) := x"04";
  constant coeff_b_reg_adr:   unsigned(7 downto 0) := x"08";
  constant broad_start_adr:   unsigned(7 downto 0) := x"0c";
  constant shift_a_reg_adr:   unsigned(7 downto 0) := x"10";
  constant shift_b_reg_adr:   unsigned(7 downto 0) := x"14";
  constant start_reg_adr:     unsigned(7 downto 0) := x"18";
  constant ramp_cnt_reg_adr:  unsigned(7 downto 0) := x"1c";
  constant sw_dst_reg_adr:    unsigned(7 downto 0) := x"20";

  signal  fg_cntrl_reg:     std_logic_vector(15 downto 0);
  signal  fg_cntrl_rd_reg:  std_logic_vector(15 downto 0);
  signal  coeff_a_reg:      std_logic_vector(15 downto 0);
  signal  coeff_b_reg:      std_logic_vector(15 downto 0);
  signal  start_value_reg:  std_logic_vector(31 downto 0);
  signal  shift_a_reg:      std_logic_vector(15 downto 0);
  signal  shift_b_reg:      std_logic_vector(15 downto 0);
  signal  ramp_cnt_reg:     unsigned(15 downto 0);
  signal  sw_dst_reg:       std_logic_vector(31 downto 0);

  signal wr_brc_start:      std_logic;
  signal wr_coeff_a:        std_logic;
  signal wr_start_value:    std_logic;
  signal fg_is_running:     std_logic;
  signal ramp_sec_fin:      std_logic;
  signal sw_out:            std_logic_vector(31 downto 0);
  signal sw_strobe:         std_logic;
  
  signal s_msg         : t_wishbone_data_array(0 downto 0);
  signal s_dst         : t_wishbone_address_array(0 downto 0);
  
  signal state_change_irq:  std_logic;


begin
  ---------------------------------------------------------------------------------------
  -- wb interface fo fg
  ---------------------------------------------------------------------------------------

  wb_regs: process (clk_i, rst_n_i, fg_slave_i)
    variable reset_cnt: unsigned(1 downto 0) := "00";
  begin

      
    if rising_edge(clk_i) then
      -- device never stalls
      fg_slave_o.ack <= fg_slave_i.cyc and fg_slave_i.stb;
      fg_slave_o.dat <= (others => '0');

      if rst_n_i = '0' or fg_cntrl_reg(0) = '1' then
        fg_cntrl_reg    <= x"03f0";         -- mark virtual fg number as unconfigured
        fg_cntrl_rd_reg <= (others => '0');
        coeff_a_reg     <= (others => '0');
        coeff_b_reg     <= (others => '0');
        start_value_reg <= (others => '0');
        shift_a_reg     <= (others => '0');
        shift_b_reg     <= (others => '0');
        ramp_cnt_reg    <= (others => '0');
        wr_brc_start    <= '0';
        wr_coeff_a      <= '0';
        wr_start_value  <= '0';
      else
        wr_brc_start    <= '0';
        wr_coeff_a      <= '0';
        wr_start_value  <= '0';
      
        -- write to registers
        if fg_slave_i.cyc = '1' and fg_slave_i.stb = '1' and fg_slave_i.we = '1' then
          if fg_slave_i.sel = x"f" then
            case unsigned(fg_slave_i.adr(7 downto 0)) is
              when cntrl_reg_adr    => fg_cntrl_reg <= fg_slave_i.dat(15 downto 0);
              when coeff_a_reg_adr  => coeff_a_reg  <= fg_slave_i.dat(15 downto 0);
                                       wr_coeff_a   <= '1';
              when coeff_b_reg_adr  => coeff_b_reg  <= fg_slave_i.dat(15 downto 0);
              when shift_a_reg_adr  => shift_a_reg  <= fg_slave_i.dat(15 downto 0);
              when shift_b_reg_adr  => shift_b_reg  <= fg_slave_i.dat(15 downto 0);
              when broad_start_adr  => wr_brc_start <= '1';
              when start_reg_adr  => start_value_reg <= fg_slave_i.dat(31 downto 0);
                                     wr_start_value <= '1';
              when sw_dst_reg_adr => sw_dst_reg <= fg_slave_i.dat(31 downto 0); 
              when others => null;
            end case;
          end if;  
        end if;
        --read from registers
        case unsigned(fg_slave_i.adr(7 downto 0)) is
          when cntrl_reg_adr    => fg_slave_o.dat <= x"0000" & fg_cntrl_reg(15 downto 4) & '0' & fg_is_running & fg_cntrl_reg(1 downto 0); 
          when coeff_a_reg_adr  => fg_slave_o.dat <= x"0000" & coeff_a_reg; 
          when coeff_b_reg_adr  => fg_slave_o.dat <= x"0000" & coeff_b_reg; 
          when start_reg_adr    => fg_slave_o.dat <= start_value_reg; 
          when shift_a_reg_adr  => fg_slave_o.dat <= x"0000" & shift_a_reg;
          when shift_b_reg_adr  => fg_slave_o.dat <= x"0000" & shift_b_reg;
          when sw_dst_reg_adr   => fg_slave_o.dat <= sw_dst_reg;
          when others => null;
        end case;
      end if;
     
      -- reset logic for fg sync reset
      if fg_cntrl_reg(0) = '1' then
        if reset_cnt < 3 then
          reset_cnt := reset_cnt + 1;
        else
          fg_cntrl_reg(0) <= '0';
          reset_cnt := "00";
        end if;
      end if;
    end if;

  end process;
  
  fg_slave_o.int <= '0';
  fg_slave_o.err <= '0';
  fg_slave_o.rty <= '0';
  fg_slave_o.stall <= '0'; -- always ready
  
  -----------------------------------------------------------------------------------------
  -- instance of fg_quad_datapath
  -----------------------------------------------------------------------------------------
  
  quad_fg: fg_quad_datapath 
    generic map (
      ClK_in_hz => 62_500_000)
    port map (
      data_a              => coeff_a_reg,
      data_b              => coeff_b_reg,
      data_c              => start_value_reg(31 downto 0),
      clk                 => clk_i,
      nrst                => rst_n_i,
      sync_rst            => fg_cntrl_reg(0),
      a_en                => wr_coeff_a,
      sync_start          => wr_brc_start and fg_cntrl_reg(1),    -- start at write to broadcast reg
      load_start          => wr_start_value,                      -- load into datapath
      step_sel            => fg_cntrl_reg(12 downto 10),
      shift_b             => to_integer(unsigned(shift_b_reg(5 downto 0))),
      shift_a             => to_integer(unsigned(shift_a_reg(5 downto 0))),
      freq_sel            => fg_cntrl_reg(15 downto 13),
      state_change_irq    => state_change_irq,
      dreq                => fg_dreq(0),
      ramp_sec_fin        => ramp_sec_fin,
      sw_out              => sw_out,
      sw_strobe           => sw_strobe,
      fg_is_running       => fg_is_running       
    );
    
  -----------------------------------------------------------------------------------------
  -- wb master for generating dreq msi
  -----------------------------------------------------------------------------------------
  fg_irq_master: wb_irq_master
    generic map (
      g_channels => 1,
      g_round_rb => true,
      g_det_edge => true) 
    port map (
      clk_i   => clk_i,
      rst_n_i => rst_n_i,
    
      -- msi if
      irq_master_o => irq_mst_o,
      irq_master_i => irq_mst_i,

      -- ctrl if
      ctrl_slave_o => ctrl_irq_o,
      ctrl_slave_i => ctrl_irq_i,

      -- irq lines
      irq_i        => fg_dreq); 
  
  ------------------------------------------------------------------------------------------
  -- wb master for converting sw_out to wishbone
  ------------------------------------------------------------------------------------------
  fg_sw_master: wbmstr_core
    port map (
      clk_i   => clk_i,
      rst_n_i => rst_n_i,
    
      -- msi if
      irq_master_o => fg_mst_o,
      irq_master_i => fg_mst_i,
      
      -- config
      wb_dst => sw_dst_reg,
      wb_msg => sw_out,

      strobe => sw_strobe); 

end architecture;
