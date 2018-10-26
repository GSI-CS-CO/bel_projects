--! @file io_control.vhd
--! @brief Control unit for bidirectional IO and more
--! @author CSCO-TG <csco-tg@gsi.de>
--!
--! Copyright (C) 2015 GSI Helmholtz Centre for Heavy Ion Research GmbH
--!
--------------------------------------------------------------------------------
--! This library is free software; you can redistribute it and/or
--! modify it under the terms of the GNU Lesser General Public
--! License as published by the Free Software Foundation; either
--! version 3 of the License, or (at your option) any later version.
--!
--! This library is distributed in the hope that it will be useful,
--! but WITHOUT ANY WARRANTY; without even the implied warranty of
--! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
--! Lesser General Public License for more details.
--!
--! You should have received a copy of the GNU Lesser General Public
--! License along with this library. If not, see <http://www.gnu.org/licenses/>.
---------------------------------------------------------------------------------
-- Libraries
use std.textio.all;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.monster_pkg.all;
use work.io_control_pkg.all;
use work.altera_lvds_pkg.all;

entity io_control is
  generic(
    g_project    : string;
    g_syn_target : string  := "Simulation";
    g_rom_delay  : natural := 1;
    g_version    : natural := 1;
    g_gpio_in    : natural := 0;
    g_gpio_out   : natural := 0;
    g_gpio_inout : natural := 0;
    g_lvds_in    : natural := 0;
    g_lvds_out   : natural := 0;
    g_lvds_inout : natural := 0;
    g_fixed      : natural := 0;
    g_io_table             : t_io_mapping_table_arg_array(natural range <>));
  port(
    clk_i           : in  std_logic;
    rst_n_i         : in  std_logic;
    slave_i         : in  t_wishbone_slave_in;
    slave_o         : out t_wishbone_slave_out;
    gpio_input_i    : in  std_logic_vector(f_sub1(g_gpio_in+g_gpio_inout)   downto 0);
    gpio_output_i   : in  std_logic_vector(f_sub1(g_gpio_out+g_gpio_inout)  downto 0);
    gpio_output_o   : out std_logic_vector(f_sub1(g_gpio_out+g_gpio_inout)  downto 0);
    lvds_input_i    : in  t_lvds_byte_array(f_sub1(g_lvds_in+g_lvds_inout)  downto 0);
    lvds_output_i   : in  t_lvds_byte_array(f_sub1(g_lvds_out+g_lvds_inout) downto 0);
    lvds_output_o   : out t_lvds_byte_array(f_sub1(g_lvds_out+g_lvds_inout) downto 0);
    gpio_oe_o       : out std_logic_vector(f_sub1(g_gpio_inout+g_gpio_out)  downto 0);
    gpio_term_o     : out std_logic_vector(f_sub1(g_gpio_inout+g_gpio_in)   downto 0);
    gpio_spec_out_o : out std_logic_vector(f_sub1(g_gpio_inout+g_gpio_out)  downto 0);
    gpio_spec_in_o  : out std_logic_vector(f_sub1(g_gpio_inout+g_gpio_in)   downto 0);
    gpio_mux_o      : out std_logic_vector(f_sub1(g_gpio_inout+g_gpio_out)  downto 0);
    gpio_pps_mux_o  : out std_logic_vector(f_sub1(g_gpio_inout+g_gpio_out)  downto 0);
    gpio_sel_o      : out std_logic_vector(f_sub1(g_gpio_inout+g_gpio_out)  downto 0);
    lvds_oe_o       : out std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out)  downto 0);
    lvds_term_o     : out std_logic_vector(f_sub1(g_lvds_inout+g_lvds_in)   downto 0);
    lvds_spec_out_o : out std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out)  downto 0);
    lvds_spec_in_o  : out std_logic_vector(f_sub1(g_lvds_inout+g_lvds_in)   downto 0);
    lvds_mux_o      : out std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out)  downto 0);
    lvds_pps_mux_o  : out std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out)  downto 0);
    lvds_sel_o      : out std_logic_vector(f_sub1(g_lvds_inout+g_lvds_out)  downto 0));
end io_control;

architecture rtl of io_control is
  -- Signals and registers
  signal r_legacy_mode                    : std_logic                     := '0';
  signal r_ack                            : std_logic                     := '0';
  signal r_ack_delay                      : std_logic                     := '0';
  signal r_ack_delay_out                  : std_logic                     := '0';
  signal r_dat                            : t_wishbone_data               := (others => '0');
  signal r_rom_data                       : std_logic_vector(31 downto 0) := (others => '0');
  signal r_io_cfg_reg                     : std_logic_vector(31 downto 0) := (others => '0');
  signal r_version_reg                    : std_logic_vector(31 downto 0) := (others => '0');
  signal r_fixed_info_reg                 : std_logic_vector(31 downto 0) := (others => '0');
  signal r_gpio_info_reg                  : std_logic_vector(31 downto 0) := (others => '0');
  signal r_lvds_info_reg                  : std_logic_vector(31 downto 0) := (others => '0');
  signal r_gpio_oe_legacy                 : std_logic_vector(63 downto 0) := (others => '0');
  signal r_lvds_oe_legacy                 : std_logic_vector(63 downto 0) := (others => '0');
  signal r_gpio_oe                        : std_logic_vector(63 downto 0) := (others => '0');
  signal r_lvds_oe                        : std_logic_vector(63 downto 0) := (others => '0');
  signal r_gpio_term                      : std_logic_vector(63 downto 0) := (others => '1');
  signal r_lvds_term                      : std_logic_vector(63 downto 0) := (others => '1');
  signal r_gpio_spec_in                   : std_logic_vector(63 downto 0) := (others => '0');
  signal r_gpio_spec_out                  : std_logic_vector(63 downto 0) := (others => '0');
  signal r_lvds_spec_in                   : std_logic_vector(63 downto 0) := (others => '0');
  signal r_lvds_spec_out                  : std_logic_vector(63 downto 0) := (others => '0');
  signal r_gpio_mux                       : std_logic_vector(63 downto 0) := (others => '0');
  signal r_lvds_mux                       : std_logic_vector(63 downto 0) := (others => '0');
  signal r_gpio_pps_mux                   : std_logic_vector(63 downto 0) := (others => '0');
  signal r_lvds_pps_mux                   : std_logic_vector(63 downto 0) := (others => '0');
  signal r_gpio_sel                       : std_logic_vector(63 downto 0) := (others => '0');
  signal r_lvds_sel                       : std_logic_vector(63 downto 0) := (others => '0');
  signal r_gpio_drive                     : std_logic_vector(63 downto 0) := (others => '0');
  signal r_lvds_drive                     : t_lvds_byte_array(f_sub1(g_lvds_out+g_lvds_inout) downto 0) := (others => (others => '0'));
  signal s_delay_counter                  : natural range 0 to 7;
  signal s_bit_selector                   : natural range 0 to (2**16)-1;
  signal s_field_selector                 : natural range 0 to (2**16)-1;
  signal s_entry_selector                 : natural range 0 to (2**16)-1;
  -- Generic constants
  constant c_table_pointer                : natural := (g_gpio_in+g_gpio_out+g_gpio_inout+g_lvds_in+g_lvds_out+g_lvds_inout)*4;
  constant c_gpio_inputs                  : natural := (g_gpio_inout+g_gpio_in);
  constant c_gpio_outputs                 : natural := (g_gpio_inout+g_gpio_out);
  constant c_gpio_total                   : natural := (g_gpio_inout+g_gpio_in+g_gpio_out);
  constant c_lvds_inputs                  : natural := (g_lvds_inout+g_lvds_in);
  constant c_lvds_outputs                 : natural := (g_lvds_inout+g_lvds_out);
  constant c_lvds_total                   : natural := (g_lvds_inout+g_lvds_in+g_lvds_out);
  -- Legacy mode registers
  constant c_gpio_oe_legacy_low_reg       : std_logic_vector (13 downto 0) := "00000000000000"; -- 0x0000
  constant c_lvds_oe_legacy_low_reg       : std_logic_vector (13 downto 0) := "00000000000001"; -- 0x0004
  constant c_gpio_oe_legacy_high_reg      : std_logic_vector (13 downto 0) := "00000000000010"; -- 0x0008
  constant c_lvds_oe_legacy_high_reg      : std_logic_vector (13 downto 0) := "00000000000011"; -- 0x000c
  constant c_io_config_reg                : std_logic_vector (13 downto 0) := "00000000000100"; -- 0x0010
  -- Information registers
  constant c_version_reg                  : std_logic_vector (13 downto 0) := "00000001000000"; -- 0x0100
  constant c_gpio_info_reg                : std_logic_vector (13 downto 0) := "00000001000001"; -- 0x0104
  constant c_lvds_info_reg                : std_logic_vector (13 downto 0) := "00000001000010"; -- 0x0108
  constant c_fixed_info_reg               : std_logic_vector (13 downto 0) := "00000001000011"; -- 0x010c
  -- GPIO OE registers
  constant c_gpio_oe_set_low_reg          : std_logic_vector (13 downto 0) := "00000010000000"; -- 0x0200
  constant c_gpio_oe_set_high_reg         : std_logic_vector (13 downto 0) := "00000010000001"; -- 0x0204
  constant c_gpio_oe_reset_low_reg        : std_logic_vector (13 downto 0) := "00000010000010"; -- 0x0208
  constant c_gpio_oe_reset_high_reg       : std_logic_vector (13 downto 0) := "00000010000011"; -- 0x020c
  -- LVDS OE registers
  constant c_lvds_oe_set_low_reg          : std_logic_vector (13 downto 0) := "00000011000000"; -- 0x0300
  constant c_lvds_oe_set_high_reg         : std_logic_vector (13 downto 0) := "00000011000001"; -- 0x0304
  constant c_lvds_oe_reset_low_reg        : std_logic_vector (13 downto 0) := "00000011000010"; -- 0x0308
  constant c_lvds_oe_reset_high_reg       : std_logic_vector (13 downto 0) := "00000011000011"; -- 0x030c
  -- GPIO TERM registers
  constant c_gpio_term_set_low_reg        : std_logic_vector (13 downto 0) := "00000100000000"; -- 0x0400
  constant c_gpio_term_set_high_reg       : std_logic_vector (13 downto 0) := "00000100000001"; -- 0x0404
  constant c_gpio_term_reset_low_reg      : std_logic_vector (13 downto 0) := "00000100000010"; -- 0x0408
  constant c_gpio_term_reset_high_reg     : std_logic_vector (13 downto 0) := "00000100000011"; -- 0x040c
  -- LVDS TERM registers
  constant c_lvds_term_set_low_reg        : std_logic_vector (13 downto 0) := "00000101000000"; -- 0x0500
  constant c_lvds_term_set_high_reg       : std_logic_vector (13 downto 0) := "00000101000001"; -- 0x0504
  constant c_lvds_term_reset_low_reg      : std_logic_vector (13 downto 0) := "00000101000010"; -- 0x0508
  constant c_lvds_term_reset_high_reg     : std_logic_vector (13 downto 0) := "00000101000011"; -- 0x050c
  -- GPIO SPECIAL IN registers
  constant c_gpio_spec_in_set_low_reg     : std_logic_vector (13 downto 0) := "00000110000000"; -- 0x0600
  constant c_gpio_spec_in_set_high_reg    : std_logic_vector (13 downto 0) := "00000110000001"; -- 0x0604
  constant c_gpio_spec_in_reset_low_reg   : std_logic_vector (13 downto 0) := "00000110000010"; -- 0x0608
  constant c_gpio_spec_in_reset_high_reg  : std_logic_vector (13 downto 0) := "00000110000011"; -- 0x060c
  -- GPIO SPECIAL OUT registers
  constant c_gpio_spec_out_set_low_reg    : std_logic_vector (13 downto 0) := "00000111000000"; -- 0x0700
  constant c_gpio_spec_out_set_high_reg   : std_logic_vector (13 downto 0) := "00000111000001"; -- 0x0704
  constant c_gpio_spec_out_reset_low_reg  : std_logic_vector (13 downto 0) := "00000111000010"; -- 0x0708
  constant c_gpio_spec_out_reset_high_reg : std_logic_vector (13 downto 0) := "00000111000011"; -- 0x070c
  -- LVDS SPECIAL IN registers
  constant c_lvds_spec_in_set_low_reg     : std_logic_vector (13 downto 0) := "00001000000000"; -- 0x0800
  constant c_lvds_spec_in_set_high_reg    : std_logic_vector (13 downto 0) := "00001000000001"; -- 0x0804
  constant c_lvds_spec_in_reset_low_reg   : std_logic_vector (13 downto 0) := "00001000000010"; -- 0x0808
  constant c_lvds_spec_in_reset_high_reg  : std_logic_vector (13 downto 0) := "00001000000011"; -- 0x080c
  -- LVDS SPECIAL OUT registers
  constant c_lvds_spec_out_set_low_reg    : std_logic_vector (13 downto 0) := "00001001000000"; -- 0x0900
  constant c_lvds_spec_out_set_high_reg   : std_logic_vector (13 downto 0) := "00001001000001"; -- 0x0904
  constant c_lvds_spec_out_reset_low_reg  : std_logic_vector (13 downto 0) := "00001001000010"; -- 0x0908
  constant c_lvds_spec_out_reset_high_reg : std_logic_vector (13 downto 0) := "00001001000011"; -- 0x090c
  -- GPIO MUX
  constant c_gpio_mux_set_low_reg         : std_logic_vector (13 downto 0) := "00001010000000"; -- 0x0a00
  constant c_gpio_mux_set_high_reg        : std_logic_vector (13 downto 0) := "00001010000001"; -- 0x0a04
  constant c_gpio_mux_reset_low_reg       : std_logic_vector (13 downto 0) := "00001010000010"; -- 0x0a08
  constant c_gpio_mux_reset_high_reg      : std_logic_vector (13 downto 0) := "00001010000011"; -- 0x0a0c
  -- LVDS MUX
  constant c_lvds_mux_set_low_reg         : std_logic_vector (13 downto 0) := "00001011000000"; -- 0x0b00
  constant c_lvds_mux_set_high_reg        : std_logic_vector (13 downto 0) := "00001011000001"; -- 0x0b04
  constant c_lvds_mux_reset_low_reg       : std_logic_vector (13 downto 0) := "00001011000010"; -- 0x0b08
  constant c_lvds_mux_reset_high_reg      : std_logic_vector (13 downto 0) := "00001011000011"; -- 0x0b0c
  -- GPIO SEL
  constant c_gpio_sel_set_low_reg         : std_logic_vector (13 downto 0) := "00001100000000"; -- 0x0c00
  constant c_gpio_sel_set_high_reg        : std_logic_vector (13 downto 0) := "00001100000001"; -- 0x0c04
  constant c_gpio_sel_reset_low_reg       : std_logic_vector (13 downto 0) := "00001100000010"; -- 0x0c08
  constant c_gpio_sel_reset_high_reg      : std_logic_vector (13 downto 0) := "00001100000011"; -- 0x0c0c
  -- LVDS SEL
  constant c_lvds_sel_set_low_reg         : std_logic_vector (13 downto 0) := "00001101000000"; -- 0x0d00
  constant c_lvds_sel_set_high_reg        : std_logic_vector (13 downto 0) := "00001101000001"; -- 0x0d04
  constant c_lvds_sel_reset_low_reg       : std_logic_vector (13 downto 0) := "00001101000010"; -- 0x0d08
  constant c_lvds_sel_reset_high_reg      : std_logic_vector (13 downto 0) := "00001101000011"; -- 0x0d0c
  -- GPIO PPS MUX
  constant c_gpio_pps_mux_set_low_reg     : std_logic_vector (13 downto 0) := "00001110000000"; -- 0x0e00
  constant c_gpio_pps_mux_set_high_reg    : std_logic_vector (13 downto 0) := "00001110000001"; -- 0x0e04
  constant c_gpio_pps_mux_reset_low_reg   : std_logic_vector (13 downto 0) := "00001110000010"; -- 0x0e08
  constant c_gpio_pps_mux_reset_high_reg  : std_logic_vector (13 downto 0) := "00001110000011"; -- 0x0e0c
  -- LVDS PPS MUX
  constant c_lvds_pps_mux_set_low_reg     : std_logic_vector (13 downto 0) := "00001111000000"; -- 0x0f00
  constant c_lvds_pps_mux_set_high_reg    : std_logic_vector (13 downto 0) := "00001111000001"; -- 0x0f04
  constant c_lvds_pps_mux_reset_low_reg   : std_logic_vector (13 downto 0) := "00001111000010"; -- 0x0f08
  constant c_lvds_pps_mux_reset_high_reg  : std_logic_vector (13 downto 0) := "00001111000011"; -- 0x0f0c
  -- GPIO registers addresses for set status/value
  constant c_set_gpio_out_begin_reg       : std_logic_vector (13 downto 0) := "10100000000000"; -- 0xannn ...
  constant c_set_gpio_out_offset_reg      : std_logic_vector (13 downto 0) := std_logic_vector(to_unsigned((f_sub1(g_gpio_out+g_gpio_inout)), c_set_gpio_out_begin_reg'length));
  constant c_set_gpio_out_end_reg         : std_logic_vector (13 downto 0) := std_logic_vector(unsigned(c_set_gpio_out_begin_reg) + unsigned(c_set_gpio_out_offset_reg));
  -- LVDS registers addresses for set status/value
  constant c_set_lvds_out_begin_reg       : std_logic_vector (13 downto 0) := "10110000000000"; -- 0xbnnn ...
  constant c_set_lvds_out_offset_reg      : std_logic_vector (13 downto 0) := std_logic_vector(to_unsigned((f_sub1(g_lvds_out+g_lvds_inout)), c_set_lvds_out_begin_reg'length));
  constant c_set_lvds_out_end_reg         : std_logic_vector (13 downto 0) := std_logic_vector(unsigned(c_set_lvds_out_begin_reg) + unsigned(c_set_lvds_out_offset_reg));
  -- GPIO registers addresses for get status/value
  constant c_get_gpio_in_begin_reg        : std_logic_vector (13 downto 0) := "11000000000000"; -- 0xcnnn ...
  constant c_get_gpio_in_offset_reg       : std_logic_vector (13 downto 0) := std_logic_vector(to_unsigned((f_sub1(g_gpio_in+g_gpio_inout)), c_get_gpio_in_begin_reg'length));
  constant c_get_gpio_in_end_reg          : std_logic_vector (13 downto 0) := std_logic_vector(unsigned(c_get_gpio_in_begin_reg) + unsigned(c_get_gpio_in_offset_reg));
  constant c_get_gpio_out_begin_reg       : std_logic_vector (13 downto 0) := std_logic_vector(unsigned(c_get_gpio_in_end_reg) + 1);
  constant c_get_gpio_out_offset_reg      : std_logic_vector (13 downto 0) := std_logic_vector(to_unsigned((f_sub1(g_gpio_out+g_gpio_inout)), c_get_gpio_out_begin_reg'length));
  constant c_get_gpio_out_end_reg         : std_logic_vector (13 downto 0) := std_logic_vector(unsigned(c_get_gpio_out_begin_reg) + unsigned(c_get_gpio_out_offset_reg));
  -- LVDS registers addresses for get status/value
  constant c_get_lvds_in_begin_reg        : std_logic_vector (13 downto 0) := "11010000000000"; -- 0xdnnn ...
  constant c_get_lvds_in_offset_reg       : std_logic_vector (13 downto 0) := std_logic_vector(to_unsigned((f_sub1(g_lvds_in+g_lvds_inout)), c_get_lvds_in_begin_reg'length));
  constant c_get_lvds_in_end_reg          : std_logic_vector (13 downto 0) := std_logic_vector(unsigned(c_get_lvds_in_begin_reg) + unsigned(c_get_lvds_in_offset_reg));
  constant c_get_lvds_out_begin_reg       : std_logic_vector (13 downto 0) := std_logic_vector(unsigned(c_get_lvds_in_end_reg) + 1);
  constant c_get_lvds_out_offset_reg      : std_logic_vector (13 downto 0) := std_logic_vector(to_unsigned((f_sub1(g_lvds_out+g_lvds_inout)), c_get_lvds_out_begin_reg'length));
  constant c_get_lvds_out_end_reg         : std_logic_vector (13 downto 0) := std_logic_vector(unsigned(c_get_lvds_out_begin_reg) + unsigned(c_get_lvds_out_offset_reg));
  -- IO mapping table
  constant c_io_map_table_begin_reg       : std_logic_vector (13 downto 0) := "11100000000000"; -- 0xennn ...
  constant c_io_map_table_end_reg         : std_logic_vector (13 downto 0) := "11111111111100"; -- 0xfff0 ...
  -- IO mapping table layout
  constant c_is_arria5       : boolean := g_syn_target = "Arria V";
  constant c_is_arria2       : boolean := g_syn_target = "Arria II";
  constant c_is_altera       : boolean := c_is_arria5 or c_is_arria2;
  constant c_is_simulation   : boolean := g_syn_target = "Simulation";
  constant c_ios_total       : natural := c_gpio_total + c_lvds_total + g_fixed;
  constant c_io_table_memory : t_io_mapping_table_array := f_gen_io_table(g_io_table, c_ios_total);
  --signal   s_io_table_memory : t_io_mapping_table_array;

begin

  -- Wishbone slave interface
  slave_o.dat <= r_dat;
  slave_o.ack <= r_ack when (g_rom_delay = 0) else r_ack_delay_out;

  -- Delay data/acknowledge by two cycles (to please slow ROMs)
  p_wishbone_delay_handler : process(clk_i, rst_n_i) is
  begin
    if (rst_n_i = '0') then
      r_ack_delay     <= '0';
      r_ack_delay_out <= '0';
    elsif (rising_edge(clk_i)) then
      r_ack_delay     <= r_ack;
      r_ack_delay_out <= r_ack_delay;
    end if;
  end process;

  -- Unused Wishbone slave signals
  slave_o.err   <= '0';
  slave_o.int   <= '0';
  slave_o.rty   <= '0';
  slave_o.stall <= '0';

  -- Output improved or legacy behavior
  gpio_oe_o                                                     <= r_gpio_oe(f_sub1(c_gpio_outputs) downto 0) when r_legacy_mode='0' else r_gpio_oe_legacy(f_sub1(c_gpio_outputs) downto 0);
  lvds_oe_o                                                     <= r_lvds_oe(f_sub1(c_lvds_outputs) downto 0) when r_legacy_mode='0' else r_lvds_oe_legacy(f_sub1(c_lvds_outputs) downto 0);
  gpio_term_io: if (g_gpio_inout>0) generate
    gpio_term_o(g_gpio_inout-1 downto 0)                        <= r_gpio_term(g_gpio_inout-1 downto 0) when r_legacy_mode='0' else not(r_gpio_oe_legacy(g_gpio_inout-1 downto 0));
  end generate;
  gpio_term_in: if (g_gpio_in>0) generate
    gpio_term_o((g_gpio_inout+g_gpio_in)-1 downto g_gpio_inout) <= r_gpio_term((g_gpio_inout+g_gpio_in)-1 downto g_gpio_inout) when r_legacy_mode='0' else (others => '1');
  end generate;
  lvds_term_io: if (g_lvds_inout>0) generate
    lvds_term_o(g_lvds_inout-1 downto 0)                        <= r_lvds_term(g_lvds_inout-1 downto 0) when r_legacy_mode='0' else not(r_lvds_oe_legacy(g_lvds_inout-1 downto 0));
  end generate;
  lvds_term_in: if (g_lvds_in>0) generate
    lvds_term_o((g_lvds_inout+g_lvds_in)-1 downto g_lvds_inout) <= r_lvds_term((g_lvds_inout+g_lvds_in)-1 downto g_lvds_inout) when r_legacy_mode='0' else (others => '1');
  end generate;
  gpio_spec_in_o                                                <= r_gpio_spec_in(f_sub1(c_gpio_inputs)   downto 0) when r_legacy_mode='0' else (others => '0');
  gpio_spec_out_o                                               <= r_gpio_spec_out(f_sub1(c_gpio_outputs) downto 0) when r_legacy_mode='0' else (others => '0');
  lvds_spec_in_o                                                <= r_lvds_spec_in(f_sub1(c_lvds_inputs)   downto 0) when r_legacy_mode='0' else (others => '0');
  lvds_spec_out_o                                               <= r_lvds_spec_out(f_sub1(c_lvds_outputs) downto 0) when r_legacy_mode='0' else (others => '0');
  gpio_mux_o                                                    <= r_gpio_mux(f_sub1(c_gpio_outputs)      downto 0) when r_legacy_mode='0' else (others => '0');
  lvds_mux_o                                                    <= r_lvds_mux(f_sub1(c_lvds_outputs)      downto 0) when r_legacy_mode='0' else (others => '0');
  gpio_pps_mux_o                                                <= r_gpio_pps_mux(f_sub1(c_gpio_outputs)  downto 0) when r_legacy_mode='0' else (others => '0');
  lvds_pps_mux_o                                                <= r_lvds_pps_mux(f_sub1(c_lvds_outputs)  downto 0) when r_legacy_mode='0' else (others => '0');
  gpio_sel_o                                                    <= r_gpio_sel(f_sub1(c_gpio_outputs)      downto 0) when r_legacy_mode='0' else (others => '0');
  lvds_sel_o                                                    <= r_lvds_sel(f_sub1(c_lvds_outputs)      downto 0) when r_legacy_mode='0' else (others => '0');
  gpio_output_o                                                 <= r_gpio_drive(gpio_output_o'range)                when r_legacy_mode='0' else (others => '0');
  lvds_output_o                                                 <= r_lvds_drive(lvds_output_o'range)                when r_legacy_mode='0' else (others => (others => '0'));

  -- IO configuration register
  r_io_cfg_reg <= (0 => r_legacy_mode, others => '0');

  -- Version register
  r_version_reg <= std_logic_vector(to_unsigned(g_version, r_version_reg'length));

  -- Fixed IOs register
  r_fixed_info_reg <= std_logic_vector(to_unsigned(g_fixed, r_version_reg'length));

  -- GPIO information register
  p_gpio_info : process(r_gpio_info_reg) is
  begin
    r_gpio_info_reg(31 downto 24) <= std_logic_vector(to_unsigned(c_gpio_total, r_gpio_info_reg'length/4));
    r_gpio_info_reg(23 downto 16) <= std_logic_vector(to_unsigned(g_gpio_in,    r_gpio_info_reg'length/4));
    r_gpio_info_reg(15 downto  8) <= std_logic_vector(to_unsigned(g_gpio_out,   r_gpio_info_reg'length/4));
    r_gpio_info_reg(7  downto  0) <= std_logic_vector(to_unsigned(g_gpio_inout, r_gpio_info_reg'length/4));
  end process;

  -- LVDS information register
  p_lvds_info : process(r_lvds_info_reg) is
  begin
    r_lvds_info_reg(31 downto 24) <= std_logic_vector(to_unsigned(c_lvds_total, r_gpio_info_reg'length/4));
    r_lvds_info_reg(23 downto 16) <= std_logic_vector(to_unsigned(g_lvds_in,    r_gpio_info_reg'length/4));
    r_lvds_info_reg(15 downto  8) <= std_logic_vector(to_unsigned(g_lvds_out,   r_gpio_info_reg'length/4));
    r_lvds_info_reg(7  downto  0) <= std_logic_vector(to_unsigned(g_lvds_inout, r_gpio_info_reg'length/4));
  end process;

  -- Decode selection from slave input address (use the least significant bits to selected IO number or table entry field)
  p_bit_selector : process(slave_i.adr) is
  begin
    s_bit_selector   <= to_integer(unsigned(slave_i.adr(9  downto 2)));
    s_field_selector <= to_integer(unsigned(slave_i.adr(3  downto 2)));
    s_entry_selector <= to_integer(unsigned(slave_i.adr(11 downto 4)));
  end process;

  -- Handle wishbone requests
  p_wishbone_handler : process(clk_i, rst_n_i) is
  begin
    if (rst_n_i = '0') then
      -- Reset everything
      r_legacy_mode    <= '0';
      r_ack            <= '0';
      r_gpio_oe_legacy <= (others => '0');
      r_lvds_oe_legacy <= (others => '0');
      r_dat            <= (others => '0');
      r_gpio_oe        <= (others => '0');
      r_lvds_oe        <= (others => '0');
      r_gpio_term      <= (others => '1');
      r_lvds_term      <= (others => '1');
      r_gpio_spec_in   <= (others => '0');
      r_gpio_spec_out  <= (others => '0');
      r_lvds_spec_in   <= (others => '0');
      r_lvds_spec_out  <= (others => '0');
      r_gpio_mux       <= (others => '0');
      r_lvds_mux       <= (others => '0');
      r_gpio_pps_mux   <= (others => '0');
      r_lvds_pps_mux   <= (others => '0');
      r_gpio_sel       <= (others => '0');
      r_lvds_sel       <= (others => '0');
      r_gpio_drive     <= (others => '0');
      r_lvds_drive     <= (others => (others => '0'));

    elsif (rising_edge(clk_i)) then
      -- Handle generic wishbone signals
      r_ack <= slave_i.cyc and slave_i.stb;
      r_dat <= (others => '0');

      -- Handle write requests
      if (slave_i.cyc and slave_i.stb and slave_i.we) = '1' then
        case slave_i.adr(15 downto 2) is
          -- Known registers
          when c_gpio_oe_legacy_low_reg       => r_gpio_oe_legacy(31 downto  0) <= slave_i.dat;
          when c_lvds_oe_legacy_low_reg       => r_lvds_oe_legacy(31 downto  0) <= slave_i.dat;
          when c_gpio_oe_legacy_high_reg      => r_gpio_oe_legacy(63 downto 32) <= slave_i.dat;
          when c_lvds_oe_legacy_high_reg      => r_lvds_oe_legacy(63 downto 32) <= slave_i.dat;
          when c_io_config_reg                => r_legacy_mode                  <= slave_i.dat(0);
          when c_version_reg                  => null;                          -- read only
          when c_gpio_info_reg                => null;                          -- read only
          when c_lvds_info_reg                => null;                          -- read only
          when c_fixed_info_reg               => null;                          -- read only
          when c_gpio_oe_set_low_reg          => r_gpio_oe(31 downto  0)        <= r_gpio_oe(31 downto  0) or slave_i.dat;
          when c_gpio_oe_set_high_reg         => r_gpio_oe(63 downto 32)        <= r_gpio_oe(63 downto 32) or slave_i.dat;
          when c_gpio_oe_reset_low_reg        => r_gpio_oe(31 downto  0)        <= r_gpio_oe(31 downto  0) and not(slave_i.dat);
          when c_gpio_oe_reset_high_reg       => r_gpio_oe(63 downto 32)        <= r_gpio_oe(63 downto 32) and not(slave_i.dat);
          when c_lvds_oe_set_low_reg          => r_lvds_oe(31 downto  0)        <= r_lvds_oe(31 downto  0) or slave_i.dat;
          when c_lvds_oe_set_high_reg         => r_lvds_oe(63 downto 32)        <= r_lvds_oe(63 downto 32) or slave_i.dat;
          when c_lvds_oe_reset_low_reg        => r_lvds_oe(31 downto  0)        <= r_lvds_oe(31 downto  0) and not(slave_i.dat);
          when c_lvds_oe_reset_high_reg       => r_lvds_oe(63 downto 32)        <= r_lvds_oe(63 downto 32) and not(slave_i.dat);
          when c_gpio_term_set_low_reg        => r_gpio_term(31 downto  0)      <= r_gpio_term(31 downto  0) or slave_i.dat;
          when c_gpio_term_set_high_reg       => r_gpio_term(63 downto 32)      <= r_gpio_term(63 downto 32) or slave_i.dat;
          when c_gpio_term_reset_low_reg      => r_gpio_term(31 downto  0)      <= r_gpio_term(31 downto  0) and not(slave_i.dat);
          when c_gpio_term_reset_high_reg     => r_gpio_term(63 downto 32)      <= r_gpio_term(63 downto 32) and not(slave_i.dat);
          when c_lvds_term_set_low_reg        => r_lvds_term(31 downto  0)      <= r_lvds_term(31 downto  0) or slave_i.dat;
          when c_lvds_term_set_high_reg       => r_lvds_term(63 downto 32)      <= r_lvds_term(63 downto 32) or slave_i.dat;
          when c_lvds_term_reset_low_reg      => r_lvds_term(31 downto  0)      <= r_lvds_term(31 downto  0) and not(slave_i.dat);
          when c_lvds_term_reset_high_reg     => r_lvds_term(63 downto 32)      <= r_lvds_term(63 downto 32) and not(slave_i.dat);
          when c_gpio_spec_in_set_low_reg     => r_gpio_spec_in(31 downto  0)   <= r_gpio_spec_in(31 downto  0) or slave_i.dat;
          when c_gpio_spec_in_set_high_reg    => r_gpio_spec_in(63 downto 32)   <= r_gpio_spec_in(63 downto 32) or slave_i.dat;
          when c_gpio_spec_in_reset_low_reg   => r_gpio_spec_in(31 downto  0)   <= r_gpio_spec_in(31 downto  0) and not(slave_i.dat);
          when c_gpio_spec_in_reset_high_reg  => r_gpio_spec_in(63 downto 32)   <= r_gpio_spec_in(63 downto 32) and not(slave_i.dat);
          when c_gpio_spec_out_set_low_reg    => r_gpio_spec_out(31 downto  0)  <= r_gpio_spec_out(31 downto  0) or slave_i.dat;
          when c_gpio_spec_out_set_high_reg   => r_gpio_spec_out(63 downto 32)  <= r_gpio_spec_out(63 downto 32) or slave_i.dat;
          when c_gpio_spec_out_reset_low_reg  => r_gpio_spec_out(31 downto  0)  <= r_gpio_spec_out(31 downto  0) and not(slave_i.dat);
          when c_gpio_spec_out_reset_high_reg => r_gpio_spec_out(63 downto 32)  <= r_gpio_spec_out(63 downto 32) and not(slave_i.dat);
          when c_lvds_spec_in_set_low_reg     => r_lvds_spec_in(31 downto  0)   <= r_lvds_spec_in(31 downto  0) or slave_i.dat;
          when c_lvds_spec_in_set_high_reg    => r_lvds_spec_in(63 downto 32)   <= r_lvds_spec_in(63 downto 32) or slave_i.dat;
          when c_lvds_spec_in_reset_low_reg   => r_lvds_spec_in(31 downto  0)   <= r_lvds_spec_in(31 downto  0) and not(slave_i.dat);
          when c_lvds_spec_in_reset_high_reg  => r_lvds_spec_in(63 downto 32)   <= r_lvds_spec_in(63 downto 32) and not(slave_i.dat);
          when c_lvds_spec_out_set_low_reg    => r_lvds_spec_out(31 downto  0)  <= r_lvds_spec_out(31 downto  0) or slave_i.dat;
          when c_lvds_spec_out_set_high_reg   => r_lvds_spec_out(63 downto 32)  <= r_lvds_spec_out(63 downto 32) or slave_i.dat;
          when c_lvds_spec_out_reset_low_reg  => r_lvds_spec_out(31 downto  0)  <= r_lvds_spec_out(31 downto  0) and not(slave_i.dat);
          when c_lvds_spec_out_reset_high_reg => r_lvds_spec_out(63 downto 32)  <= r_lvds_spec_out(63 downto 32) and not(slave_i.dat);
          when c_gpio_mux_set_low_reg         => r_gpio_mux(31 downto  0)       <= r_gpio_mux(31 downto  0) or slave_i.dat;
          when c_gpio_mux_set_high_reg        => r_gpio_mux(63 downto 32)       <= r_gpio_mux(63 downto 32) or slave_i.dat;
          when c_gpio_mux_reset_low_reg       => r_gpio_mux(31 downto  0)       <= r_gpio_mux(31 downto  0) and not(slave_i.dat);
          when c_gpio_mux_reset_high_reg      => r_gpio_mux(63 downto 32)       <= r_gpio_mux(63 downto 32) and not(slave_i.dat);
          when c_lvds_mux_set_low_reg         => r_lvds_mux(31 downto  0)       <= r_lvds_mux(31 downto  0) or slave_i.dat;
          when c_lvds_mux_set_high_reg        => r_lvds_mux(63 downto 32)       <= r_lvds_mux(63 downto 32) or slave_i.dat;
          when c_lvds_mux_reset_low_reg       => r_lvds_mux(31 downto  0)       <= r_lvds_mux(31 downto  0) and not(slave_i.dat);
          when c_lvds_mux_reset_high_reg      => r_lvds_mux(63 downto 32)       <= r_lvds_mux(63 downto 32) and not(slave_i.dat);
          when c_gpio_sel_set_low_reg         => r_gpio_sel(31 downto  0)       <= r_gpio_sel(31 downto  0) or slave_i.dat;
          when c_gpio_sel_set_high_reg        => r_gpio_sel(63 downto 32)       <= r_gpio_sel(63 downto 32) or slave_i.dat;
          when c_gpio_sel_reset_low_reg       => r_gpio_sel(31 downto  0)       <= r_gpio_sel(31 downto  0) and not(slave_i.dat);
          when c_gpio_sel_reset_high_reg      => r_gpio_sel(63 downto 32)       <= r_gpio_sel(63 downto 32) and not(slave_i.dat);
          when c_lvds_sel_set_low_reg         => r_lvds_sel(31 downto  0)       <= r_lvds_sel(31 downto  0) or slave_i.dat;
          when c_lvds_sel_set_high_reg        => r_lvds_sel(63 downto 32)       <= r_lvds_sel(63 downto 32) or slave_i.dat;
          when c_lvds_sel_reset_low_reg       => r_lvds_sel(31 downto  0)       <= r_lvds_sel(31 downto  0) and not(slave_i.dat);
          when c_lvds_sel_reset_high_reg      => r_lvds_sel(63 downto 32)       <= r_lvds_sel(63 downto 32) and not(slave_i.dat);
          when c_gpio_pps_mux_set_low_reg     => r_gpio_pps_mux(31 downto  0)   <= r_gpio_pps_mux(31 downto  0) or slave_i.dat;
          when c_gpio_pps_mux_set_high_reg    => r_gpio_pps_mux(63 downto 32)   <= r_gpio_pps_mux(63 downto 32) or slave_i.dat;
          when c_gpio_pps_mux_reset_low_reg   => r_gpio_pps_mux(31 downto  0)   <= r_gpio_pps_mux(31 downto  0) and not(slave_i.dat);
          when c_gpio_pps_mux_reset_high_reg  => r_gpio_pps_mux(63 downto 32)   <= r_gpio_pps_mux(63 downto 32) and not(slave_i.dat);
          when c_lvds_pps_mux_set_low_reg     => r_lvds_pps_mux(31 downto  0)   <= r_lvds_pps_mux(31 downto  0) or slave_i.dat;
          when c_lvds_pps_mux_set_high_reg    => r_lvds_pps_mux(63 downto 32)   <= r_lvds_pps_mux(63 downto 32) or slave_i.dat;
          when c_lvds_pps_mux_reset_low_reg   => r_lvds_pps_mux(31 downto  0)   <= r_lvds_pps_mux(31 downto  0) and not(slave_i.dat);
          when c_lvds_pps_mux_reset_high_reg  => r_lvds_pps_mux(63 downto 32)   <= r_lvds_pps_mux(63 downto 32) and not(slave_i.dat);
          when others =>
            -- Set driven GPIO OUT values
            if (slave_i.adr(15 downto 2) >= c_set_gpio_out_begin_reg and slave_i.adr(15 downto 2) <= c_set_gpio_out_end_reg) then
              r_gpio_drive(s_bit_selector) <= slave_i.dat(0);
              report "Setting driven GPIO OUT at: " & integer'image(s_bit_selector) severity note;

            -- Set driven LVDS OUT values
            elsif (slave_i.adr(15 downto 2) >= c_set_lvds_out_begin_reg and slave_i.adr(15 downto 2) <= c_set_lvds_out_end_reg) then
              r_lvds_drive(s_bit_selector)  <= slave_i.dat(7 downto 0);
              report "Setting driven LVDS OUT at: " & integer'image(s_bit_selector-g_lvds_in-g_lvds_inout) severity note;

            -- Unknown access
            else
              if ((slave_i.cyc and slave_i.stb) = '1') then
                report "Unknown register access (write)!" severity error;
                r_dat(31 downto 16) <= x"dead";
                r_dat(15 downto  2) <= slave_i.adr(15 downto 2);
                r_dat( 1 downto  0) <= (others => '0');
              else
                r_dat <= (others => '0');
              end if;

            end if;
        end case;
      end if;

      -- Handle read/no-write requests
      case slave_i.adr(15 downto 2) is
        -- Known registers
          when c_gpio_oe_legacy_low_reg       => r_dat <= r_gpio_oe_legacy(31 downto  0);
          when c_lvds_oe_legacy_low_reg       => r_dat <= r_lvds_oe_legacy(31 downto  0);
          when c_gpio_oe_legacy_high_reg      => r_dat <= r_gpio_oe_legacy(63 downto 32);
          when c_lvds_oe_legacy_high_reg      => r_dat <= r_lvds_oe_legacy(63 downto 32);
          when c_io_config_reg                => r_dat <= r_io_cfg_reg;
          when c_version_reg                  => r_dat <= r_version_reg;
          when c_gpio_info_reg                => r_dat <= r_gpio_info_reg;
          when c_lvds_info_reg                => r_dat <= r_lvds_info_reg;
          when c_fixed_info_reg               => r_dat <= r_fixed_info_reg;
          when c_gpio_oe_set_low_reg          => r_dat <= r_gpio_oe(31 downto  0);
          when c_gpio_oe_set_high_reg         => r_dat <= r_gpio_oe(63 downto 32);
          when c_gpio_oe_reset_low_reg        => r_dat <= r_gpio_oe(31 downto  0);
          when c_gpio_oe_reset_high_reg       => r_dat <= r_gpio_oe(63 downto 32);
          when c_lvds_oe_set_low_reg          => r_dat <= r_lvds_oe(31 downto  0);
          when c_lvds_oe_set_high_reg         => r_dat <= r_lvds_oe(63 downto 32);
          when c_lvds_oe_reset_low_reg        => r_dat <= r_lvds_oe(31 downto  0);
          when c_lvds_oe_reset_high_reg       => r_dat <= r_lvds_oe(63 downto 32);
          when c_gpio_term_set_low_reg        => r_dat <= r_gpio_term(31 downto  0);
          when c_gpio_term_set_high_reg       => r_dat <= r_gpio_term(63 downto 32);
          when c_gpio_term_reset_low_reg      => r_dat <= r_gpio_term(31 downto  0);
          when c_gpio_term_reset_high_reg     => r_dat <= r_gpio_term(63 downto 32);
          when c_lvds_term_set_low_reg        => r_dat <= r_lvds_term(31 downto  0);
          when c_lvds_term_set_high_reg       => r_dat <= r_lvds_term(63 downto 32);
          when c_lvds_term_reset_low_reg      => r_dat <= r_lvds_term(31 downto  0);
          when c_lvds_term_reset_high_reg     => r_dat <= r_lvds_term(63 downto 32);
          when c_gpio_spec_in_set_low_reg     => r_dat <= r_gpio_spec_in(31 downto  0);
          when c_gpio_spec_in_set_high_reg    => r_dat <= r_gpio_spec_in(63 downto 32);
          when c_gpio_spec_in_reset_low_reg   => r_dat <= r_gpio_spec_in(31 downto  0);
          when c_gpio_spec_in_reset_high_reg  => r_dat <= r_gpio_spec_in(63 downto 32);
          when c_gpio_spec_out_set_low_reg    => r_dat <= r_gpio_spec_out(31 downto  0);
          when c_gpio_spec_out_set_high_reg   => r_dat <= r_gpio_spec_out(63 downto 32);
          when c_gpio_spec_out_reset_low_reg  => r_dat <= r_gpio_spec_out(31 downto  0);
          when c_gpio_spec_out_reset_high_reg => r_dat <= r_gpio_spec_out(63 downto 32);
          when c_lvds_spec_in_set_low_reg     => r_dat <= r_lvds_spec_in(31 downto  0);
          when c_lvds_spec_in_set_high_reg    => r_dat <= r_lvds_spec_in(63 downto 32);
          when c_lvds_spec_in_reset_low_reg   => r_dat <= r_lvds_spec_in(31 downto  0);
          when c_lvds_spec_in_reset_high_reg  => r_dat <= r_lvds_spec_in(63 downto 32);
          when c_lvds_spec_out_set_low_reg    => r_dat <= r_lvds_spec_out(31 downto  0);
          when c_lvds_spec_out_set_high_reg   => r_dat <= r_lvds_spec_out(63 downto 32);
          when c_lvds_spec_out_reset_low_reg  => r_dat <= r_lvds_spec_out(31 downto  0);
          when c_lvds_spec_out_reset_high_reg => r_dat <= r_lvds_spec_out(63 downto 32);
          when c_gpio_mux_set_low_reg         => r_dat <= r_gpio_mux(31 downto  0);
          when c_gpio_mux_set_high_reg        => r_dat <= r_gpio_mux(63 downto 32);
          when c_gpio_mux_reset_low_reg       => r_dat <= r_gpio_mux(31 downto  0);
          when c_gpio_mux_reset_high_reg      => r_dat <= r_gpio_mux(63 downto 32);
          when c_lvds_mux_set_low_reg         => r_dat <= r_lvds_mux(31 downto  0);
          when c_lvds_mux_set_high_reg        => r_dat <= r_lvds_mux(63 downto 32);
          when c_lvds_mux_reset_low_reg       => r_dat <= r_lvds_mux(31 downto  0);
          when c_lvds_mux_reset_high_reg      => r_dat <= r_lvds_mux(63 downto 32);
          when c_gpio_sel_set_low_reg         => r_dat <= r_gpio_sel(31 downto  0);
          when c_gpio_sel_set_high_reg        => r_dat <= r_gpio_sel(63 downto 32);
          when c_gpio_sel_reset_low_reg       => r_dat <= r_gpio_sel(31 downto  0);
          when c_gpio_sel_reset_high_reg      => r_dat <= r_gpio_sel(63 downto 32);
          when c_lvds_sel_set_low_reg         => r_dat <= r_lvds_sel(31 downto  0);
          when c_lvds_sel_set_high_reg        => r_dat <= r_lvds_sel(63 downto 32);
          when c_lvds_sel_reset_low_reg       => r_dat <= r_lvds_sel(31 downto  0);
          when c_lvds_sel_reset_high_reg      => r_dat <= r_lvds_sel(63 downto 32);
          when c_gpio_pps_mux_set_low_reg     => r_dat <= r_gpio_pps_mux(31 downto  0);
          when c_gpio_pps_mux_set_high_reg    => r_dat <= r_gpio_pps_mux(63 downto 32);
          when c_gpio_pps_mux_reset_low_reg   => r_dat <= r_gpio_pps_mux(31 downto  0);
          when c_gpio_pps_mux_reset_high_reg  => r_dat <= r_gpio_pps_mux(63 downto 32);
          when c_lvds_pps_mux_set_low_reg     => r_dat <= r_lvds_pps_mux(31 downto  0);
          when c_lvds_pps_mux_set_high_reg    => r_dat <= r_lvds_pps_mux(63 downto 32);
          when c_lvds_pps_mux_reset_low_reg   => r_dat <= r_lvds_pps_mux(31 downto  0);
          when c_lvds_pps_mux_reset_high_reg  => r_dat <= r_lvds_pps_mux(63 downto 32);
        when others =>
          -- Get driven GPIO OUT values
          if (slave_i.adr(15 downto 2) >= c_set_gpio_out_begin_reg and slave_i.adr(15 downto 2) <= c_set_gpio_out_end_reg) then
            r_dat(31 downto  8) <= (others => '0');
            r_dat( 7 downto  0) <= (others => r_gpio_drive(s_bit_selector));
            report "Getting driven GPIO OUT at: " & integer'image(s_bit_selector) severity note;

          -- Get driven LVDS OUT values
          elsif (slave_i.adr(15 downto 2) >= c_set_lvds_out_begin_reg and slave_i.adr(15 downto 2) <= c_set_lvds_out_end_reg) then
            r_dat(31 downto  8) <= (others => '0');
            r_dat( 7 downto  0) <= r_lvds_drive(s_bit_selector);
            report "Getting driven LVDS OUT at: " & integer'image(s_bit_selector) severity note;

          -- Get GPIO IN values
          elsif (slave_i.adr(15 downto 2) >= c_get_gpio_in_begin_reg and slave_i.adr(15 downto 2) <= c_get_gpio_in_end_reg) then
            r_dat(31 downto  8) <= (others => '0');
            r_dat( 7 downto  0) <= (others => gpio_input_i(s_bit_selector));
            report "Getting GPIO IN at: " & integer'image(s_bit_selector) severity note;

          -- Get GPIO OUT values
          elsif (slave_i.adr(15 downto 2) >= c_get_gpio_out_begin_reg and slave_i.adr(15 downto 2) <= c_get_gpio_out_end_reg) then
            r_dat(31 downto  8) <= (others => '0');
            r_dat( 7 downto  0) <= (others => gpio_output_i(s_bit_selector-g_gpio_in-g_gpio_inout));
            report "Getting GPIO OUT at: " & integer'image(s_bit_selector-g_gpio_in-g_gpio_inout) severity note;

          -- Get LVDS IN values
          elsif (slave_i.adr(15 downto 2) >= c_get_lvds_in_begin_reg and slave_i.adr(15 downto 2) <= c_get_lvds_in_end_reg) then
            r_dat(31 downto  8) <= (others => '0');
            r_dat( 7 downto  0) <= lvds_input_i(s_bit_selector);
            report "Getting LVDS IN at: " & integer'image(s_bit_selector) severity note;

          -- Get LVDS OUT values
          elsif (slave_i.adr(15 downto 2) >= c_get_lvds_out_begin_reg and slave_i.adr(15 downto 2) <= c_get_lvds_out_end_reg) then
            r_dat(31 downto  8) <= (others => '0');
            r_dat( 7 downto  0) <= lvds_output_i(s_bit_selector-g_lvds_in-g_lvds_inout);
            report "Getting LVDS OUT at: " & integer'image(s_bit_selector-g_lvds_in-g_lvds_inout) severity note;

          -- GET IO mapping table
          elsif (slave_i.adr(15 downto 2) >= c_io_map_table_begin_reg and slave_i.adr(15 downto 2) <= c_io_map_table_end_reg) then
              -- Prevent out of range access
              if (s_entry_selector < c_ios_total) then
                case s_field_selector is
                  when 0      => r_dat               <= c_io_table_memory(s_entry_selector).info_name(95 downto 64);
                  when 1      => r_dat               <= c_io_table_memory(s_entry_selector).info_name(63 downto 32);
                  when 2      => r_dat               <= c_io_table_memory(s_entry_selector).info_name(31 downto 0);
                  when others => r_dat(31 downto 26) <= c_io_table_memory(s_entry_selector).info_special;
                                 r_dat(25)           <= c_io_table_memory(s_entry_selector).info_special_out;
                                 r_dat(24)           <= c_io_table_memory(s_entry_selector).info_special_in;
                                 r_dat(23 downto 16) <= c_io_table_memory(s_entry_selector).info_index;
                                 r_dat(15 downto 14) <= c_io_table_memory(s_entry_selector).info_direction;
                                 r_dat(13 downto 11) <= c_io_table_memory(s_entry_selector).info_channel;
                                 r_dat(10)           <= c_io_table_memory(s_entry_selector).info_oe;
                                 r_dat(9)            <= c_io_table_memory(s_entry_selector).info_term;
                                 r_dat(8)            <= c_io_table_memory(s_entry_selector).info_res_bit;
                                 r_dat(7 downto 4)   <= c_io_table_memory(s_entry_selector).info_logic_level;
                                 r_dat(3 downto 0)   <= c_io_table_memory(s_entry_selector).info_reserved;
                end case;
              else
                report "Unknown table access (read)!" severity error;
                r_dat(31 downto 16) <= x"beef";
                r_dat(15 downto  2) <= slave_i.adr(15 downto 2);
                r_dat( 1 downto  0) <= (others => '0');
              end if;
            report "Getting mapping table" severity note;

          -- Unknown access
          else
            if ((slave_i.cyc and slave_i.stb) = '1') then
              report "Unknown register access (read)!" severity error;
              r_dat(31 downto 16) <= x"dead";
              r_dat(15 downto  2) <= slave_i.adr(15 downto 2);
              r_dat( 1 downto  0) <= (others => '0');
            else
              r_dat <= (others => '0');
            end if;

          end if;
      end case;

    end if;
  end process;

end rtl;
