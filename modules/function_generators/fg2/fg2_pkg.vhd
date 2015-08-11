library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

--library work;
--use work.wishbone_pkg.all;

package fg2_pkg is

component fg_scubus is
  generic (
    Base_addr:            unsigned(15 downto 0);
    clk_in_hz:            integer := 50_000_000);
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
end component;


component fg_accumulator is
  port(
    clk_i     : in  std_logic;
    rstn_i    : in  std_logic;
    dac_stb_i : in  std_logic;
    dac_val_o : out std_logic_vector(63 downto 0);
    start_i   : in  std_logic; -- 0=>1 means accumulator begins
    running_o : out std_logic;
    irq_o     : out std_logic; -- 0=>1 means refill
    full_o    : out std_logic; -- writes to abcdsn should fail
    stall_o   : out std_logic; -- writes are ignored
    a_o       : out std_logic_vector(63 downto 0);
    b_o       : out std_logic_vector(63 downto 0);
    c_o       : out std_logic_vector(63 downto 0);
    d_o       : out std_logic_vector(63 downto 0);
    n_o       : out std_logic_vector(15 downto 0);
    a_we_i    : in  std_logic_vector(7 downto 0);
    b_we_i    : in  std_logic_vector(7 downto 0);
    c_we_i    : in  std_logic_vector(7 downto 0);
    d_we_i    : in  std_logic_vector(7 downto 0);
    s_we_i    : in  std_logic_vector(1 downto 0);
    n_we_i    : in  std_logic_vector(1 downto 0); -- low byte (0) triggers readiness
    a_i       : in  std_logic_vector(63 downto 0);
    b_i       : in  std_logic_vector(63 downto 0);
    c_i       : in  std_logic_vector(63 downto 0);
    d_i       : in  std_logic_vector(63 downto 0);
    s_i       : in  std_logic_vector(15 downto 0); -- 0, d, c, b
    n_i       : in  std_logic_vector(15 downto 0));
end component;




end package fg2_pkg;
