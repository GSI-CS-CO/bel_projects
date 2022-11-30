library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.simbridge_pkg.all;

entity simbridge is
  generic (
      g_sdb_address    : t_wishbone_address;
      g_simbridge_msi  : t_sdb_msi := c_simbridge_msi
    );
  port (
  clk_i       : in  std_logic;
  rstn_i      : in  std_logic;
  master_o    : out t_wishbone_master_out;
  master_i    : in  t_wishbone_master_in;
  msi_slave_i : in  t_wishbone_slave_in;
  msi_slave_o : out t_wishbone_slave_out
    );
end entity;

architecture simulation of simbridge is
  signal counter : integer;
  signal end_cyc : boolean;
begin
  process
    variable master_o_cyc,master_o_stb,master_o_we  : std_logic;
    variable master_o_dat,master_o_adr,master_o_sel,master_o_end_cyc : integer := 0;
    variable master_i_ack,master_i_err,master_i_rty,master_i_stall : std_logic;
    variable master_i_dat,master_i_end_cyc                         : integer := 0;
    variable msi_slave_i_cyc,msi_slave_i_stb,msi_slave_i_we  : std_logic;
    variable msi_slave_i_dat,msi_slave_i_adr,msi_slave_i_sel : integer;
    variable msi_slave_o_ack,msi_slave_o_err,msi_slave_o_rty,msi_slave_o_stall : std_logic;
    variable msi_slave_o_dat                                                   : integer;
    constant stop_until_connected : integer := 1;
    variable stb_cnt : integer := 0;
    variable end_cycle : boolean := false;
    variable end_cycle_request : boolean := false;
  begin
    master_o.cyc <= '0';
    master_o.stb <= '0';
    master_o.we  <= '0';
    master_o.adr <= (others => '0');
    master_o.dat <= (others => '0');
    master_o.sel <= (others => '0');

    wait until rising_edge(rstn_i);
    wait until rising_edge(clk_i);
    eb_simbridge_init(stop_until_connected, 
                      to_integer(signed(g_sdb_address)), 
                      to_integer(signed(g_simbridge_msi.sdb_component.addr_first)), 
                      to_integer(signed(g_simbridge_msi.sdb_component.addr_last)));
    while true loop

      wait until rising_edge(clk_i);
      end_cyc <= end_cycle;
      if end_cycle and stb_cnt = 0 then
        master_o.cyc <= '0';
        master_o.stb <= '0';
        end_cycle := false;
        wait until rising_edge(clk_i);
        end_cyc <= end_cycle;
      end if;

      eb_simbridge_msi_slave_in(msi_slave_i_cyc,msi_slave_i_stb,msi_slave_i_we,msi_slave_i_adr,msi_slave_i_dat,msi_slave_i_sel);
      eb_simbridge_master_out(master_o_cyc,master_o_stb,master_o_we,master_o_adr,master_o_dat,master_o_sel,master_o_end_cyc);
      if master_o_end_cyc /= 0 then
        end_cycle_request := true;
      end if;
      master_o.cyc <= master_o_cyc;
      master_o.stb <= master_o_stb;
      master_o.we  <= master_o_we;
      master_o.adr <= std_logic_vector(to_signed(master_o_adr,32));
      master_o.dat <= std_logic_vector(to_signed(master_o_dat,32));      
      master_o.sel <= std_logic_vector(to_signed(master_o_sel, 4));    
      --if master_o_stb = '1' and master_o_cyc = '1' and master_i.stall = '0' then
      --  report "stb:" & integer'image(master_o_adr);
      --end if;

      wait until falling_edge(clk_i);
      end_cyc <= end_cycle;
      master_i_ack := master_i.ack;
      master_i_err := master_i.err;
      master_i_rty := master_i.rty;
      master_i_stall := master_i.stall;
      if end_cycle then master_i_stall := '1'; end if;
      master_i_dat := to_integer(signed(master_i.dat));
      if master_o_cyc = '1' and master_o_stb = '1' and master_i_stall = '0' and master_i_ack = '0' then stb_cnt := stb_cnt + 1; end if;
      if master_o_cyc = '1' and master_o_stb = '1' and master_i_stall = '1' and master_i_ack = '1' then stb_cnt := stb_cnt - 1; end if;
      if master_o_cyc = '1' and master_o_stb = '0'                          and master_i_ack = '1' then stb_cnt := stb_cnt - 1; end if;
      counter <= stb_cnt;
      if stb_cnt > 0 and end_cycle_request then
        end_cycle_request := false;
        end_cycle := true;
      end if;
      eb_simbridge_master_in(master_i_ack,master_i_err,master_i_rty,master_i_stall,master_i_dat,master_i_end_cyc);
      if master_i_end_cyc /= 0 then
        end_cycle_request := true;
      end if;
      eb_simbridge_msi_slave_out(msi_slave_o_ack,msi_slave_o_err,msi_slave_o_rty,msi_slave_o_stall,msi_slave_o_dat);
      if master_i.err = '1' and master_o_cyc = '1' then
        report "err:" & integer'image(master_i_dat);
      end if;
      
    end loop;
  end process;

end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.simbridge_pkg.all;

entity simbridge_chopper is
  port (
    clk_i    :  in std_logic;
    rstn_i    :  in std_logic;
    slave_i  :  in t_wishbone_slave_in;
    slave_o  : out t_wishbone_slave_out;
    master_o : out t_wishbone_master_out;
    master_i :  in t_wishbone_master_in
    );
end entity;

architecture rtl of simbridge_chopper is
  signal stall : std_logic := '0';
  signal master_o_adr : std_logic_vector(31 downto 0) := (others => '0');
  signal slave_i_cyc_1 : std_logic := '0';
begin
  master_o <= (cyc => slave_i.cyc,
             stb => slave_i.stb and not stall,
             we  => slave_i.we,
             sel => slave_i.sel,
             adr => master_o_adr,
             dat => slave_i.dat);
  slave_o <= (ack   => master_i.ack,
            err   => master_i.err,
            rty   => master_i.rty,
            stall => master_i.stall or stall,
            dat   => master_i.dat);

  master_o_adr <= slave_i.adr when (master_i.stall = '0' and stall = '0' and slave_i.stb = '1' and slave_i.cyc = '1') 
                                or slave_i.cyc = '0'
                                or (slave_i_cyc_1 = '0' and slave_i.cyc = '1')
              else master_o_adr;

  process 
  begin
    wait until rising_edge(clk_i);
    slave_i_cyc_1 <= slave_i.cyc;
    if stall = '0' and master_i.stall = '0' and slave_i.stb = '1' then
      stall <= '1';
    elsif master_i.ack = '1' or master_i.err = '1' or master_i.rty = '1' then
    --else 
      stall <= '0';
    end if;
  end process;

end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.wishbone_pkg.all;
use work.simbridge_pkg.all;


entity simbridge_chopped is
  generic (
      g_sdb_address    : t_wishbone_address;
      g_simbridge_msi  : t_sdb_msi := c_simbridge_msi
    );
  port (
  clk_i       : in  std_logic;
  rstn_i      : in  std_logic;
  master_o    : out t_wishbone_master_out;
  master_i    : in  t_wishbone_master_in;
  msi_slave_i : in  t_wishbone_slave_in;
  msi_slave_o : out t_wishbone_slave_out
    );
end entity;

architecture simulation of simbridge_chopped is
  signal master_out : t_wishbone_master_out;
  signal master_in  : t_wishbone_master_in;
  signal slave_out  : t_wishbone_slave_out;
  signal slave_int  : t_wishbone_slave_in;
begin

  sb: entity work.simbridge 
    generic map(g_sdb_address, g_simbridge_msi)
    port map(clk_i, rstn_i, master_out, master_in, msi_slave_i, msi_slave_o);

  cp: entity work.simbridge_chopper 
    port map(
      clk_i  => clk_i, 
      rstn_i => rstn_i,
      slave_i => master_out,
      slave_o => master_in,
      master_o => master_o,
      master_i => master_i);

end architecture;