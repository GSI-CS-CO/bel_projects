library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;

entity wb_scu_reg is
  generic (
    Base_addr:      unsigned(15 downto 0);
    size:           integer;
    g_init_file:    string
  );
  port (
    clk_sys_i : in std_logic;
    rst_n_i   : in std_logic;

    -- Wishbone
    slave_i : in  t_wishbone_slave_in;
    slave_o : out t_wishbone_slave_out;
    
    -- SCU bus
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus 
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
    user_rd_active:     out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB:       out   std_logic_vector(15 downto 0);  -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:      out   std_logic                       -- connect Dtack to SCUB-Macro
  );
end entity;

architecture wb_scu_reg_arch of wb_scu_reg is

  signal rd_active:         std_logic;
  signal dtack:             std_logic;
  signal rd_active_dly:     std_logic;
  signal dtack_dly:         std_logic;
  constant scubus_width :   integer := 16;
  constant wishbone_width : integer := 32;
  signal s_adr_a :          std_logic_vector(15 downto 0);
  signal s_scub_sel:        std_logic_vector(3 downto 0);
  signal s_qa_o:            std_logic_vector(31 downto 0);
  
  signal wrpulse:           std_logic;
  signal pulse1:            std_logic;
  signal pulse2:            std_logic;
  
  
  
begin
  
  -- only 16Bit writes from scubus
  with Adr_from_SCUB_LA(0) select s_scub_sel <=
                                 "1100" when '0',
                                 "0011" when '1';
  with Adr_from_SCUB_LA(0) select Data_to_SCUB <=
                                  s_qa_o(31 downto 16) when '0',
                                  s_qa_o(15 downto 0) when '1';
                          
  s_adr_a <= std_logic_vector(unsigned(Adr_from_SCUB_LA) - Base_addr);
  
  write_pulse: process(clk_sys_i, Ext_Wr_active, Ext_Adr_Val)
  begin
    if rising_edge(clk_sys_i) then
      if rst_n_i = '0' then
        pulse1 <= '0';
        pulse2 <= '0';
      else
        pulse1 <= Ext_Wr_active;
        pulse2 <= pulse1;
      end if;
    end if;
  end process;

  wrpulse <= pulse1 and not pulse2;
  
  dpram:  generic_dpram
  generic map (
    g_data_width        => 32,
    g_size              => size * 2,
    g_with_byte_enable  => true,
    g_dual_clock        => false,
    g_addr_conflict_resolution => "read_first",
    g_init_file           => g_init_file)
  port map (
    -- port A
    clka_i  => clk_sys_i,
    bwea_i  => s_scub_sel,  
    wea_i   => wrpulse and Ext_Adr_Val,
    aa_i    => '0' & s_adr_a(f_log2_size(size*2)-1 downto 1),
    da_i    => Data_from_SCUB_LA & Data_from_SCUB_LA,
    qa_o    => s_qa_o,
    
    -- port B
    clkb_i  => clk_sys_i,
    bweb_i  => slave_i.sel,
    web_i   => slave_i.we and slave_i.stb and slave_i.cyc,
    ab_i    => "00" & slave_i.adr(f_log2_size(size*2)-1 downto 2),
    db_i    => slave_i.dat,
    qb_o    => slave_o.dat);

  wb_regs: process (clk_sys_i, rst_n_i, slave_i)
  begin
    if rising_edge(clk_sys_i) then
      -- This is an easy solution for a device that never stalls:
      slave_o.ack <= slave_i.cyc and slave_i.stb; 
    end if;  
  end process;
  
  slave_o.int <= '0';
  slave_o.err <= '0';
  slave_o.rty <= '0';
  slave_o.stall <= '0'; -- always ready
  
  
  
  adr_decoder: process (clk_sys_i)
  begin
    if rising_edge(clk_sys_i) then
      dtack         <= '0';
      rd_active     <= '0';
      
      if Ext_Adr_Val = '1' then
        if unsigned(Adr_from_SCUB_LA) >= Base_addr and unsigned(Adr_from_SCUB_LA) < Base_addr + size then
          if Ext_Rd_active = '1' then
            dtack        <= '1';
            rd_active    <= '1';
          elsif Ext_Wr_active = '1' then
            dtack        <= '1';
          end if;
        end if;
      end if;
      
    end if;
  end process adr_decoder;
  
  rd_delay: process(clk_sys_i, rd_active, dtack)
  begin
    if rising_edge(clk_sys_i) then
      rd_active_dly <= rd_active;
      dtack_dly <= dtack;
    end if;
  end process;
  
  
  user_rd_active <= rd_active_dly;
  Dtack_to_SCUB <= dtack_dly;

end architecture;