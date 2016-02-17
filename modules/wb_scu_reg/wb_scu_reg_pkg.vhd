library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;

package wb_scu_reg_pkg is

component wb_scu_reg is
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
end component;

constant c_xwb_scu_reg : t_sdb_device := (
  abi_class     => x"0000", -- undocumented device
  abi_ver_major => x"01",
  abi_ver_minor => x"01",
  wbd_endian    => c_sdb_endian_big,
  wbd_width     => x"2", -- 8/16/32-bit port granularity
  sdb_component => (
  addr_first    => x"0000000000000000",
  addr_last     => x"00000000000001ff",
  product => (
  vendor_id     => x"0000000000000651", -- GSI
  device_id     => x"e2d13d04",
  version       => x"00000001",
  date          => x"20130322",
  name          => "WB_SCU_REG         ")));

end package;