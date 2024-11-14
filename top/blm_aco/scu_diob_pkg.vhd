library ieee;  
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

--library work;
--use work.gencores_pkg.all;

package scu_diob_pkg is

  TYPE    t_IO_Reg_1_to_7_Array     is array (1 to 7)  of std_logic_vector(15 downto 0);
  TYPE    t_IO_Reg_0_to_7_Array     is array (0 to 7)  of std_logic_vector(15 downto 0);
  TYPE    t_led_array               is array (1 to 12) of std_logic_vector(6 downto 1);
  TYPE    t_id_array                is array (1 to 12) of std_logic_vector(7 downto 0);
  TYPE    t_IOBP_array              is array (1 to 12) of std_logic_vector(5 downto 0);
  TYPE    t_IO_Reg_0_to_4_Array     is array (0 to 4)  of std_logic_vector(15 downto 0);
  TYPE    t_in_array                is array (0 to 8) of std_logic_vector(5 downto 0);
  TYPE    t_gate_counter_in_Array   is array (0 to 5) of std_logic_vector(11 downto 0);
  TYPE    t_IO_Reg_1_to_8_Array     is array (1 to 8)  of std_logic_vector(15 downto 0);
  TYPE    t_BLM_reg_Array           is array (0 to 127) of std_logic_vector(15 downto 0);
  TYPE    t_BLM_data_Array           is array (0 to 63) of std_logic_vector(15 downto 0);
  TYPE    t_BLM_th_Array           is array (0 to 127) of std_logic_vector(31 downto 0);
  TYPE    t_IO_Reg_0_to_2_Array     is array (0 to 2)  of std_logic_vector(15 downto 0);
  TYPE    t_BLM_out_Array           is array (0 to 127) of std_logic_vector(5 downto 0);
  TYPE    t_BLM_12_to_6_mux_sel_array   is array (0 to 5) of std_logic_vector(3 downto 0);
  --TYPE    t_BLM_or_mux_sel_array is array (0 to 5) of  std_logic_vector(4 downto 0);
  --TYPE    t_BLM_wd_mux_sel_array  is array (0 to 5) of  std_logic_vector(5 downto 0);
  TYPE    t_BLM_mux_reg_Array     is array (0 to 5) of std_logic_vector(15 downto 0);
  --TYPE  t_BLM_out_reg_Array    is array (0 to 191) of std_logic_vector(15 downto 0);
    TYPE  t_BLM_out_sel_reg_Array    is array (0 to 125) of std_logic_vector(15 downto 0);
 -- TYPE t_BLM_reg_Array is array (natural range <>) of std_logic_vector(15 downto 0);
 TYPE     t_IO_Reg_0_to_25_Array is array(0 to 25) of std_logic_vector(15 downto 0);
 TYPE     t_IO_Reg_0_to_27_Array     is array (0 to 27)  of std_logic_vector(15 downto 0);
 TYPE     t_BLM_gate_hold_Time_Array  is array (0 to 11)  of std_logic_vector(15 downto 0);
 TYPE     t_IO_Reg_0_to_15_Array     is array (0 to 15)  of std_logic_vector(15 downto 0);
 TYPE     t_IO_Reg_0_to_3_Array     is array (0 to 3)  of std_logic_vector(15 downto 0);
 TYPE     t_IO_Reg_0_to_8_Array     is array (0 to 8)  of std_logic_vector(15 downto 0);
 TYPE     t_IO_Reg_0_to_32_Array  is array (0 to 32)  of std_logic_vector(15 downto 0);
 TYPE     t_BLM_in_sel_Array           is array (0 to 15) of std_logic_vector(15 downto 0);
 TYPE     t_BLM_counter_Array           is array (0 to 127) of std_logic_vector(31 downto 0);
 TYPE     t_gate_state_nr is array (0 to 11) of std_logic_vector(2 downto 0);
 TYPE     t_IO_Reg_0_to_29_Array is array(0 to 29) of std_logic_vector(15 downto 0);
 TYPE     t_IO_Reg_0_to_1_Array     is array (0 to 1)  of std_logic_vector(15 downto 0);
 TYPE     t_IO_Reg_0_to_31_Array is array (0 to 31)  of std_logic_vector(15 downto 0);
 TYPE     t_group_Array 		is array (0 to 127) of std_logic_vector(3 downto 0);
 
 type    local_set_thr_array        is array (0 to 4095) of  std_logic_vector(31 downto 0);
end scu_diob_pkg;
