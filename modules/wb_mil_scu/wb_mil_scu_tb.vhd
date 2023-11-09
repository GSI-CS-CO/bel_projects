----------------------------------------------------------------------
--! @brief   Testbench for block mode operation.
--! 
----------------------------------------------------------------------
--! @author Jernej Kokalj, Cosylab (jernej.kokalj@gcosylab.com)
--!
--! @date September 8 2023 created
--! @date September 8 2023 last modify
--!
--! @version 1.00
----------------------------------------------------------------------
--! @par Modifications:
--! 
----------------------------------------------------------------------
--! @todo /
----------------------------------------------------------------------
--! @file wb_mil_scu_tb.vhd
----------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use std.textio.all;
use ieee.std_logic_textio.all;

use work.wishbone_pkg.all;

----------------------------------------------------------------------
entity wb_mil_scu_tb is

end entity wb_mil_scu_tb;
----------------------------------------------------------------------
architecture testbench of wb_mil_scu_tb is

  -- Testbench DUT generics
  constant Clk_in_Hz       : INTEGER := 62_500_000;
  constant slave_i_adr_max : INTEGER := 14;

  -- Testbench DUT ports
  signal clk_i               : STD_LOGIC := '0';
  signal nRst_i              : STD_LOGIC := '0';
  signal slave_i             : t_wishbone_slave_in;
  signal slave_o             : t_wishbone_slave_out;
  signal nME_BOO             : STD_LOGIC := '0';
  signal nME_BZO             : STD_LOGIC := '0';
  signal ME_SD               : STD_LOGIC := '0';
  signal ME_ESC              : STD_LOGIC := '0';
  signal ME_SDI              : STD_LOGIC;
  signal ME_EE               : STD_LOGIC;
  signal ME_SS               : STD_LOGIC;
  signal ME_BOI              : STD_LOGIC;
  signal ME_BZI              : STD_LOGIC;
  signal ME_UDI              : STD_LOGIC;
  signal ME_CDS              : STD_LOGIC := '0';
  signal ME_SDO              : STD_LOGIC := '0';
  signal ME_DSC              : STD_LOGIC := '0';
  signal ME_VW               : STD_LOGIC := '0';
  signal ME_TD               : STD_LOGIC := '0';
  signal Mil_BOI             : STD_LOGIC := '0';
  signal Mil_BZI             : STD_LOGIC := '0';
  signal Sel_Mil_Drv         : STD_LOGIC;
  signal nSel_Mil_Rcv        : STD_LOGIC;
  signal Mil_nBOO            : STD_LOGIC;
  signal Mil_nBZO            : STD_LOGIC;
  signal nLed_Mil_Rcv        : STD_LOGIC;
  signal nLed_Mil_Trm        : STD_LOGIC;
  signal nLed_Mil_Err        : STD_LOGIC;
  signal error_limit_reached : STD_LOGIC;
  signal Mil_Decoder_Diag_p  : STD_LOGIC_VECTOR(15 DOWNTO 0);
  signal Mil_Decoder_Diag_n  : STD_LOGIC_VECTOR(15 DOWNTO 0);
  signal timing              : STD_LOGIC := '0';
  signal nLed_Timing         : STD_LOGIC;
  signal dly_intr_o          : STD_LOGIC;
  signal nLed_Fifo_ne        : STD_LOGIC;
  signal ev_fifo_ne_intr_o   : STD_LOGIC;
  signal Interlock_Intr_i    : STD_LOGIC := '0';
  signal Data_Rdy_Intr_i     : STD_LOGIC := '0';
  signal Data_Req_Intr_i     : STD_LOGIC := '0';
  signal Interlock_Intr_o    : STD_LOGIC;
  signal Data_Rdy_Intr_o     : STD_LOGIC;
  signal Data_Req_Intr_o     : STD_LOGIC;
  signal nLed_Interl         : STD_LOGIC;
  signal nLed_Dry            : STD_LOGIC;
  signal nLed_Drq            : STD_LOGIC;
  signal every_ms_intr_o     : STD_LOGIC;
  signal lemo_data_o         : STD_LOGIC_VECTOR(4 DOWNTO 1);
  signal lemo_nled_o         : STD_LOGIC_VECTOR(4 DOWNTO 1);
  signal lemo_out_en_o       : STD_LOGIC_VECTOR(4 DOWNTO 1);
  signal lemo_data_i         : STD_LOGIC_VECTOR(4 DOWNTO 1) := (OTHERS => '0');
  signal nsig_wb_err         : STD_LOGIC;
  signal n_tx_req_led        : STD_LOGIC;
  signal n_rx_avail_led      : STD_LOGIC;

  -- Other constants
  constant C_CLK_PERIOD : time := 10.0ns;

  -- procedure for wishbone read transaction
  procedure tbWbRead (
      signal clk_i   : in  STD_LOGIC;
      signal slave_o : out t_wishbone_slave_in;
      signal slave_i : in  t_wishbone_slave_out;
      constant addr  : in  STD_LOGIC_VECTOR(31 downto 0);
      signal data    : out STD_LOGIC_VECTOR(31 downto 0);
      constant tpd   : in  time := 1 ns
    ) is
  begin
    slave_o.adr(addr'range) <= addr after tpd;
    slave_o.sel             <= "1111" after tpd;
    slave_o.we              <= '0' after tpd;
    slave_o.cyc             <= '1' after tpd;
    slave_o.stb             <= '1' after tpd;

    while (slave_i.ack = '0' and slave_i.err = '0') loop
      wait until clk_i = '1';
    end loop;

    -- read the data
    data <= slave_i.dat;

    -- stop waiting for data
    slave_o.cyc <= '0' after tpd;
    slave_o.stb <= '0' after tpd;
  end procedure;

  -- procedure for wishbone write transaction
  procedure tbWbWrite (
      signal clk_i   : in  STD_LOGIC;
      signal slave_o : out t_wishbone_slave_in;
      signal slave_i : in  t_wishbone_slave_out;
      constant addr  : in  STD_LOGIC_VECTOR(31 downto 0);
      constant data  : in  STD_LOGIC_VECTOR(31 downto 0);
      constant tpd   : in  time := 1 ns
    ) is
  begin
    slave_o.adr(addr'range) <= addr after tpd;
    slave_o.dat(data'range) <= data after tpd;
    slave_o.sel             <= "1111" after tpd;
    slave_o.we              <= '1' after tpd;
    slave_o.cyc             <= '1' after tpd;
    slave_o.stb             <= '1' after tpd;

    while (slave_i.ack = '0' and slave_i.err = '0') loop
      wait until clk_i = '1';
    end loop;

    -- stop waiting for data
    slave_o.cyc <= '0' after tpd;
    slave_o.stb <= '0' after tpd;
  end procedure;

  signal dataR : std_logic_vector(31 downto 0);

----------------------------------------------------------------------
begin

  -----------------------------------------------------------
  -- Clocks and Reset, Testbench Stimulus
  -----------------------------------------------------------
  CLK_GEN : process
  begin
    clk_i <= '1';
    wait for C_CLK_PERIOD / 2.0;
    clk_i <= '0';
    wait for C_CLK_PERIOD / 2.0;
  end process CLK_GEN;

  RESET_GEN : process
    variable addr : std_logic_vector(31 downto 0) := (others => '0');
    variable data : std_logic_vector(31 downto 0) := (others => '0');
  begin
    slave_i.adr <= (others => '0');
    slave_i.sel <= (others => '0');
    slave_i.we  <= '0';
    slave_i.dat <= (others => '0');
    slave_i.cyc <= '0';
    slave_i.stb <= '0';

    nRst_i <= '0','1' after 20.0*C_CLK_PERIOD;
    wait for 1us + 1ns;
    --------------------------------------------------------------------------
    -- Start simulation
    --------------------------------------------------------------------------

    --------------------------------------------------------------------------
    ---- set machester_fpga flag (will handle TX handshaking)
    addr(15+2 downto 0) := x"0402"&"00";
    data(15 downto 0)   := x"9000";
    tbWbWrite(clk_i,slave_i, slave_o, addr, data);
    wait for 1us;

    -- wait for 20us for internal logic to assert hw6408_rdy signal
    wait for 20us;

    --------------------------------------------------------------------------
    -- Write to TX ram (timeslot 0) to find out timings of poping and data
    addr(15+2 downto 0) := x"0400"&"00";
    data(15 downto 0)   := x"0001";
    tbWbWrite(clk_i,slave_i, slave_o, addr, data);
    wait for 1us;

    addr(15+2 downto 0) := x"0401"&"00";
    data(15 downto 0)   := x"0002";
    tbWbWrite(clk_i,slave_i, slave_o, addr, data);
    wait for 1us;

    -- Wait for a bit to separate timeslot 0 and normal Taskram 1-254
    wait for 20us;

    addr(15+2 downto 0) := x"0CF0"&"00"; -- CMD2: address location is 255 of TX taskram
    data(15 downto 0)   := x"0003";
    tbWbWrite(clk_i,slave_i, slave_o, addr, data);
    wait for 1us;

    -- Wait for a bit to separate timeslot 0 and 255
    wait for 50us;

    --------------------------------------------------------------------------
    addr(15+2 downto 0) := x"0413"&"00"; -- Block RX packet size: starts with 0
    data(15 downto 0)   := x"0009";
    tbWbWrite(clk_i,slave_i, slave_o, addr, data);
    wait for 1us;

    addr(15+2 downto 0) := x"0CFF"&"00"; -- CMD2: address location is 255 of TX taskram
    data(15 downto 0)   := x"0005";
    tbWbWrite(clk_i,slave_i, slave_o, addr, data);
    wait for 1us;

    -- Wait for a bit to timeout enough times for finishig task run for receiver on block mdoe
    wait for 500us;

    --------------------------------------------------------------------------
    -- read out the RX FIFO from timeslot 255
    for i in 0 to 9 loop
      addr(15+2 downto 0) := x"0DFF"&"00"; -- Block mode RX FIFO: reading from address location 255 of RX Taskram
      tbWbRead(clk_i,slave_i, slave_o, addr, dataR);
      wait for 1us;
    end loop;

    --------------------------------------------------------------------------
    -- End simulation
    --------------------------------------------------------------------------
    wait for 10us;
    assert (false) report "Simulation finished" severity FAILURE;
    wait;
  end process RESET_GEN;

  --------------------------------------------------------------------------
  -- Entity Under Test
  --------------------------------------------------------------------------
  DUT : entity work.wb_mil_scu
    generic map (
      Clk_in_Hz       => Clk_in_Hz,
      slave_i_adr_max => slave_i_adr_max
    )
    port map (
      clk_i               => clk_i,
      nRst_i              => nRst_i,
      slave_i             => slave_i,
      slave_o             => slave_o,
      nME_BOO             => nME_BOO,
      nME_BZO             => nME_BZO,
      ME_SD               => ME_SD,
      ME_ESC              => ME_ESC,
      ME_SDI              => ME_SDI,
      ME_EE               => ME_EE,
      ME_SS               => ME_SS,
      ME_BOI              => ME_BOI,
      ME_BZI              => ME_BZI,
      ME_UDI              => ME_UDI,
      ME_CDS              => ME_CDS,
      ME_SDO              => ME_SDO,
      ME_DSC              => clk_i, --ME_DSC,
      ME_VW               => ME_VW,
      ME_TD               => ME_TD,
      Mil_BOI             => Mil_BOI,
      Mil_BZI             => Mil_BZI,
      Sel_Mil_Drv         => Sel_Mil_Drv,
      nSel_Mil_Rcv        => nSel_Mil_Rcv,
      Mil_nBOO            => Mil_nBOO,
      Mil_nBZO            => Mil_nBZO,
      nLed_Mil_Rcv        => nLed_Mil_Rcv,
      nLed_Mil_Trm        => nLed_Mil_Trm,
      nLed_Mil_Err        => nLed_Mil_Err,
      error_limit_reached => error_limit_reached,
      Mil_Decoder_Diag_p  => Mil_Decoder_Diag_p,
      Mil_Decoder_Diag_n  => Mil_Decoder_Diag_n,
      timing              => timing,
      nLed_Timing         => nLed_Timing,
      dly_intr_o          => dly_intr_o,
      nLed_Fifo_ne        => nLed_Fifo_ne,
      ev_fifo_ne_intr_o   => ev_fifo_ne_intr_o,
      Interlock_Intr_i    => Interlock_Intr_i,
      Data_Rdy_Intr_i     => Data_Rdy_Intr_i,
      Data_Req_Intr_i     => Data_Req_Intr_i,
      Interlock_Intr_o    => Interlock_Intr_o,
      Data_Rdy_Intr_o     => Data_Rdy_Intr_o,
      Data_Req_Intr_o     => Data_Req_Intr_o,
      nLed_Interl         => nLed_Interl,
      nLed_Dry            => nLed_Dry,
      nLed_Drq            => nLed_Drq,
      every_ms_intr_o     => every_ms_intr_o,
      lemo_data_o         => lemo_data_o,
      lemo_nled_o         => lemo_nled_o,
      lemo_out_en_o       => lemo_out_en_o,
      lemo_data_i         => lemo_data_i,
      nsig_wb_err         => nsig_wb_err,
      n_tx_req_led        => n_tx_req_led,
      n_rx_avail_led      => n_rx_avail_led
    );

end architecture testbench;
----------------------------------------------------------------------