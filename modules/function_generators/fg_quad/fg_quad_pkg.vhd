library IEEE;
use IEEE.STD_LOGIC_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

package fg_quad_pkg is

component fg_quad_datapath is
  generic (
    CLK_in_Hz:  integer := 125_000_000);
  port (
  data_a:             in  std_logic_vector(15 downto 0);
  data_b:             in  std_logic_vector(15 downto 0);
  data_c:             in  std_logic_vector(31 downto 0);
  clk:                in  std_logic;
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
    diag_on_is_1:       integer range 0 to 1 := 0         -- if 1 then diagnosic information is generated during compilation
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

component wb_fg_quad is
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
end component wb_fg_quad;

constant c_wb_fg_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"4", -- 32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000000000ff",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"863e07f0",
    version       => x"00000001",
    date          => x"20140730",
    name          => "WB_FG_QUAD         ")));

constant c_fg_irq_ctrl_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"01",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"00000000000000ff",
    product => (
    vendor_id     => x"0000000000000651", -- GSI
    device_id     => x"9602eb71",
    version       => x"00000001",
    date          => x"20140730",
    name          => "IRQ_MASTER_CTRL    ")));

component wbmstr_core is 
port    (clk_i          : in  std_logic;   -- clock
         rst_n_i        : in  std_logic;   -- reset, active LO
         --msi if
         irq_master_o   : out t_wishbone_master_out;  -- Wishbone msi irq interface
         irq_master_i   : in  t_wishbone_master_in;
         --config 
         -- we assume these as stable, they won't be synced!			
         wb_dst         : in  std_logic_vector(31 downto 0);
         wb_msg         : in  std_logic_vector(31 downto 0);

         strobe         : in std_logic
);
end component;
end package fg_quad_pkg;
