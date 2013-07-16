library ieee;
USE ieee.std_logic_1164.all;
use ieee.math_real.all;
use ieee.numeric_std.all;

library work;
use work.fg_quad_pkg.all;

entity fg_quad_scu_bus is
    generic (
    Base_addr:            unsigned(15 downto 0);
    clk_in_hz:            integer := 50_000_000;        -- 50Mhz
    diag_on_is_1:         integer range 0 to 1 := 0);   -- if 1 then diagnosic information is generated during compilation
  port (
    clk:            in std_logic;
    nrst:           in std_logic;
    
    -- fg_quad
    sw_out:             out std_logic_vector(23 downto 0);

    -- SCUB interface
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


architecture fg_quad_scu_bus_arch of fg_quad_scu_bus is
  constant cntrl_reg_adr:   unsigned(15 downto 0) := Base_addr + x"0000";
  constant coeff_a_reg_adr: unsigned(15 downto 0) := Base_addr + x"0001";
  constant coeff_b_reg_adr: unsigned(15 downto 0) := Base_addr + x"0002";
  constant start_h_reg_adr: unsigned(15 downto 0) := Base_addr + x"0003";
  constant start_l_reg_adr: unsigned(15 downto 0) := Base_addr + x"0004";

  
  signal fg_cntrl_reg:    std_logic_vector(15 downto 0);
  signal coeff_a_reg:     std_logic_vector(15 downto 0);
  signal coeff_b_reg:     std_logic_vector(15 downto 0);
  signal start_value_reg: std_logic_vector(31 downto 0);
  
  signal set_out :        std_logic;

begin
  quad_fg: fg_quad_datapath 
    generic map (
      ClK_in_hz => clk_in_hz)
    port map (
      data_a              => coeff_a_reg,
      data_b              => coeff_b_reg,
      clk                 => clk,
      nrst                => nrst,
      a_en                => wr_coeff_a,
      b_en                => wr_coeff_b,
      load_start          => fg_cntrl_reg(1),
      s_en                => t_s_en,
      status_reg_changed  => wr_fg_cntrl,
      step_sel            => fg_cntrl_reg(10 downto 7),
      b_shift             => to_integer(unsigned(fg_cntrl_reg(6 downto 4))),
      freq_sel            => fg_cntrl_reg(13 dwonto 11),
      sw_out              => sw_out,
      set_out             => set_out    
    );
    
  adr_decoder: process (clk, nrst)
  begin
    if nrst = '0' then
      wr_fg_cntrl       <= '0';
      rd_fg_cntrl       <= '0';
      wr_coeff_a        <= '0';
      rd_coeff_a        <= '0';
      wr_coeff_b        <= '0';
      rd_coeff_b        <= '0';
      wr_start_value_h  <= '0';
      rd_start_value_h  <= '0';
      wr_start_value_l  <= '0';
      rd_start_value_l  <= '0';
      dtack             <= '0';
    elsif rising_edge(clk) then
      wr_fg_cntrl       <= '0';
      rd_fg_cntrl       <= '0';
      wr_coeff_a        <= '0';
      rd_coeff_a        <= '0';
      wr_coeff_b        <= '0';
      rd_coeff_b        <= '0';
      wr_start_value_h  <= '0';
      rd_start_value_h  <= '0';
      wr_start_value_l  <= '0';
      rd_start_value_l  <= '0';
      dtack             <= '0';
    
    if Ext_Adr_Val = '1' then
      case cntrl_reg_adr is
        when cntrl_reg_adr =>
          if Ext_Wr_active = '1' then
            wr_fg_cntrl  <= '1';
            dtack         <= '1';
          end if;
          if Ext_Rd_active = '1' then
            rd_fg_cntrl  <= '1';
            dtack         <= '1';
          end if;
        
        when coeff_a_reg_adr =>
          if Ext_Wr_active = '1' then
            wr_coeff_a  <= '1';
            dtack       <= '1';
          elsif Ext_Rd_active = '1' then
            rd_coeff_a <= '1';
            dtack        <= '1';
          end if;
          
        when coeff_b_reg_adr =>
          if Ext_Wr_active = '1' then
            wr_coeff_b  <= '1';
            dtack       <= '1';
          elsif Ext_Rd_active = '1' then
            rd_coeff_b <= '1';
            dtack        <= '1';
          end if;
          
        when start_h_reg_adr =>
          if Ext_Wr_active = '1' then
            wr_start_value_h  <= '1';
            dtack             <= '1';
          elsif Ext_Rd_active = '1' then
            rd_start_value_h  <= '1';
            dtack             <= '1';
          end if;
          
        when start_l_reg_adr =>
          if Ext_Wr_active = '1' then
            wr_start_value_l  <= '1';
            dtack             <= '1';
          elsif Ext_Rd_active = '1' then
            rd_start_value_l  <= '1';
            dtack             <= '1';
          end if;
          
        
        when others =>
          wr_fg_cntrl       <= '0';
          rd_fg_cntrl       <= '0';
          wr_coeff_a        <= '0';
          rd_coeff_a        <= '0';
          wr_coeff_b        <= '0';
          rd_coeff_b        <= '0';
          wr_start_value_h  <= '0';
          rd_start_value_h  <= '0';
          wr_start_value_l  <= '0';
          rd_start_value_l  <= '0';
          dtack             <= '0';
      end case;
    end if;
  end if;
end process adr_decoder;

-- fg_cntrl_reg(0)            : reset, 1 -> active
-- fg_cntrl_reg(1)            : load start
-- fg_cntrl_reg(6 downto 4)   : shift value b
-- fg_cntrl_reg(10 downto 7)  : step value M
-- fg_cntrl_reg(13 dwonto 11) : add frequency select
cntrl_reg: process (clk, nrst, rd_fg_cntrl, wr_fg_cntrl)
  variable reset_cnt: unsigned(1 downto 0) := "00";
begin
  if nrst = '0' then
    fg_cntrl_reg <= (others => '0');
    coeff_a_reg <= (others => '0');
    coeff_b_reg <= (others => '0');
    start_value_reg <= (others => '0');
    reset_cnt := "00";
  elsif rising_edge(clk) then
    if wr_fg_cntrl = '1' then
      fg_cntrl_reg <= Data_from_SCUB_LA;
    end if;
    if wr_coeff_a = '1' then
      coeff_a_reg <= Data_from_SCUB_LA;
    end if;
    if wr_coeff_b = '1' then
      coeff_b_reg <= Data_from_SCUB_LA;
    end if;
    if wr_start_value_h = '1' then
      start_value_reg(31 downto 16) <= Data_from_SCUB_LA;
    end if;
    if wr_start_value_l = '1' then
      start_value_reg(15 downto 0) <= Data_from_SCUB_LA;
    end if;
    if  adc_cntrl_reg(0) = '1' then
      if reset_cnt < 3 then
        reset_cnt := reset_cnt + 1;
      else
        adc_cntrl_reg(0) <= '0';
        reset_cnt := "00";
      end if;
    end if;
  end if;
end process;

adc_cntrl_rd_reg <=  adc_cntrl_reg;

  
end architecture;