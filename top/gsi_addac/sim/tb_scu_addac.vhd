-- Copyright (C) 1991-2012 Altera Corporation
-- Your use of Altera Corporation's design tools, logic functions 
-- and other software and tools, and its AMPP partner logic 
-- functions, and any output files from any of the foregoing 
-- (including device programming or simulation files), and any 
-- associated documentation or information are expressly subject 
-- to the terms and conditions of the Altera Program License 
-- Subscription Agreement, Altera MegaCore Function License 
-- Agreement, or other applicable license agreement, including, 
-- without limitation, that your use is for the sole purpose of 
-- programming logic devices manufactured by Altera and sold by 
-- Altera or its authorized distributors.  Please refer to the 
-- applicable agreement for further details.

-- ***************************************************************************
-- This file contains a Vhdl test bench template that is freely editable to   
-- suit user's needs .Comments are provided in each section to help the user  
-- fill out necessary details.                                                
-- ***************************************************************************
-- Generated on "02/27/2013 09:19:12"
                                                            
-- Vhdl Test Bench template for design  :  scu_addac
-- 
-- Simulation tool : ModelSim-Altera (VHDL)
-- 

LIBRARY ieee;                                               
USE ieee.std_logic_1164.all;                                
USE ieee.math_real.all;                              
USE IEEE.STD_LOGIC_arith.all;

ENTITY scu_addac_vhd_tst IS
END scu_addac_vhd_tst;
ARCHITECTURE scu_addac_arch OF scu_addac_vhd_tst IS
-- constants
CONSTANT  scu_master_clk_in_hz: INTEGER := 120_000_000;
CONSTANT  master_clk_period:    time := (real(1_000_000_000)/real(scu_master_clk_in_hz/1000)) * 1 ps;
CONSTANT  slave_clk_in_hz:    INTEGER := 125_000_000;
CONSTANT  slave_clk_period:   time := (real(1_000_000_000)/real(slave_clk_in_hz/1000)) * 1 ps;
CONSTANT  A_SysClock_period:  time  := 40.0 * 1 ns;


CONSTANT  Wr:           STD_LOGIC := '1';
CONSTANT  Rd:           STD_LOGIC := '0';
CONSTANT  FPGA_Reg_Acc:     STD_LOGIC_VECTOR(3 DOWNTO 0) := X"0";
CONSTANT  Slave1_Acc:       STD_LOGIC_VECTOR(3 DOWNTO 0) := X"1";
CONSTANT  Slave2_Acc:       STD_LOGIC_VECTOR(3 DOWNTO 0) := X"2";


CONSTANT  C_Slave_ID_Adr:         STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0001";   -- address of slave ident code (rd)
CONSTANT  C_Slave_FW_Version_Adr:     STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0002";   -- address of slave firmware version (rd)
CONSTANT  C_Slave_FW_Release_Adr:     STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0003";   -- address of slave firmware release (rd)
CONSTANT  C_Slave_HW_Version_Adr:     STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0004";   -- address of slave hardware version (rd)
CONSTANT  C_Slave_HW_Release_Adr:     STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0005";   -- address of slave hardware release (rd)
CONSTANT  C_Vers_Revi_of_this_Macro:    STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0006";   -- address of version and revision register of this macro (rd)
CONSTANT  C_Slave_Echo_Reg_Adr:     STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0010";   -- address of slave echo register (rd/wr)
CONSTANT  C_Status_Reg_Adr:       STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0011";   -- address of status register (rd)



CONSTANT	C_Slave_Intr_In_Adr:			STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0020";		-- address of slave interrupt In register (rd)
CONSTANT	C_Slave_Intr_Enable_Adr:		STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0021";		-- address of slave interrupt pending register (rd/wr)
CONSTANT	C_Slave_Intr_Pending_Adr:		STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0022";		-- address of slave interrupt pending register (rd/wr)
CONSTANT	C_Slave_Intr_Mask_Adr:			STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0023";		-- address of slave interrupt mask register (rd/wr)
CONSTANT	C_Slave_Intr_Active_Adr:		STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0024";		-- address of slave interrupt active register (rd)
CONSTANT	C_Slave_Intr_Level_Adr:			STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0025";		-- address of slave interrupt level register (rd/wr)
CONSTANT	C_Slave_Intr_Edge_Adr:			STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0026";		-- address of slave interrupt edge register (rd/wr)

                                                 
-- signals
SIGNAL    Wr_Data:              STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL    Rd_Data:              STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL    Slave_Nr:             STD_LOGIC_VECTOR(3 DOWNTO 0);
signal    Adr:                  STD_LOGIC_VECTOR(15 downto 0);
SIGNAL    scub_wr_active:       STD_LOGIC;
SIGNAL    scub_wr_err_no_dtack: STD_LOGIC;
SIGNAL    scub_rd_active:       STD_LOGIC;
SIGNAL    scub_rd_err_no_dtack: STD_LOGIC;
SIGNAL    Timing_In:            STD_LOGIC_VECTOR(31 DOWNTO 0) := (OTHERS => 'H');
SIGNAL    master_clk:           STD_LOGIC := '0';
SIGNAL    Clk:                  STD_LOGIC := '0';
SIGNAL    Last_Rd_Data:         STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL    Start_Cycle:          STD_LOGIC := 'L';
SIGNAL    Wr_Cycle:             STD_LOGIC := 'L';
SIGNAL    Rd_Cycle:             STD_LOGIC := 'L';
SIGNAL    Start_Timing_Cycle:   STD_LOGIC := '0';
SIGNAL    SCU_Bus_Access_Active:  STD_LOGIC; 
SIGNAL    Intr:                 STD_LOGIC;
SIGNAL    nscub_srq_slaves:     STD_LOGIC_VECTOR(11 DOWNTO 0) := (OTHERS => 'H');
SIGNAL    Reset:                STD_LOGIC := '0';
SIGNAL    nSel_Ext_Data_Drv:    STD_LOGIC;
SIGNAL    scub_rd_fin:          STD_LOGIC;
SIGNAL    scub_wr_fin:          STD_LOGIC;
SIGNAL    nscub_timing_cycle:   STD_LOGIC;
SIGNAL		scub_ti_cyc_err:		  STD_LOGIC;
SIGNAL		scub_ti_fin:			    STD_LOGIC;
SIGNAL		Intr_In:				      STD_LOGIC_VECTOR(15 DOWNTO 1) := (OTHERS => 'H');
  

SIGNAL SCUB_Addr : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL A_ADC_DAC_SEL : STD_LOGIC_VECTOR(3 DOWNTO 0);
SIGNAL SCUB_Data : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL A_Ext_Data_RD : STD_LOGIC;
SIGNAL a_ext_io_7_0_dis : STD_LOGIC;
SIGNAL a_ext_io_15_8_dis : STD_LOGIC;
SIGNAL a_ext_io_23_16_dis : STD_LOGIC;
SIGNAL a_ext_io_31_24_dis : STD_LOGIC;
SIGNAL a_io : STD_LOGIC_VECTOR(31 DOWNTO 0);
SIGNAL a_io_7_0_tx : STD_LOGIC;
SIGNAL a_io_15_8_tx : STD_LOGIC;
SIGNAL a_io_23_16_tx : STD_LOGIC;
SIGNAL a_io_31_24_tx : STD_LOGIC;
SIGNAL A_nADR_EN : STD_LOGIC;
SIGNAL A_nADR_FROM_SCUB : STD_LOGIC;
SIGNAL nSCUB_Slave_Sel : STD_LOGIC_VECTOR(11 downto 0);
SIGNAL nSCUB_DS : STD_LOGIC;
SIGNAL nSCUB_Dtack : STD_LOGIC;
SIGNAL A_nEvent_Str : STD_LOGIC := 'H';
SIGNAL A_nLED : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL A_nReset : STD_LOGIC;
SIGNAL A_nSel_Ext_Data_Drv : STD_LOGIC;
SIGNAL A_nSRQ : STD_LOGIC;
SIGNAL A_nState_LED : STD_LOGIC_VECTOR(2 DOWNTO 0);
SIGNAL SCUB_RDnWR : STD_LOGIC;
SIGNAL A_SEL : STD_LOGIC_VECTOR(3 DOWNTO 0);
SIGNAL A_Spare0 : STD_LOGIC;
SIGNAL A_Spare1 : STD_LOGIC;
SIGNAL A_SysClock : STD_LOGIC := '0';
SIGNAL A_TA : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL A_TB : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL ADC_BUSY : STD_LOGIC;
SIGNAL ADC_CONVST_A : STD_LOGIC;
SIGNAL ADC_CONVST_B : STD_LOGIC;
SIGNAL ADC_DB : STD_LOGIC_VECTOR(15 DOWNTO 0);
SIGNAL ADC_FRSTDATA : STD_LOGIC;
SIGNAL ADC_OS : STD_LOGIC_VECTOR(2 DOWNTO 0);
SIGNAL ADC_Range : STD_LOGIC;
SIGNAL ADC_RESET : STD_LOGIC;
--SIGNAL slave_clk_period : STD_LOGIC;
SIGNAL DAC1_SDI : STD_LOGIC;
SIGNAL DAC2_SDI : STD_LOGIC;
SIGNAL nADC_CS : STD_LOGIC;
SIGNAL nADC_PAR_SER_SEL : STD_LOGIC;
SIGNAL nADC_RD_SCLK : STD_LOGIC;
SIGNAL nDAC1_A0 : STD_LOGIC;
SIGNAL nDAC1_A1 : STD_LOGIC;
SIGNAL nDAC1_CLK : STD_LOGIC;
SIGNAL nDAC1_CLR : STD_LOGIC;
SIGNAL nDAC2_A0 : STD_LOGIC;
SIGNAL nDAC2_A1 : STD_LOGIC;
SIGNAL nDAC2_CLK : STD_LOGIC;
SIGNAL nDAC2_CLR : STD_LOGIC;

COMPONENT scu_addac
  PORT (
  A_A : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
  A_ADC_DAC_SEL : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
  A_D : INOUT STD_LOGIC_VECTOR(15 DOWNTO 0);
  A_Ext_Data_RD : OUT STD_LOGIC;
  a_ext_io_7_0_dis : OUT STD_LOGIC;
  a_ext_io_15_8_dis : OUT STD_LOGIC;
  a_ext_io_23_16_dis : OUT STD_LOGIC;
  a_ext_io_31_24_dis : OUT STD_LOGIC;
  a_io : INOUT STD_LOGIC_VECTOR(31 DOWNTO 0);
  a_io_7_0_tx : OUT STD_LOGIC;
  a_io_15_8_tx : OUT STD_LOGIC;
  a_io_23_16_tx : OUT STD_LOGIC;
  a_io_31_24_tx : OUT STD_LOGIC;
  A_nADR_EN : OUT STD_LOGIC;
  A_nADR_FROM_SCUB : OUT STD_LOGIC;
  A_nBoardSel : IN STD_LOGIC;
  A_nDS : IN STD_LOGIC;
  A_nDtack : OUT STD_LOGIC;
  A_nEvent_Str : IN STD_LOGIC;
  A_nLED : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
  A_nReset : IN STD_LOGIC;
  A_nSel_Ext_Data_Drv : OUT STD_LOGIC;
  A_nSRQ : OUT STD_LOGIC;
  A_nState_LED : OUT STD_LOGIC_VECTOR(2 DOWNTO 0);
  A_RnW : IN STD_LOGIC;
  A_SEL : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
  A_Spare0 : IN STD_LOGIC;
  A_Spare1 : IN STD_LOGIC;
  A_SysClock : IN STD_LOGIC := '0';
  A_TA : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
  A_TB : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
  ADC_BUSY : IN STD_LOGIC;
  ADC_CONVST_A : BUFFER STD_LOGIC;
  ADC_CONVST_B : BUFFER STD_LOGIC;
  ADC_DB : INOUT STD_LOGIC_VECTOR(15 DOWNTO 0);
  ADC_FRSTDATA : IN STD_LOGIC;
  ADC_OS : BUFFER STD_LOGIC_VECTOR(2 DOWNTO 0);
  ADC_Range : BUFFER STD_LOGIC;
  ADC_RESET : BUFFER STD_LOGIC;
  CLK_FPGA : IN STD_LOGIC;
  DAC1_SDI : BUFFER STD_LOGIC;
  DAC2_SDI : BUFFER STD_LOGIC;
  nADC_CS : BUFFER STD_LOGIC;
  nADC_PAR_SER_SEL : BUFFER STD_LOGIC;
  nADC_RD_SCLK : BUFFER STD_LOGIC;
  nDAC1_A0 : BUFFER STD_LOGIC;
  nDAC1_A1 : BUFFER STD_LOGIC;
  nDAC1_CLK : BUFFER STD_LOGIC;
  nDAC1_CLR : BUFFER STD_LOGIC;
  nDAC2_A0 : BUFFER STD_LOGIC;
  nDAC2_A1 : BUFFER STD_LOGIC;
  nDAC2_CLK : BUFFER STD_LOGIC;
  nDAC2_CLR : BUFFER STD_LOGIC;
   EXT_TRIG_DAC: in std_logic;
   EXT_TRIG_ADC: in std_logic;
    HW_REV:               in    std_logic_vector(3 downto 0);
    A_MODE_SEL:           in    std_logic_vector(1 downto 0)
   
   
  );
END COMPONENT;

component SCU_Bus_Master
GENERIC (
    CLK_in_Hz     : INTEGER := 100000000;
    Time_Out_in_ns    : INTEGER := 250;
    Sel_dly_in_ns   : INTEGER := 30;              -- delay to the I/O pins is not included
    Sel_release_in_ns : INTEGER := 30;              -- delay to the I/O pins is not included
    D_Valid_to_DS_in_ns : INTEGER := 30;              -- delay to the I/O pins is not included
    Timing_str_in_ns  : INTEGER := 80;              -- delay to the I/O pins is not included
    Test        : INTEGER := 0
    );

PORT  (
    Wr_Data         : IN    STD_LOGIC_VECTOR(15 DOWNTO 0);    -- wite data to SCU_Bus, or internal FPGA register
    Rd_Data         : OUT   STD_LOGIC_VECTOR(15 DOWNTO 0);    -- read data from SCU_Bus, or internal FPGA register
    Adr           : IN    STD_LOGIC_VECTOR(15 DOWNTO 0);
    Slave_Nr        : IN    STD_LOGIC_VECTOR(3 DOWNTO 0);   -- 0x0 => internal access, 0x1 to 0xC => slave 1 to 12 access
    Timing_In       : IN    STD_LOGIC_VECTOR(31 DOWNTO 0);
    Start_Cycle       : IN    STD_LOGIC;              -- start data access from/to SCU_Bus
    Wr_Cycle        : IN    STD_LOGIC;              -- direction of SCU_Bus data access, write is active high.
    Rd_Cycle        : IN    STD_LOGIC;              -- direction of SCU_Bus data access, read is active high.
    Start_Timing_Cycle    : IN    STD_LOGIC;              -- start timing cycle to SCU_Bus
    clk           : IN    STD_LOGIC;
    Reset         : IN    STD_LOGIC;
    SCU_Bus_Access_Active : OUT   STD_LOGIC;
    Intr          : OUT   STD_LOGIC;              -- One or more slave interrupts, or internal Interrupts (like
                                        -- SCU_Bus-busy or SCU_Bus-timeout) are active. Intr is ative high.
    SCUB_Data       : INOUT   STD_LOGIC_VECTOR(15 DOWNTO 0);
    nSCUB_DS        : OUT   STD_LOGIC;              -- SCU_Bus Data Strobe, low active.
    nSCUB_Dtack       : IN    STD_LOGIC;              -- SCU_Bus Data Acknowledge, low active.
    SCUB_Addr       : OUT   STD_LOGIC_VECTOR(15 DOWNTO 0);    -- Address Bus of SCU_Bus
    SCUB_RDnWR        : OUT   STD_LOGIC;              -- Read/Write Signal of SCU_Bus. Read is aktive high.
                                        -- Direction seen from this marco.
    nSCUB_SRQ_Slaves    : IN    STD_LOGIC_VECTOR(11 DOWNTO 0);    -- Input of service requests up to 12 SCU_Bus slaves, active low.
    nSCUB_Slave_Sel     : OUT   STD_LOGIC_VECTOR(11 DOWNTO 0);    -- Output select one or more of 12 SCU_Bus slaves, active low.
    nSCUB_Timing_Cycle    : OUT   STD_LOGIC;              -- Strobe to signal a timing cycle on SCU_Bus, active low.
    nSel_Ext_Data_Drv   : OUT   STD_LOGIC;              -- select for external data transceiver to the SCU_Bus, active low.
  
    SCUB_Rd_Err_no_Dtack  : OUT   STD_LOGIC;
    SCUB_Rd_Fin       : OUT   STD_LOGIC;
    SCUB_Rd_active      : OUT   STD_LOGIC;
    SCUB_Wr_Err_no_Dtack  : OUT   STD_LOGIC;
    SCUB_Wr_Fin       : OUT   STD_LOGIC;
    SCUB_Wr_active      : OUT   STD_LOGIC;
    SCUB_Ti_Cyc_Err     : OUT   STD_LOGIC;
    SCUB_Ti_Fin       : OUT   STD_LOGIC
    );
end component;



Procedure Wait_DT ( SIGNAL nSCUB_Dtack : IN STD_LOGIC;
          SIGNAL scub_wr_err_no_dtack, scub_rd_err_no_dtack : IN STD_LOGIC
          ) IS
  VARIABLE time_out:  INTEGER;
  BEGIN
    time_out := 0;
    WHILE ((nSCUB_Dtack /= '0') and (time_out < 600))
    LOOP WAIT FOR 1 ns;
        time_out := time_out + 1;
    END LOOP;
  IF time_out = 600 THEN
   ASSERT FALSE REPORT "no dtack " SEVERITY warning;
  ELSE
    time_out := 0;
    WHILE ((nSCUB_Dtack = '0') and (time_out < 600))
    LOOP WAIT FOR 1 ns;
      time_out := time_out + 1;
    END LOOP;
    IF time_out = 600 THEN
     ASSERT FALSE REPORT "dtack is 600 ns ative" SEVERITY warning;
    END IF;
   END IF;
  END Wait_DT;
  

Procedure master_free_for_data_trans (SIGNAL SCUB_Wr_active, SCUB_Rd_active : IN STD_LOGIC) IS
  BEGIN
    WHILE (SCUB_Wr_active = '1') OR (SCUB_Rd_active = '1')
    LOOP WAIT FOR 1 ns; END LOOP;
  END master_free_for_data_trans;

CONSTANT  C_Master_Rd_Wr_Status_Adr:    STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0000";   -- address of master status register (rd-wr)
CONSTANT  C_Global_Intr_Ena_Adr:      STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0001";
CONSTANT  C_Master_Version_Revision_Adr:  STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0002";   -- address of master version revision register (rd-only)
CONSTANT  C_Master_SRQ_Ena_Adr:     STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0003";   -- address of master SRQ enable register (rd-wr).
                                              -- Data bit(0) to bit(11) represent Slave1 to Slave12. A one at the
                                              -- specific data bit indicate that the interrupt of the korrospondent slave is enabled.
CONSTANT  C_Master_SRQ_Active_Adr:    STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0004";   -- address of master SRQ active register (rd-only).
CONSTANT  C_Master_SRQ_In_Adr:      STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0005";   -- address of master SRQ in register (rd-only).
CONSTANT  C_Master_Wr_Multi_Slave_Sel_Adr:STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0006";   -- address of master SRQ in register (rd-only).
CONSTANT  C_Master_intern_Echo_1_Adr:   STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0007";   -- address of master SRQ in register (rd-only).
CONSTANT  C_Master_intern_Echo_2_Adr:   STD_LOGIC_VECTOR(15 DOWNTO 0) := X"0008";   -- address of master SRQ in register (rd-only).




Procedure Computer_access_master  (
                  Slave_Nr_from_Comp        : IN  STD_LOGIC_VECTOR(Slave_Nr'range);
                  Adr_from_Comp           : IN  STD_LOGIC_VECTOR(ADR'range);
                  Wr_Data_from_Comp         : IN  STD_LOGIC_VECTOR(Wr_Data'range);
                  Wr_Cycle_from_Comp        : IN  STD_LOGIC;
                  SIGNAL Slave_Nr     : OUT STD_LOGIC_VECTOR(Slave_Nr'range);
                  SIGNAL Adr        : OUT STD_LOGIC_VECTOR(Adr'range);
                  SIGNAL Wr_Data      : OUT STD_LOGIC_VECTOR(Wr_Data'range);
                  SIGNAL Wr_Cycle  : OUT STD_LOGIC;
                  SIGNAL Rd_Cycle  : OUT STD_LOGIC;
                  SIGNAL Start_Cycle : OUT STD_LOGIC;
                  SIGNAL nSCUB_Dtack    : IN  STD_LOGIC 
                  ) IS
  BEGIN
  IF Slave_Nr_from_Comp /= X"0" THEN
    master_free_for_data_trans(SCUB_Wr_active, SCUB_Rd_active); -- wait SCU bus macro free for data transfer
  END IF;
  Slave_Nr <= Slave_Nr_from_Comp;  
  ADR <= Adr_from_Comp;
  Wr_Data <= Wr_Data_from_Comp;
  IF Wr_Cycle_from_Comp = Wr THEN
    Wr_Cycle <= '1';
    Rd_Cycle <= '0';
  ELSE
    Wr_Cycle <= '0';
    Rd_Cycle <= '1';
  END IF;
  Wait for master_clk_period;
  Start_Cycle <= '1';
  Wait for master_clk_period;
  Start_Cycle <= '0';
  IF Slave_Nr_from_Comp /= X"0" THEN
    Wait_DT(nSCUB_Dtack, scub_wr_err_no_dtack, scub_rd_err_no_dtack);
    Wr_Cycle <= '0';
    Rd_Cycle <= '0';
    Slave_Nr <= FPGA_Reg_Acc;
  END IF;
  Wr_Cycle <= '0';
  Rd_Cycle <= '0';
  Slave_Nr <= FPGA_Reg_Acc;
  wait for 20 ns;
  END Computer_access_master;


PROCEDURE Timing (Timing_Pattern : IN STD_LOGIC_VECTOR(Timing_In'range);
                  SIGNAL Timing_In : OUT STD_LOGIC_VECTOR(Timing_In'range);
                  SIGNAL Start_Timing_Cycle : OUT STD_LOGIC
                  ) IS
  BEGIN
    Timing_IN <= Timing_Pattern;
    Wait for master_clk_period;
    Start_Timing_Cycle <= '1';
    Wait for master_clk_period;
    Start_Timing_Cycle <= '0';
  END Timing;
    


BEGIN

P_Fair_Bus_Err: PROCESS (SCUB_Wr_Err_no_Dtack,SCUB_Rd_Err_no_Dtack)
  BEGIN
  IF rising_edge(SCUB_Wr_Err_no_Dtack) THEN ASSERT FALSE REPORT "FB write error: no dtack " SEVERITY warning; END IF;
  IF rising_edge(SCUB_Rd_Err_no_Dtack) THEN ASSERT FALSE REPORT "FB read error: no dtack " SEVERITY warning; END IF;
  END PROCESS P_Fair_Bus_Err;

    
P_master_clk: Process
  begin
    loop
         master_clk <= NOT master_clk;
         wait for master_clk_period / 2;
    end loop;
  end process P_master_clk;

P_clk: Process
  begin
    loop
         clk <= NOT clk;
         wait for slave_clk_period / 2;
    end loop;
  end process P_clk;
  
P_SysClock: Process
  begin
    loop
         A_SysClock <= NOT A_SysClock;
         wait for A_SysClock_period / 2;
    end loop;
  end process P_SysClock;

Last_Rd_Mem: Process (nSCUB_Dtack, scub_rd_err_no_dtack, SCUB_Data)
  BEGIN
    IF nSCUB_Dtack = '0' AND SCUB_RDnWR = '1' THEN
      Last_Rd_Data <= SCUB_Data;
    ELSIF scub_rd_err_no_dtack = '1' THEN
      Last_Rd_Data <= (OTHERS => 'X');
    END IF;
  END Process Last_Rd_Mem;
  
  
SCU_M:  SCU_Bus_Master
GENERIC MAP(
      CLK_in_Hz     => SCU_master_clk_in_hz,
      D_Valid_to_DS_in_ns => 30,
      Sel_dly_in_ns   => 30,
      Sel_release_in_ns => 30,
      Test        => 0,
      Time_Out_in_ns    => 250,
      Timing_str_in_ns  => 80
      )
PORT MAP(
    Wr_Data         => Wr_Data,
    Rd_Data         => Rd_Data,
    Adr           => Adr,
    Slave_Nr        => Slave_Nr,
    Start_Cycle       => Start_Cycle,
    Wr_Cycle        => Wr_Cycle,
    Rd_Cycle        => Rd_Cycle,
    Timing_In       => Timing_In,
    Start_Timing_Cycle    => Start_Timing_Cycle,
    clk           => master_clk,
    Reset         => Reset,
    SCU_Bus_Access_Active => SCU_Bus_Access_Active,
    Intr          => Intr,
  
    SCUB_Data       => SCUB_Data,
    nSCUB_DS        => nSCUB_DS,
    nSCUB_Dtack       => nSCUB_Dtack,
    SCUB_Addr       => SCUB_Addr,
    SCUB_RDnWR        => SCUB_RDnWR,
  
    nSCUB_SRQ_Slaves    => nSCUB_SRQ_Slaves,
    nSCUB_Slave_Sel     => nSCUB_Slave_Sel,
    nSCUB_Timing_Cycle    => nSCUB_Timing_Cycle,
    nSel_Ext_Data_Drv   => nSel_Ext_Data_Drv,
  
    SCUB_Rd_Err_no_Dtack  => SCUB_Rd_Err_no_Dtack,
    SCUB_Rd_Fin       => SCUB_Rd_Fin,
    SCUB_Rd_active      => SCUB_Rd_active,
    SCUB_Wr_Err_no_Dtack  => SCUB_Wr_Err_no_Dtack,
    SCUB_Wr_Fin       => SCUB_Wr_Fin,
    SCUB_Wr_active      => SCUB_Wr_active,
    SCUB_Ti_Cyc_Err     => SCUB_Ti_Cyc_Err,
    SCUB_Ti_Fin       => SCUB_Ti_Fin
    );
    
  adda1 : scu_addac
  PORT MAP (
-- list connections between master ports and signals
  A_A => SCUB_Addr,
  A_ADC_DAC_SEL => A_ADC_DAC_SEL,
  A_D => SCUB_Data,
  A_Ext_Data_RD => A_Ext_Data_RD,
  a_ext_io_7_0_dis => a_ext_io_7_0_dis,
  a_ext_io_15_8_dis => a_ext_io_15_8_dis,
  a_ext_io_23_16_dis => a_ext_io_23_16_dis,
  a_ext_io_31_24_dis => a_ext_io_31_24_dis,
  a_io => a_io,
  a_io_7_0_tx => a_io_7_0_tx,
  a_io_15_8_tx => a_io_15_8_tx,
  a_io_23_16_tx => a_io_23_16_tx,
  a_io_31_24_tx => a_io_31_24_tx,
  A_nADR_EN => A_nADR_EN,
  A_nADR_FROM_SCUB => A_nADR_FROM_SCUB,
  A_nBoardSel => nSCUB_Slave_Sel(0),
  A_nDS => nSCUB_DS,
  A_nDtack => nSCUB_Dtack,
  A_nEvent_Str => nSCUB_Timing_Cycle,
  A_nLED => A_nLED,
  A_nReset => A_nReset,
  A_nSel_Ext_Data_Drv => A_nSel_Ext_Data_Drv,
  A_nSRQ => A_nSRQ,
  A_nState_LED => A_nState_LED,
  A_RnW => SCUB_RDnWR,
  A_SEL => A_SEL,
  A_Spare0 => A_Spare0,
  A_Spare1 => A_Spare1,
  A_SysClock => A_SysClock,
  A_TA => A_TA,
  A_TB => A_TB,
  ADC_BUSY => ADC_BUSY,
  ADC_CONVST_A => ADC_CONVST_A,
  ADC_CONVST_B => ADC_CONVST_B,
  ADC_DB => ADC_DB,
  ADC_FRSTDATA => ADC_FRSTDATA,
  ADC_OS => ADC_OS,
  ADC_Range => ADC_Range,
  ADC_RESET => ADC_RESET,
  CLK_FPGA => clk,
  DAC1_SDI => DAC1_SDI,
  DAC2_SDI => DAC2_SDI,
  nADC_CS => nADC_CS,
  nADC_PAR_SER_SEL => nADC_PAR_SER_SEL,
  nADC_RD_SCLK => nADC_RD_SCLK,
  nDAC1_A0 => nDAC1_A0,
  nDAC1_A1 => nDAC1_A1,
  nDAC1_CLK => nDAC1_CLK,
  nDAC1_CLR => nDAC1_CLR,
  nDAC2_A0 => nDAC2_A0,
  nDAC2_A1 => nDAC2_A1,
  nDAC2_CLK => nDAC2_CLK,
  nDAC2_CLR => nDAC2_CLR,
     EXT_TRIG_DAC => '0',
   EXT_TRIG_ADC => '0',
    HW_REV => "0000",
    A_MODE_SEL => "00"
  );




always : PROCESS                                              
-- optional sensitivity list                                  
-- (        )                                                 
-- variable declarations                                      
BEGIN
	ASSERT FALSE REPORT "master clock period is %" & time'image(master_clk_period) SEVERITY warning;
	ASSERT FALSE REPORT "slave clock period is %" & time'image(slave_clk_period) SEVERITY warning;

  
 	Intr_In(15 DOWNTO 1) <= B"1111_1111_1111_111";
                                                    
	Timing_In <= (OTHERS => 'H');
	Start_Timing_Cycle <= '0';
	Start_Cycle <= 'L';
 	Reset <= '1';

 	A_SEL <= X"1";

 	A_nReset <= '0';
	wait for 1200 ns;
 	A_nReset <= '1';
 	Reset <= '0';
	wait for 800 ns;
	
  Timing( X"DEADDEAD", Timing_In, Start_Timing_Cycle);
  wait for 500 ns;
                                                           
  Computer_access_master(Slave1_Acc, C_Slave_Intr_Active_Adr, X"0000", Rd, Slave_Nr, Adr, Wr_Data, Wr_Cycle, Rd_Cycle, Start_Cycle, nSCUB_Dtack);
  Computer_access_master(Slave1_Acc, C_Slave_Intr_Active_Adr, Last_Rd_Data, Wr, Slave_Nr, Adr, Wr_Data, Wr_Cycle, Rd_Cycle, Start_Cycle, nSCUB_Dtack);
 	ASSERT FALSE REPORT "read and quitt power up intr (bit(0))" SEVERITY warning;
  Wait for 50 ns;

  Computer_access_master(Slave1_Acc, x"0010", X"5A0F", wr, Slave_Nr, Adr, Wr_Data, Wr_Cycle, Rd_Cycle, Start_Cycle, nSCUB_Dtack);
  Computer_access_master(Slave1_Acc, x"0010", X"005f", rd, Slave_Nr, Adr, Wr_Data, Wr_Cycle, Rd_Cycle, Start_Cycle, nSCUB_Dtack);
  
  Computer_access_master(Slave1_Acc, x"0220", X"005f", wr, Slave_Nr, Adr, Wr_Data, Wr_Cycle, Rd_Cycle, Start_Cycle, nSCUB_Dtack);
  Computer_access_master(Slave1_Acc, x"0220", X"005f", rd, Slave_Nr, Adr, Wr_Data, Wr_Cycle, Rd_Cycle, Start_Cycle, nSCUB_Dtack);
  
  Computer_access_master(Slave1_Acc, x"0200", X"0002", wr, Slave_Nr, Adr, Wr_Data, Wr_Cycle, Rd_Cycle, Start_Cycle, nSCUB_Dtack);
  Computer_access_master(Slave1_Acc, x"0201", X"1234", wr, Slave_Nr, Adr, Wr_Data, Wr_Cycle, Rd_Cycle, Start_Cycle, nSCUB_Dtack);
	wait for 2 us;	
  Computer_access_master(Slave1_Acc, x"0201", X"aaaa", wr, Slave_Nr, Adr, Wr_Data, Wr_Cycle, Rd_Cycle, Start_Cycle, nSCUB_Dtack);
	wait for 2 us;	
  Computer_access_master(Slave1_Acc, x"0201", X"5555", wr, Slave_Nr, Adr, Wr_Data, Wr_Cycle, Rd_Cycle, Start_Cycle, nSCUB_Dtack);
	wait for 2 us;	

	ASSERT FALSE REPORT "testbench finished" SEVERITY failure;
                                                       
END PROCESS always;                                          
END scu_addac_arch;
