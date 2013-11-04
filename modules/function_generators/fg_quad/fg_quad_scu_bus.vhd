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
    diag_on_is_1:         integer range 0 to 1 := 0     -- if 1 then diagnosic information is generated during compilation
    );
  port (
    -- SCUB interface
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus 
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
    user_rd_active:     out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
    clk:                in    std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    nReset:             in    std_logic;
    Rd_Port:            out   std_logic_vector(15 downto 0);  -- output for all read sources of this macro
    Dtack:              out   std_logic;                       -- connect Dtack to SCUB-Macro
    -- fg_quad
    dreq:               out   std_logic;
    sw_out:             out   std_logic_vector(23 downto 0);  -- function generator output
    sw_strobe:          out   std_logic                       -- 
    );
end entity;


architecture fg_quad_scu_bus_arch of fg_quad_scu_bus is

  constant cntrl_reg_adr:   unsigned(15 downto 0) := Base_addr + x"0000";
  constant coeff_a_reg_adr: unsigned(15 downto 0) := Base_addr + x"0001";
  constant coeff_b_reg_adr: unsigned(15 downto 0) := Base_addr + x"0002";
  constant broad_start_adr: unsigned(15 downto 0) := Base_addr + x"0003";
  constant shift_a_reg_adr: unsigned(15 downto 0) := Base_addr + x"0004";
  constant shift_b_reg_adr: unsigned(15 downto 0) := Base_addr + x"0005";
  constant start_h_reg_adr: unsigned(15 downto 0) := Base_addr + x"0006";
  constant start_l_reg_adr: unsigned(15 downto 0) := Base_addr + x"0007";

  
  signal  fg_cntrl_reg:     std_logic_vector(15 downto 0);
  signal  fg_cntrl_rd_reg:  std_logic_vector(15 downto 0);
  signal  coeff_a_reg:      std_logic_vector(15 downto 0);
  signal  coeff_b_reg:      std_logic_vector(15 downto 0);
  signal  start_value_reg:  std_logic_vector(31 downto 0);
  signal  shift_a_reg:      std_logic_vector(15 downto 0);
  signal  shift_b_reg:      std_logic_vector(15 downto 0);

  signal  wr_fg_cntrl:      std_logic;
  signal  rd_fg_cntrl:      std_logic;
  signal  wr_coeff_a:       std_logic;
  signal  rd_coeff_a:       std_logic;
  signal  wr_coeff_b:       std_logic;
  signal  rd_coeff_b:       std_logic;
  signal  wr_start_value_h: std_logic;
  signal  rd_start_value_h: std_logic;
  signal  wr_start_value_l: std_logic;
  signal  rd_start_value_l: std_logic;
  signal  wr_shift_a:       std_logic;
  signal  rd_shift_a:       std_logic;
  signal  wr_shift_b:       std_logic;
  signal  rd_shift_b:       std_logic;
  signal  wr_brc_start:     std_logic;
  
  signal  set_out:          std_logic;
  signal  s_en:             std_logic;
  signal  fg_stopped:       std_logic;
  signal  fg_running:       std_logic;


begin
  quad_fg: fg_quad_datapath 
    generic map (
      ClK_in_hz => clk_in_hz)
    port map (
      data_a              => coeff_a_reg,
      data_b              => coeff_b_reg,
      clk                 => clk,
      nrst                => nReset,
      sync_rst            => fg_cntrl_reg(0),
      a_en                => wr_coeff_a,
      b_en                => wr_coeff_b,
      sync_start          => wr_brc_start,
      load_start          => wr_start_value_h,              -- when high word was written, load into datapath
      start_value         => start_value_reg(31 downto 0),
      s_en                => s_en,
      status_reg_changed  => wr_fg_cntrl,
      step_sel            => fg_cntrl_reg(12 downto 10),
      shift_b             => to_integer(unsigned(shift_b_reg(5 downto 0))),
      shift_a             => to_integer(unsigned(shift_a_reg(5 downto 0))),
      freq_sel            => fg_cntrl_reg(15 downto 13),
      dreq                => dreq,
      sw_out              => sw_out,
      sw_strobe           => sw_strobe,
      fg_stopped          => fg_stopped,
      fg_running          => fg_running       
    );
    
adr_decoder: process (clk, nReset)
  begin
    if nReset = '0' then
      wr_fg_cntrl       <= '0';
      rd_fg_cntrl       <= '0';
      wr_coeff_a        <= '0';
      rd_coeff_a        <= '0';
      wr_coeff_b        <= '0';
      rd_coeff_b        <= '0';
      wr_brc_start      <= '0';
      wr_start_value_h  <= '0';
      rd_start_value_h  <= '0';
      wr_start_value_l  <= '0';
      rd_start_value_l  <= '0';
      wr_shift_a        <= '0';
      wr_shift_b        <= '0';
      rd_shift_a        <= '0';
      rd_shift_b        <= '0';
      dtack             <= '0';
      
    elsif rising_edge(clk) then
      wr_fg_cntrl       <= '0';
      rd_fg_cntrl       <= '0';
      wr_coeff_a        <= '0';
      rd_coeff_a        <= '0';
      wr_coeff_b        <= '0';
      rd_coeff_b        <= '0';
      wr_brc_start      <= '0';
      wr_start_value_h  <= '0';
      rd_start_value_h  <= '0';
      wr_start_value_l  <= '0';
      rd_start_value_l  <= '0';
      wr_shift_a        <= '0';
      wr_shift_b        <= '0';
      rd_shift_a        <= '0';
      rd_shift_b        <= '0';
      dtack             <= '0';
    
      if Ext_Adr_Val = '1' then

        case unsigned(Adr_from_SCUB_LA) is

          when cntrl_reg_adr =>
            if Ext_Wr_active = '1' then
              wr_fg_cntrl <= '1';
              dtack       <= '1';
            end if;
            if Ext_Rd_active = '1' then
              rd_fg_cntrl <= '1';
              dtack       <= '1';
            end if;
            
          when broad_start_adr =>
            if Ext_Wr_active = '1' then
              wr_brc_start <= '1';
              dtack       <= '1';
            end if;
          
          when coeff_a_reg_adr =>
            if Ext_Wr_active = '1' then
              wr_coeff_a  <= '1';
              dtack       <= '1';
            elsif Ext_Rd_active = '1' then
              rd_coeff_a  <= '1';
              dtack       <= '1';
            end if;
            
          when coeff_b_reg_adr =>
            if Ext_Wr_active = '1' then
              wr_coeff_b  <= '1';
              dtack       <= '1';
            elsif Ext_Rd_active = '1' then
              rd_coeff_b  <= '1';
              dtack       <= '1';
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
          
          when shift_a_reg_adr =>
            if Ext_Wr_active = '1' then
              wr_shift_a  <= '1';
              dtack       <= '1';
            elsif Ext_Rd_active = '1' then
              rd_shift_a  <= '1';
              dtack       <= '1';
            end if;
            
          when shift_b_reg_adr =>
            if Ext_Wr_active = '1' then
              wr_shift_b  <= '1';
              dtack       <= '1';
            elsif Ext_Rd_active = '1' then
              rd_shift_b  <= '1';
              dtack       <= '1';
            end if;

          when others =>
            wr_fg_cntrl       <= '0';
            rd_fg_cntrl       <= '0';
            wr_coeff_a        <= '0';
            rd_coeff_a        <= '0';
            wr_coeff_b        <= '0';
            rd_coeff_b        <= '0';
            wr_brc_start      <= '0';
            wr_start_value_h  <= '0';
            rd_start_value_h  <= '0';
            wr_start_value_l  <= '0';
            rd_start_value_l  <= '0';
            wr_shift_a        <= '0';
            wr_shift_b        <= '0';
            rd_shift_a        <= '0';
            rd_shift_b        <= '0';
            dtack             <= '0';
        end case;
      end if;
    end if;
  end process adr_decoder;

-- fg_cntrl_reg(0)            : reset, 1 -> active 
-- fg_cntrl_reg(1)            : -
-- fg_cntrl_reg(2)            : running (ro)
-- fg_cntrl_reg(3)            : stopped (ro)
-- fg_cntrl_reg(9 downto 4)   : shift value b (wo)
-- fg_cntrl_reg(12 downto 10) : step value M (wo)
-- fg_cntrl_reg(15 downto 13) : add frequency select (wo)
cntrl_reg: process (clk, nReset, rd_fg_cntrl, fg_cntrl_reg, wr_fg_cntrl)
  variable reset_cnt: unsigned(1 downto 0) := "00";
begin
  if nReset = '0' or fg_cntrl_reg(0) = '1' then
    fg_cntrl_reg <= (others => '0');
    coeff_a_reg <= (others => '0');
    coeff_b_reg <= (others => '0');
    shift_a_reg <= (others => '0');
    shift_b_reg <= (others => '0');
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
    if wr_shift_a = '1' then
      shift_a_reg <= Data_from_SCUB_LA;
    end if;
    if wr_shift_b = '1' then
      shift_b_reg <= Data_from_SCUB_LA;
    end if;
    if wr_start_value_h = '1' then
      start_value_reg(31 downto 16) <= Data_from_SCUB_LA;
    end if;
    if wr_start_value_l = '1' then
      start_value_reg(15 downto 0) <= Data_from_SCUB_LA;
    end if;
    if  fg_cntrl_reg(0) = '1' then
      if reset_cnt < 3 then
        reset_cnt := reset_cnt + 1;
      else
        fg_cntrl_reg(0) <= '0';
        reset_cnt := "00";
      end if;
    end if;
  end if;
end process;

fg_cntrl_rd_reg <=  fg_cntrl_reg(15 downto 13) & fg_cntrl_reg(12 downto 10) &
                    fg_cntrl_reg(9 downto 4) & fg_stopped & fg_running & fg_cntrl_reg(1 downto 0);

user_rd_active <= rd_fg_cntrl or rd_coeff_a or rd_coeff_b or rd_start_value_h
                  or rd_start_value_l or rd_shift_a or rd_shift_b;

Rd_Port <=  fg_cntrl_rd_reg               when rd_fg_cntrl = '1' else
            coeff_a_reg                   when rd_coeff_a = '1' else
            coeff_b_reg                   when rd_coeff_b = '1' else
            start_value_reg(31 downto 16) when rd_start_value_h = '1' else
            start_value_reg(15 downto 0)  when rd_start_value_l = '1' else
            shift_a_reg                   when rd_shift_a = '1' else
            shift_b_reg                   when rd_shift_b = '1' else
                x"0000";

end architecture;
