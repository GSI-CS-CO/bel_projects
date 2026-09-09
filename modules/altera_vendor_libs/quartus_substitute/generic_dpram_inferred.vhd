-------------------------------------------------------------------------------
-- Inferred generic_dpram for GHDL without Quartus (no altera_mf / altsyncram).
-- Same entity as ip_cores/general-cores/.../altera/generic_dpram.vhd.
-- Loads Intel MIF (program.mif from bin2mif.py): "ADDR : DATA;" hex lines.
-- Used when make NO_QUARTUS=yes.
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use std.textio.all;

library work;
use work.genram_pkg.all;

entity generic_dpram is
  generic(
    g_data_width               : natural;
    g_size                     : natural;
    g_with_byte_enable         : boolean := false;
    g_addr_conflict_resolution : string  := "dont_care";
    g_init_file                : string  := "none";
    g_dual_clock               : boolean := true;
    g_fail_if_file_not_found   : boolean := false);
  port(
    rst_n_i : in std_logic := '1';

    clka_i : in  std_logic;
    bwea_i : in  std_logic_vector((g_data_width+7)/8-1 downto 0) := (others => '1');
    wea_i  : in  std_logic := '0';
    aa_i   : in  std_logic_vector(f_log2_size(g_size)-1 downto 0);
    da_i   : in  std_logic_vector(g_data_width-1 downto 0) := (others => '0');
    qa_o   : out std_logic_vector(g_data_width-1 downto 0);

    clkb_i : in  std_logic;
    bweb_i : in  std_logic_vector((g_data_width+7)/8-1 downto 0) := (others => '1');
    web_i  : in  std_logic := '0';
    ab_i   : in  std_logic_vector(f_log2_size(g_size)-1 downto 0);
    db_i   : in  std_logic_vector(g_data_width-1 downto 0) := (others => '0');
    qb_o   : out std_logic_vector(g_data_width-1 downto 0)
    );
end generic_dpram;

architecture syn of generic_dpram is

  constant c_num_bytes : integer := (g_data_width+7)/8;

  type t_ram_type is array(0 to g_size-1) of std_logic_vector(g_data_width-1 downto 0);

  function f_is_hex(c : character) return boolean is
  begin
    return (c >= '0' and c <= '9') or
           (c >= 'A' and c <= 'F') or
           (c >= 'a' and c <= 'f');
  end f_is_hex;

  function f_hex_nibble(c : character) return integer is
  begin
    if c >= '0' and c <= '9' then
      return character'pos(c) - character'pos('0');
    elsif c >= 'A' and c <= 'F' then
      return character'pos(c) - character'pos('A') + 10;
    elsif c >= 'a' and c <= 'f' then
      return character'pos(c) - character'pos('a') + 10;
    else
      return 0;
    end if;
  end f_hex_nibble;

  impure function f_load_mif return t_ram_type is
    file f_in       : text;
    variable status : file_open_status;
    variable l      : line;
    variable ram    : t_ram_type := (others => (others => '0'));

    procedure parse_line(s : string) is
      variable i    : integer := s'low;
      variable addr : integer;
      variable data : unsigned(g_data_width-1 downto 0);
    begin
      while i <= s'high and (s(i) = ' ' or s(i) = HT) loop
        i := i + 1;
      end loop;
      -- MIF data lines start with a hex address. Headers (WIDTH=..., CONTENT BEGIN)
      -- and comments do not.
      if i > s'high or not f_is_hex(s(i)) then
        return;
      end if;

      addr := 0;
      while i <= s'high and f_is_hex(s(i)) loop
        addr := addr * 16 + f_hex_nibble(s(i));
        i    := i + 1;
      end loop;

      while i <= s'high and (s(i) = ' ' or s(i) = HT) loop
        i := i + 1;
      end loop;
      if i > s'high or s(i) /= ':' then
        return;
      end if;
      i := i + 1;

      while i <= s'high and (s(i) = ' ' or s(i) = HT) loop
        i := i + 1;
      end loop;
      if i > s'high or not f_is_hex(s(i)) then
        return;
      end if;

      data := (others => '0');
      while i <= s'high and f_is_hex(s(i)) loop
        data := shift_left(data, 4) or to_unsigned(f_hex_nibble(s(i)), g_data_width);
        i    := i + 1;
      end loop;

      if addr >= 0 and addr < g_size then
        ram(addr) := std_logic_vector(data);
      end if;
    end procedure;
  begin
    if g_init_file'length = 0 or g_init_file = "none" or g_init_file = "UNUSED" then
      return ram;
    end if;

    file_open(status, f_in, g_init_file, read_mode);
    if status /= OPEN_OK then
      assert not g_fail_if_file_not_found
        report "generic_dpram: cannot open init file '" & g_init_file & "'"
        severity failure;
      return ram;
    end if;

    while not endfile(f_in) loop
      readline(f_in, l);
      if l /= null and l'length > 0 then
        parse_line(l.all);
      end if;
    end loop;

    file_close(f_in);
    return ram;
  end f_load_mif;

  shared variable ram : t_ram_type := f_load_mif;

  signal clkb : std_logic;

begin

  clkb <= clka_i when not g_dual_clock else clkb_i;

  port_a : process(clka_i)
    variable idx : integer;
  begin
    if rising_edge(clka_i) then
      idx  := f_check_bounds(to_integer(unsigned(aa_i)), 0, g_size-1);
      qa_o <= ram(idx);
      if wea_i = '1' then
        if g_with_byte_enable then
          for b in 0 to c_num_bytes-1 loop
            if bwea_i(b) = '1' then
              ram(idx)((b+1)*8-1 downto b*8) := da_i((b+1)*8-1 downto b*8);
            end if;
          end loop;
        else
          ram(idx) := da_i;
        end if;
      end if;
    end if;
  end process;

  port_b : process(clkb)
    variable idx : integer;
  begin
    if rising_edge(clkb) then
      idx  := f_check_bounds(to_integer(unsigned(ab_i)), 0, g_size-1);
      qb_o <= ram(idx);
      if web_i = '1' then
        if g_with_byte_enable then
          for b in 0 to c_num_bytes-1 loop
            if bweb_i(b) = '1' then
              ram(idx)((b+1)*8-1 downto b*8) := db_i((b+1)*8-1 downto b*8);
            end if;
          end loop;
        else
          ram(idx) := db_i;
        end if;
      end if;
    end if;
  end process;

end syn;
