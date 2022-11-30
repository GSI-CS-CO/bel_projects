library ieee;
use ieee.std_logic_1164.all;
use work.wishbone_pkg.all;

package simbridge_pkg is

   constant c_simbridge_msi : t_sdb_msi := (
     wbd_endian    => c_sdb_endian_big,
     wbd_width     => x"7", -- 8/16/32-bit port granularity
     sdb_component => (
     addr_first    => x"0000000000000000",
     addr_last     => x"000000000000ffff",
     product => (
     vendor_id     => x"0000000000000651", -- GSI
     device_id     => x"2ba55191",
     version       => x"00000002",
     date          => x"20200831",
     name          => "Simulat.=>WB bridge")));

      --eb_slave_control(master_o_cyc,master_o_stb,master_o_we,master_o_adr,master_o_dat,
      --                 master_i_ack,master_i_err,master_i_rty,master_i_stall_master_i_dat);

  procedure eb_simbridge_init(stop_unitl_connected : in integer; sdb_adr, msi_addr_first, msi_addr_last : in integer);
  attribute foreign of eb_simbridge_init: procedure is "VHPIDIRECT eb_simbridge_init";

  procedure eb_simbridge_master_out(cyc, stb, we : out  std_logic; adr, dat , sel, end_cyc: out integer);
  attribute foreign of eb_simbridge_master_out : procedure is "VHPIDIRECT eb_simbridge_master_out";

  procedure eb_simbridge_master_in(ack, err, rty, stall : in std_logic; dat : in integer; end_cyc : out integer);
  attribute foreign of eb_simbridge_master_in : procedure is "VHPIDIRECT eb_simbridge_master_in";

  procedure eb_simbridge_msi_slave_out(ack, err, rty, stall : out std_logic; dat : out integer);
  attribute foreign of eb_simbridge_msi_slave_out : procedure is "VHPIDIRECT eb_simbridge_msi_slave_out";

  procedure eb_simbridge_msi_slave_in(cyc, stb, we : in  std_logic; adr, dat , sel: in integer);
  attribute foreign of eb_simbridge_msi_slave_in : procedure is "VHPIDIRECT eb_simbridge_msi_slave_in";

end package;

package body simbridge_pkg is

  procedure eb_simbridge_init(stop_unitl_connected : in integer; sdb_adr, msi_addr_first, msi_addr_last : in integer) is 
  begin
    assert false report "VHPI" severity failure;
  end procedure;

  procedure eb_simbridge_master_out(cyc, stb, we : out  std_logic; adr, dat , sel, end_cyc: out integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

  procedure eb_simbridge_master_in(ack, err, rty, stall : in std_logic; dat : in integer; end_cyc : out integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

  procedure eb_simbridge_msi_slave_out(ack, err, rty, stall : out std_logic; dat : out integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

  procedure eb_simbridge_msi_slave_in(cyc, stb, we : in  std_logic; adr, dat , sel: in integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;


end package body;