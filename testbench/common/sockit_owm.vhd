library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

use work.wishbone_pkg.all;

package interface_sockit_owm is
    --VL_IN(__SYM__interrupt,31,0);

  function to_integer(logic_value : std_logic) return integer;
  function to_std_logic(integer_value: integer) return std_logic;

  function interface_sockit_owm_init return integer;
  attribute foreign of interface_sockit_owm_init : function is "VHPIDIRECT interface_sockit_owm_init";


  procedure interface_sockit_owm_clk(idx : integer; clk : integer);
  attribute foreign of interface_sockit_owm_clk : procedure is "VHPIDIRECT interface_sockit_owm_clk";

  procedure interface_sockit_owm_rst(idx : integer; rst : integer);
  attribute foreign of interface_sockit_owm_rst : procedure is "VHPIDIRECT interface_sockit_owm_rst";

  procedure interface_sockit_owm_bus_ren(idx : integer; bus_ren : integer);
  attribute foreign of interface_sockit_owm_bus_ren : procedure is "VHPIDIRECT interface_sockit_owm_bus_ren";

  procedure interface_sockit_owm_bus_wen(idx : integer; bus_wen : integer);
  attribute foreign of interface_sockit_owm_bus_wen : procedure is "VHPIDIRECT interface_sockit_owm_bus_wen";

  procedure interface_sockit_owm_bus_adr(idx : integer; bus_adr : integer);
  attribute foreign of interface_sockit_owm_bus_adr : procedure is "VHPIDIRECT interface_sockit_owm_bus_adr";

  function interface_sockit_owm_bus_irq(idx : integer) return integer;
  attribute foreign of interface_sockit_owm_bus_irq : function is "VHPIDIRECT interface_sockit_owm_bus_irq";

  function interface_sockit_owm_owr_p(idx : integer) return integer;
  attribute foreign of interface_sockit_owm_owr_p : function is "VHPIDIRECT interface_sockit_owm_owr_p";

  function interface_sockit_owm_owr_e(idx : integer) return integer;
  attribute foreign of interface_sockit_owm_owr_e : function is "VHPIDIRECT interface_sockit_owm_owr_e";

  procedure interface_sockit_owm_owr_i(idx : integer; owr_i : integer);
  attribute foreign of interface_sockit_owm_owr_i : procedure is "VHPIDIRECT interface_sockit_owm_owr_i";

  procedure interface_sockit_owm_bus_wdt(idx : integer; bus_wdt : integer);
  attribute foreign of interface_sockit_owm_bus_wdt : procedure is "VHPIDIRECT interface_sockit_owm_bus_wdt";

  function interface_sockit_owm_bus_rdt(idx : integer) return integer;
  attribute foreign of interface_sockit_owm_bus_rdt : function is "VHPIDIRECT interface_sockit_owm_bus_rdt";

end package;

package body interface_sockit_owm is
  function to_integer(logic_value: std_logic) return integer is
  begin
    if logic_value = '1' then 
      return 1;
    else 
      return 0;
    end if;
  end function;

  function to_std_logic(integer_value: integer) return std_logic is
  begin
    if integer_value = 0 then 
      return '0';
    else 
      return '1';
    end if;
  end function;


  function interface_sockit_owm_init return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;


  procedure interface_sockit_owm_clk(idx : integer; clk : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;
  procedure interface_sockit_owm_rst(idx : integer; rst : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;



  procedure interface_sockit_owm_bus_ren(idx : integer; bus_ren : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;
  procedure interface_sockit_owm_bus_wen(idx : integer; bus_wen : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;
  procedure interface_sockit_owm_bus_adr(idx : integer; bus_adr : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;



  function interface_sockit_owm_bus_irq(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;
  function interface_sockit_owm_owr_p(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;
  function interface_sockit_owm_owr_e(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  procedure interface_sockit_owm_owr_i(idx : integer; owr_i : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;
  procedure interface_sockit_owm_bus_wdt(idx : integer; bus_wdt : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

  function interface_sockit_owm_bus_rdt(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

end package body;


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

use work.wishbone_pkg.all;
use work.interface_sockit_owm.all;


entity sockit_owm is
  generic(
    BTP_N : string;
    BTP_O : string;
    OWN   : integer;
    CDR_N : integer;
    CDR_O : integer);
  port(
    clk     : in  std_logic;
    rst     : in  std_logic;
    bus_ren : in  std_logic;
    bus_wen : in  std_logic;
    bus_adr : in  std_logic_vector(0 downto 0);
    bus_wdt : in  std_logic_vector(31 downto 0);
    bus_rdt : out std_logic_vector(31 downto 0);
    bus_irq : out std_logic;
    owr_p   : out std_logic_vector(OWN-1 downto 0);
    owr_e   : out std_logic_vector(OWN-1 downto 0);
    owr_i   : in  std_logic_vector(OWN-1 downto 0)
    );
end entity;

architecture verilator_interface of sockit_owm is
  signal sockit_owm_idx  : integer := interface_sockit_owm_init;
begin

  process begin

    wait until rising_edge(clk);
    interface_sockit_owm_rst(sockit_owm_idx, to_integer(rst));
    interface_sockit_owm_clk(sockit_owm_idx, to_integer(clk));

    -- transport outputs from verilator side to our vhdl interface
    bus_rdt <= std_logic_vector(to_signed(interface_sockit_owm_bus_rdt(sockit_owm_idx)-integer'low, 32));
    bus_irq <= to_std_logic(interface_sockit_owm_bus_irq(sockit_owm_idx));
    owr_p   <= std_logic_vector(to_signed(interface_sockit_owm_owr_p(sockit_owm_idx), 1));
    owr_e   <= std_logic_vector(to_signed(interface_sockit_owm_owr_e(sockit_owm_idx), 1));

    wait until falling_edge(clk);
    -- transport inputs from our vhdl interface to the verilator side
    interface_sockit_owm_bus_ren(sockit_owm_idx, to_integer(bus_ren));
    --if bus_wen = '1' then
    --  report "VHDL: interface_sockit_owm_bus_wen " & integer'image(sockit_owm_idx) & " " &
    --        integer'image(to_integer(bus_wen));
    --end if;
    interface_sockit_owm_bus_wen(sockit_owm_idx, to_integer(bus_wen));
    interface_sockit_owm_bus_adr(sockit_owm_idx, to_integer(signed(bus_adr)));
    interface_sockit_owm_bus_wdt(sockit_owm_idx, to_integer(signed(bus_wdt)+integer'low));
    interface_sockit_owm_owr_i  (sockit_owm_idx, to_integer(signed(owr_i)));

    interface_sockit_owm_clk(sockit_owm_idx, to_integer(clk));

  end process;

end architecture;
