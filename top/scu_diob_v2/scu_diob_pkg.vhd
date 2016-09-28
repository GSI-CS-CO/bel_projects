library ieee;  
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.gencores_pkg.all;
use work.scu_bus_slave_pkg.all;

package scu_diob_pkg is

  TYPE    t_IO_Reg_1_to_7_Array     is array (1 to 7)  of std_logic_vector(15 downto 0);
  TYPE    t_IO_Reg_0_to_7_Array     is array (0 to 7)  of std_logic_vector(15 downto 0);
  
  constant clk_switch_status_cntrl_addr:  unsigned := x"0030";
  constant c_lm32_ow_Base_Addr:           unsigned(15 downto 0):=  x"0040";   -- housekeeping/LM32

  constant c_ADDAC_Base_addr:             integer := 16#0200#;                -- ADDAC (DAC = x"0200", ADC = x"0230")
  constant c_io_port_Base_Addr:           unsigned(15 downto 0):=  x"0220";   -- 4x8 Bit (ADDAC FG900.161)
  constant c_fg_1_Base_Addr:              unsigned(15 downto 0):=  x"0300";   -- FG1
  constant c_tmr_Base_Addr:               unsigned(15 downto 0):=  x"0330";   -- Timer
  constant c_fg_2_Base_Addr:              unsigned(15 downto 0):=  x"0340";   -- FG2

  constant c_Conf_Sts1_Base_Addr:         integer := 16#0500#;                -- Status-Config-Register
  constant c_AW_Port1_Base_Addr:          integer := 16#0510#;                -- Anwender I/O-Register
  constant c_INL_xor1_Base_Addr:          integer := 16#0530#;                -- Interlock-Pegel-Register
  constant c_INL_msk1_Base_Addr:          integer := 16#0540#;                -- Interlock-Masken-Register
  constant c_Tag_Ctrl1_Base_Addr:         integer := 16#0580#;                -- Tag-Steuerung
  constant c_IOBP_Masken_Base_Addr:       integer := 16#0600#;                -- IO-Backplane Masken-Register
  
  constant module_count:            natural := 15;
  constant pio_max:                 natural := 142;
  constant pio_min:                 natural := 16;
  constant pio_size:                natural := pio_max - pio_min + 1;
  
  type bit_vector_array is array (0 to module_count-1) of std_logic_vector(pio_max downto pio_min);
 
  type t_pio_bit_vectors is record
    vect_in:         std_logic_vector(pio_max downto pio_min);
    vect_out:        std_logic_vector(pio_max downto pio_min);
    vect_en:         std_logic_vector(pio_max downto pio_min);
    irq:             std_logic_vector(15 downto 1);
  end record t_pio_bit_vectors;
  
  type t_pio_bit_vectors_array is array (0 to module_count-1) of t_pio_bit_vectors;
  signal pio_bit_vectors:           t_pio_bit_vectors_array;
  
 
  type id_cid is record
    id:     std_logic_vector(7 downto 0); -- Piggy-ID(Codierung)
    cid:    integer range 0 to 16#ffff#;  -- CID(extension card: cid_system)
    name:   string(1 to 13);              -- module name
  end record;
  type t_module_array is array(natural range <>) of id_cid;
  
  
end scu_diob_pkg;
