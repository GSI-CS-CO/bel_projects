library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;
use work.scu_diob_pkg.all;
use work.fg_quad_pkg.all;

architecture diob_module1_arch of diob_module is
  
  alias A_nLED7: std_logic is vect_en(17);
  alias A_nLED6: std_logic is vect_en(19);
  alias A_nLED5: std_logic is vect_en(21);
  alias A_nLED4: std_logic is vect_en(23);
  alias A_nLED3: std_logic is vect_en(25);
  alias A_nLED2: std_logic is vect_en(27);
  alias A_nLED1: std_logic is vect_en(29);
  alias A_nLED0: std_logic is vect_en(31);
  
  alias TRIGGER_OUT2:     std_logic is vect_en(47);
  alias TRIGGER_OUT1:     std_logic is vect_en(49);
  alias A_nLED_EXT_TRG1:  std_logic is vect_en(51);
  alias A_nLED_EXT_TRG2:  std_logic is vect_en(53);
  alias A_nLED_INT_TRG1:  std_logic is vect_en(55);
  alias A_nLED_INT_TRG2:  std_logic is vect_en(57);
  
  alias DA1_DB0:    std_logic is vect_en(67);
  alias DA1_DB1:    std_logic is vect_en(69);
  alias DA1_DB2:    std_logic is vect_en(71);
  alias DA1_DB3:    std_logic is vect_en(73);
  alias DA1_DB4:    std_logic is vect_en(75);
  alias DA1_DB5:    std_logic is vect_en(77);
  alias DA1_DB6:    std_logic is vect_en(79);
  alias DA1_DB7:    std_logic is vect_en(81);
  alias DA1_DB8:    std_logic is vect_en(83);
  alias DA1_DB9:    std_logic is vect_en(85);
  alias DA1_DB10:   std_logic is vect_en(87);
  alias DA1_DB11:   std_logic is vect_en(89);
  alias DA1_DB12:   std_logic is vect_en(91);
  alias DA1_DB13:   std_logic is vect_en(93);
  alias DA1_DB14:   std_logic is vect_en(95);
  alias DA1_DB15:   std_logic is vect_en(97);
  alias DA1_CLK:    std_logic is vect_en(99);
  
  alias DA2_DB0:    std_logic is vect_en(101);
  alias DA2_DB1:    std_logic is vect_en(103);
  alias DA2_DB2:    std_logic is vect_en(105);
  alias DA2_DB3:    std_logic is vect_en(107);
  alias DA2_DB4:    std_logic is vect_en(109);
  alias DA2_DB5:    std_logic is vect_en(111);
  alias DA2_DB6:    std_logic is vect_en(113);
  alias DA2_DB7:    std_logic is vect_en(115);
  alias DA2_DB8:    std_logic is vect_en(117);
  alias DA2_DB9:    std_logic is vect_en(119);
  alias DA2_DB10:   std_logic is vect_en(121);
  alias DA2_DB11:   std_logic is vect_en(123);
  alias DA2_DB12:   std_logic is vect_en(125);
  alias DA2_DB13:   std_logic is vect_en(127);
  alias DA2_DB14:   std_logic is vect_en(129);
  alias DA2_DB15:   std_logic is vect_en(131);
  alias DA2_CLK:    std_logic is vect_en(133);
  
  signal FG_1_data_to_SCUB: std_logic_vector(15 downto 0);
  signal FG_2_data_to_SCUB: std_logic_vector(15 downto 0);
  signal FG_1_rd_active:    std_logic;
  signal FG_2_rd_active:    std_logic;
  signal FG_1_dtack:        std_logic;
  signal FG_2_dtack:        std_logic;
  signal da1_db:            std_logic_vector(15 downto 0);
  signal da2_db:            std_logic_vector(15 downto 0);
  signal Data_to_SCUB:      std_logic_vector(15 downto 0);
  
  signal user_dtack:        std_logic;
  signal user_rd_active:    std_logic;
  signal user_data_to_SCUB: std_logic_vector(15 downto 0);
  
  signal user_config:           std_logic_vector(15 downto 0);
  signal user_dac1_reg:         std_logic_vector(15 downto 0);
  signal user_dac2_reg:         std_logic_vector(15 downto 0);
  signal user_dac1_wr, da1_str: std_logic;
  signal user_dac2_wr, da2_str: std_logic;
  
  signal fg1_sw_out:        std_logic_vector(31 downto 0);
  signal fg2_sw_out:        std_logic_vector(31 downto 0);
  signal fg1_sw_strobe:     std_logic;
  signal fg2_sw_strobe:     std_logic;

begin
  vect_o(17) <= '1'; --A_nLED7
  vect_o(19) <= '1'; --A_nLED6
  vect_o(21) <= '1'; --A_nLED5
  vect_o(23) <= '1'; --A_nLED4
  vect_o(25) <= '1'; --A_nLED3
  vect_o(27) <= '1'; --A_nLED2
  vect_o(29) <= '1'; --A_nLED1
  vect_o(31) <= '1'; --A_nLED0
  
  vect_o(47) <= '1'; -- TRIGGER_OUT2
  vect_o(49) <= '1'; -- TRIGGER_OUT1
  
  vect_o(51) <= '1'; -- A_nLED_EXT_TRG1
  vect_o(53) <= '1'; -- A_nLED_EXT_TRG2
  vect_o(55) <= '1'; -- A_nLED_INT_TRG1
  vect_o(57) <= '1'; -- A_nLED_INT_TRG2
  
  vect_o(67) <= '1'; -- DA1_DB0 
  vect_o(69) <= '1'; -- DA1_DB1
  vect_o(71) <= '1'; -- DA1_DB2 
  vect_o(73) <= '1'; -- DA1_DB3
  vect_o(75) <= '1'; -- DA1_DB4 
  vect_o(77) <= '1'; -- DA1_DB5
  vect_o(79) <= '1'; -- DA1_DB6 
  vect_o(81) <= '1'; -- DA1_DB7
  vect_o(83) <= '1'; -- DA1_DB8 
  vect_o(85) <= '1'; -- DA1_DB9
  vect_o(87) <= '1'; -- DA1_DB10 
  vect_o(89) <= '1'; -- DA1_DB11
  vect_o(91) <= '1'; -- DA1_DB12
  vect_o(93) <= '1'; -- DA1_DB13
  vect_o(95) <= '1'; -- DA1_DB14
  vect_o(97) <= '1'; -- DA1_DB15
  vect_o(99) <= '1'; -- DA1_CLK
  
  vect_o(101) <= '1'; -- DA1_DB0 
  vect_o(103) <= '1'; -- DA1_DB1
  vect_o(105) <= '1'; -- DA1_DB2 
  vect_o(107) <= '1'; -- DA1_DB3
  vect_o(109) <= '1'; -- DA1_DB4 
  vect_o(111) <= '1'; -- DA1_DB5
  vect_o(113) <= '1'; -- DA1_DB6 
  vect_o(115) <= '1'; -- DA1_DB7
  vect_o(117) <= '1'; -- DA1_DB8 
  vect_o(119) <= '1'; -- DA1_DB9
  vect_o(121) <= '1'; -- DA1_DB10 
  vect_o(123) <= '1'; -- DA1_DB11
  vect_o(125) <= '1'; -- DA1_DB12
  vect_o(127) <= '1'; -- DA1_DB13
  vect_o(129) <= '1'; -- DA1_DB14
  vect_o(131) <= '1'; -- DA1_DB15
  vect_o(133) <= '1'; -- DA1_CLK
  
  vect_i(35) <= '1'; -- RESKNAPP
  vect_i(37) <= '1'; -- RESET1
  vect_i(43) <= '1'; -- EXT_TRIGGER_IN1
  vect_i(45) <= '1'; -- EXT_TRIGGER_IN2
  
  
  clk_divider: process (clk)
    variable count: unsigned(26 downto 0);
  begin
    if rising_edge(clk) then
        count := count - 1;
    end if;
    A_nLED1 <= count(26);
  end process;
  
  user_reg: entity work.user_io_reg(arch)
    generic map (
      base_addr => c_AW_Port1_Base_Addr,
      count     => 3
    )
    port map (
      clk                 => clk,
      rstn                => rstn,
      scu_slave_i         => scu_slave_i,
      scu_slave_o.dtack   => user_dtack,
      scu_slave_o.dat_val => user_rd_active,
      scu_slave_o.dat     => user_data_to_SCUB,
      
      user_in_reg(0)   => (others => 'X'),
      user_in_reg(1)   => (others => 'X'),
      user_in_reg(2)   => (others => 'X'),
      user_out_reg(0)  => user_config,
      user_out_reg(1)  => user_dac1_reg,
      user_out_reg(2)  => user_dac2_reg,
      
      user_io_wr(1)    => user_dac1_wr,
      user_io_wr(2)    => user_dac2_wr
    );
  
  fg_1: fg_quad_scu_bus
  generic map (
    Base_addr     => c_fg_1_Base_Addr,
    clk_in_hz     => clk_in_hz,
    diag_on_is_1  => 0 -- if 1 then diagnosic information is generated during compilation
    )
  port map (

    -- SCUB interface
    Adr_from_SCUB_LA  => scu_slave_i.adr,
    Data_from_SCUB_LA => scu_slave_i.dat,
    Ext_Adr_Val       => scu_slave_i.adr_val and en_i,
    Ext_Rd_active     => scu_slave_i.rd_act,
    Ext_Wr_active     => scu_slave_i.wr_act,
    clk               => clk,
    nReset            => rstn,
    Rd_Port           => FG_1_data_to_SCUB,     -- out, connect read sources (over multiplexer) to SCUB-Macro
    user_rd_active    => FG_1_rd_active,        -- '1' = read data available at 'Rd_Port'-output
    Dtack             => FG_1_dtack,            -- connect Dtack to SCUB-Macro
    irq               => irq_o(15),             -- shared irq
    tag               => tag_i,
    tag_valid         => tag_valid,
    ext_trigger       => '0',                   -- starts the ramping by external signal

    -- fg output
    sw_out            => fg1_sw_out,
    sw_strobe         => fg1_sw_strobe
  );
  
  DA1_DB15 <= da1_db(15);
  DA1_DB14 <= da1_db(14);
  DA1_DB13 <= da1_db(13);
  DA1_DB12 <= da1_db(12);
  DA1_DB11 <= da1_db(11);
  DA1_DB10 <= da1_db(10);
  DA1_DB9 <= da1_db(9);
  DA1_DB8 <= da1_db(8);
  DA1_DB7 <= da1_db(7);
  DA1_DB6 <= da1_db(6);
  DA1_DB5 <= da1_db(5);
  DA1_DB4 <= da1_db(4);
  DA1_DB3 <= da1_db(3);
  DA1_DB2 <= da1_db(2);
  DA1_DB1 <= da1_db(1);
  DA1_DB0 <= da1_db(0);

  fg_2: fg_quad_scu_bus
  generic map (
    Base_addr => c_fg_2_Base_Addr,
    clk_in_hz => clk_in_hz,
    diag_on_is_1 => 0 -- if 1 then diagnosic information is generated during compilation
    )
  port map (

    -- SCUB interface
    Adr_from_SCUB_LA  => scu_slave_i.adr,
    Data_from_SCUB_LA => scu_slave_i.dat,
    Ext_Adr_Val       => scu_slave_i.adr_val and en_i,
    Ext_Rd_active     => scu_slave_i.rd_act,
    Ext_Wr_active     => scu_slave_i.wr_act,
    clk               => clk,                   -- in, should be the same clk, used by SCU_Bus_Slave
    nReset            => rstn,                  -- in, '0' => resets the fg_1
    Rd_Port           => FG_2_data_to_SCUB,     -- out, connect read sources (over multiplexer) to SCUB-Macro
    user_rd_active    => FG_2_rd_active,        -- '1' = read data available at 'Rd_Port'-output
    Dtack             => FG_2_dtack,            -- connect Dtack to SCUB-Macro
    irq               => irq_o(14),             -- shared irq
    tag               => tag_i,
    tag_valid         => tag_valid,
    ext_trigger       => '0',                   -- starts the ramping by external signal

    -- fg output
    sw_out            => fg2_sw_out,
    sw_strobe         => fg2_sw_strobe
  );
  
  DA2_DB15 <= da2_db(15);
  DA2_DB14 <= da2_db(14);
  DA2_DB13 <= da2_db(13);
  DA2_DB12 <= da2_db(12);
  DA2_DB11 <= da2_db(11);
  DA2_DB10 <= da2_db(10);
  DA2_DB9 <= da2_db(9);
  DA2_DB8 <= da2_db(8);
  DA2_DB7 <= da2_db(7);
  DA2_DB6 <= da2_db(6);
  DA2_DB5 <= da2_db(5);
  DA2_DB4 <= da2_db(4);
  DA2_DB3 <= da2_db(3);
  DA2_DB2 <= da2_db(2);
  DA2_DB1 <= da2_db(1);
  DA2_DB0 <= da2_db(0);
  
  
  da1_db <= fg1_sw_out(30 downto 15) when user_config(4)  = '1' else user_dac1_reg;
  da2_db <= fg2_sw_out(30 downto 15) when user_config(4)  = '1' else user_dac2_reg;
  
  
  dac_str: process(clk)
  begin
    -- just delay the strobe one clk cycle
    if rising_edge(clk) then
      da1_str <= user_dac1_wr;
      da2_str <= user_dac2_wr;
    end if;
  end process dac_str;
  
  da1_CLK <= fg1_sw_strobe when user_config(4) = '1' else da1_str;
  da2_CLK <= fg2_sw_strobe when user_config(4) = '1' else da2_str;
  
  
  p_read_mux: process (
    FG_1_rd_active, FG_1_data_to_SCUB,
    FG_2_rd_active, FG_2_data_to_SCUB,
    user_rd_active, user_data_to_SCUB
  )
  variable sel: unsigned(2 downto 0);
  begin
    sel := user_rd_active & FG_2_rd_active & FG_1_rd_active;
    case sel IS
      when "001" => Data_to_SCUB <= FG_1_data_to_SCUB;
      when "010" => Data_to_SCUB <= FG_2_data_to_SCUB;
      when "100" => Data_to_SCUB <= user_data_to_SCUB;
      when others =>
        Data_to_SCUB <= x"0000";
    end case;
  end process p_read_mux;
  
  scu_slave_o.dat     <= Data_to_SCUB;
  scu_slave_o.dat_val <= FG_1_rd_active or FG_2_rd_active or user_rd_active;
  scu_slave_o.dtack   <= FG_1_dtack or FG_2_dtack or user_dtack;

  A_nLED7 <= '1'; --led off
  A_nLED6 <= '1'; --led off
  A_nLED5 <= '1'; --led off
  A_nLED4 <= '1'; --led off
  A_nLED3 <= not user_config(4); --fg mode
  A_nLED2 <= not user_config(4); --fg mode
  A_nLED0 <= '1'; --led off
  
  A_nLED_EXT_TRG1 <= '1'; -- led off
  A_nLED_EXT_TRG2 <= '1'; -- led off
  A_nLED_INT_TRG1 <= '1'; -- led off
  A_nLED_INT_TRG2 <= '1'; -- led off
end architecture diob_module1_arch;
