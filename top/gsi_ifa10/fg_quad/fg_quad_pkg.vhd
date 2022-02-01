library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

--library work;
--use work.wishbone_pkg.all;

package fg_quad_pkg is

component fg_quad_datapath is
  generic (
    CLK_in_Hz:  integer := 125_000_000;
         ACU :  boolean := false
          );
  port (
  data_a:             in  std_logic_vector(15 downto 0);
  data_b:             in  std_logic_vector(15 downto 0);
  data_c:             in  std_logic_vector(31 downto 0);
  clk:                in  std_logic;
  sysclk:             in  std_logic;                      -- 12.5 MHz Backplane clock
  nrst:               in  std_logic;
  sync_rst:           in  std_logic;
  a_en:               in  std_logic;                      -- data register enable
  load_start:         in  std_logic;
  sync_start:         in  std_logic;
  step_sel:           in  std_logic_vector(2 downto 0);
  shift_a:            in  integer range 0 to 48;          -- shiftvalue coeff b
  shift_b:            in  integer range 0 to 48;          -- shiftvalue coeff b
  freq_sel:           in  std_logic_vector(2 downto 0);
  state_change_irq:   out std_logic;
  dreq:               out std_logic;
  ramp_sec_fin:       out std_logic;
  sw_out:             out std_logic_vector(31 downto 0);
  sw_strobe:          out std_logic;
  fg_is_running:      out std_logic);
end component;

component fg_quad_scu_bus is
  generic (
    Base_addr:          unsigned(15 downto 0) := X"0300";
    CLK_in_Hz:          integer := 100_000_000;
    diag_on_is_1:       integer range 0 to 1 := 0;        -- if 1 then diagnosic information is generated during compilation
    ACU:                boolean := false
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
    sysclk:             in    std_logic;                      -- 12.5 MHz Backplane clock
    nReset:             in    std_logic;
    tag:                in    std_logic_vector(31 downto 0);  -- 32Bit tag from timing
    tag_valid:          in    std_logic;                      -- tag valid
    ext_trigger:        in    std_logic;                      -- external trigger for ramp start
    Rd_Port:            out   std_logic_vector(15 downto 0);  -- output for all read sources of this macro
    Dtack:              out   std_logic;                      -- connect Dtack to SCUB-Macro
    -- fg_quad
    irq:                out   std_logic;
    sw_out:             out   std_logic_vector(31 downto 0);  -- function generator output
    sw_strobe:          out   std_logic
    );
end component fg_quad_scu_bus;

end package fg_quad_pkg;
