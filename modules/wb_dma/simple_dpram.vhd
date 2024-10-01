-- Simple Dual-Port Block RAM with One Clock
-- Correct Modelization with a Shared Variable
-- File:simple_dual_one_clock.vhd

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;

entity simple_dpram is
generic(
  byte_size                          : natural;
  numwords_a                         : natural;
  numwords_b                         : natural;
  widthad_a                          : natural;
  widthad_b                          : natural;
  width_a                            : natural;
  width_b                            : natural;
  operation_mode                     : string;
  read_during_write_mode_mixed_ports : string;
  read_during_write_mode_port_a      : string;
  read_during_write_mode_port_b      : string;
  outdata_reg_a                      : string;
  outdata_reg_b                      : string;
  address_reg_b                      : string;
  wrcontrol_wraddress_reg_b          : string;
  byteena_reg_b                      : string;
  indata_reg_b                       : string;
  rdcontrol_reg_b                    : string;
  init_file                          : string
);
port(
  -- clk       : in std_logic;
  -- ena       : in std_logic;
  -- enb       : in std_logic;
  -- wea       : in std_logic;
  -- addra     : in std_logic_vector(9 downto 0);
  -- addrb     : in std_logic_vector(9 downto 0);
  -- dia       : in std_logic_vector(15 downto 0);
  -- dob       : out std_logic_vector(15 downto 0);
  clock0    : in std_logic;
  wren_a    : in std_logic;
  address_a : in std_logic_vector(widthad_a-1 downto 0);
  data_a    : in std_logic_vector(width_a-1 downto 0);
  q_a       : out std_logic_vector(width_a-1 downto 0);

  wren_b    : in std_logic;
  address_b : in std_logic_vector(widthad_b-1 downto 0);
  data_b    : in std_logic_vector(width_b-1 downto 0);
  q_b       : out std_logic_vector(width_b-1 downto 0);
);
end simple_dpram;

architecture syn of simple_dpram is
type ram_type is array (1023 downto 0) of std_logic_vector(15 downto 0);
shared variable RAM : ram_type;
begin
process(clock0)
begin
if clock0'event and clock0 = '1' then
if wren_a = '1' then
RAM(conv_integer(address_a)) := data_a;
end if;
end if;
end process;

process(clock0)
begin
if clock0'event and clock0 = '1' then
q_b <= RAM(conv_integer(address_b));
end if;
end process;

end syn;