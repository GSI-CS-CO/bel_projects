library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.wbgenplus_pkg.all;
use work.genram_pkg.all;

package wb_read_access_tester_pkg is
  
    --| Component ---------------- wb_read_access_tester -----------------------------------|
    component wb_read_access_tester is
        Port(
          clk_sys_i   : std_logic;                            -- Clock input for sys domain
          rst_sys_n_i : std_logic;                            -- Reset input (active low) for sys domain
          
          data_i      : in  t_wishbone_slave_in;
          data_o      : out t_wishbone_slave_out
        
        );
    end component;
  
    constant c_wb_read_access_tester_sdb : t_sdb_device := (
    abi_class     => x"0000", -- undocumented device
    abi_ver_major => x"01",
    abi_ver_minor => x"00",
    wbd_endian    => c_sdb_endian_big,
    wbd_width     => x"7", -- 8/16/32-bit port granularity
    sdb_component => (
    addr_first    => x"0000000000000000",
    addr_last     => x"0000000000000007",
    product => (
    vendor_id     => x"0000000000000651",
    device_id     => x"2ead7e58",
    version       => x"00000001",
    date          => x"20250211",
    name          => "Wb read access test")));
  
  end wb_read_access_tester_pkg;
  package body wb_read_access_tester_pkg is
  end wb_read_access_tester_pkg;