--wishbone dma testbench

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library std;
use std.textio.all;
use ieee.std_logic_textio.all;
library work;
use work.wishbone_pkg.all;


--Entity(empty)
entity wb_dma_tb is
end;

architecture behavioural of wb_dma_tb is
  --Testbench settings
  constant c_reset_time       : time  := 100 ns;
  constant c_sys_clock_cycle  : time  := 16 ns;

  --Testbench signals
  signal s_clk    : std_logic := '0';
  signal s_rstn   : std_logic := '0';

  signal s_start_desc        : std_logic := '0'; 
  signal s_read_init_address : t_wishbone_address := (others => '0');
  signal s_descriptor_active : std_logic := '0';

  

  -- Functions
  -- Function wb_stim -> Helper function to create a human-readable testbench
  function wb_stim(cyc : std_logic; stb : std_logic; we : std_logic; adr : t_wishbone_address; dat : t_wishbone_data) return t_wishbone_slave_in is
    variable v_setup : t_wishbone_slave_in;
  begin
    v_setup.cyc := cyc;
    v_setup.stb := stb;
    v_setup.we  := we;
    v_setup.adr := adr;
    v_setup.dat := dat;
    v_setup.sel := (others => '0'); -- Don't care
    return v_setup;
  end function wb_stim;
  
  -- Procedures
  -- Procedure wb_expect -> Check WB slave answer
  procedure wb_expect(msg : string; dat_from_slave : t_wishbone_data; compare_value : t_wishbone_data) is
  begin
    if (to_integer(unsigned(dat_from_slave)) = to_integer(unsigned(compare_value))) then
      report "Test passed: " & msg;
    else
      report "Test errored: " & msg;
      report "-> Info:  Answer from slave:          " & integer'image(to_integer(unsigned(dat_from_slave)));
      report "-> Error: Expected answer from slave: " & integer'image(to_integer(unsigned(compare_value)));
    end if;
  end procedure wb_expect;
  
-- component

  component wb_dma is
  port (
    clk_sys_i     : in std_logic;
    rstn_sys_i    : in std_logic;

    --master_o       : out t_wishbone_slave_out;
    --master_i       : in  t_wishbone_slave_in;

    -- test signals
    s_start_desc_i        : in std_logic;
    s_read_init_address_i : in t_wishbone_address;
    s_descriptor_active_i : in std_logic
  );
  end component;

begin------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

-- dut
  dut : wb_dma
  port map (
    clk_sys_i     => s_clk,
    rstn_sys_i    => s_rstn,

    --master_o      => open,
    --master_i      => c_DUMMY_WB_SLAVE_IN,

    -- test signals
    s_start_desc_i        => s_start_desc,
    s_read_init_address_i => s_read_init_address,
    s_descriptor_active_i => s_descriptor_active
  );



  -- Reset controller
  p_reset : process
  begin
    wait for c_reset_time;
    s_rstn <= '1';
  end process;



  test : process
  file vector_file  : text is "test_vectors.inp";
  variable vector   : line;
  variable good     : boolean;
  variable ch       : character;
  
  variable vstart   : std_logic;
  variable vaddr    : t_wishbone_address;
  variable vdesc_en : std_logic;
  begin

  while not endfile(vector_file) loop
    readline(vector_file, vector);
    -- skip lines that don't start with a std_logic value
    read(vector, vstart, good);
    next when not good;

    -- skip comma and space
    read(vector, ch);
    read(vector, ch);
    hread(vector, vaddr);
    read(vector, ch);
    read(vector, ch);
    read(vector, vdesc_en);

    wait for c_sys_clock_cycle/4;

    s_start_desc <= vstart;
    s_read_init_address <= vaddr;
    s_descriptor_active <= vdesc_en;

    wait for c_sys_clock_cycle/4;

    s_clk <= not (s_clk);

    wait for c_sys_clock_cycle/2;

    s_clk <= not (s_clk);

  end loop;

  assert false report "Test complete";
  wait for 1 ms;

  end process;

end architecture;
