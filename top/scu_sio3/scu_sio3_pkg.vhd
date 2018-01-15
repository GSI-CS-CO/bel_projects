library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use work.wishbone_pkg.all;
use work.genram_pkg.all;

library work;

package scu_sio3_pkg is

component wb_mil_wrapper_sio is 
generic (
    Clk_in_Hz:                 INTEGER := 125_000_000;    -- Manchester IP needs 20 Mhz clock for proper detection of short 500ns data pulses
    ram_count:                 INTEGER := 254
    );
port  (
    Adr_from_SCUB_LA:     in       std_logic_vector(15 downto 0);
    Data_from_SCUB_LA:    in       std_logic_vector(15 downto 0);
    Ext_Adr_Val:          in       std_logic;                  
    Ext_Rd_active:        in       std_logic;                  
    Ext_Rd_fin:           in       std_logic;                      -- marks end of read cycle, active one for one clock period of sys_clk
    Ext_Wr_active:        in       std_logic;                    
    Ext_Wr_fin:           in       std_logic;                      -- marks end of write cycle, active one for one clock period of sys_clk
    clk:                  in       std_logic;                      -- should be the same clk, used by SCU_Bus_Slave
    Data_to_SCUB:         out      std_logic_vector(15 downto 0);
    Data_for_SCUB:        out      std_logic;
    Dtack_to_SCUB:        out      std_logic;

    nME_BZO:              in       std_logic;
    nME_BOO:              in       std_logic;
    Reset_Puls:           in       std_logic;
    ME_SD:                in       std_logic;
    ME_ESC:               in       std_logic;
    ME_CDS:               in       std_logic;
    ME_SDO:               in       std_logic;
    ME_DSC:               in       std_logic;
    ME_VW:                in       std_logic;
    ME_TD:                in       std_logic;
    ME_SDI:               out      std_logic;
    ME_SS:                out      std_logic;
    ME_EE:                out      std_logic;
    Mil_In_Pos:           in       std_logic;                      --A_Mil1_BOI
    Mil_In_Neg:           in       std_logic;                      --A_Mil1_BZI
    ME_BOI:               out      std_logic;  
    ME_BZI:               out      std_logic;  
    nSel_Mil_Drv:         out      std_logic;                      --A_MIL1_OUT_En = not nSEl_Mil_Drv
    nSel_Mil_Rcv:         out      std_logic;                      --A_Mil1_nIN_En
    nMil_Out_Pos:         out      std_logic;                      --A_Mil1_nBZO
    nMil_Out_Neg:         out      std_logic;                      --A_Mil1_nBOO
    nLed_Mil_Rcv:         out      std_logic;                      --For Led and TestPort
    
    nLed_Mil_Trm:         out      std_logic;
    nLED_Mil_Rcv_Error:   out      std_logic;                      --For Led and TestPort
    error_limit_reached:  out      std_logic;                      --not used
    Mil_Decoder_Diag_p:   out      std_logic_vector(15 downto 0);  --EPLD-Manchester-Decoders Diagnose: des Positiven Signalpfades
    Mil_Decoder_Diag_n:   out      std_logic_vector(15 downto 0);  --EPLD-Manchester-Decoders Diagnose: des Negativen Signalpfades

    timing:               in       std_logic;
    nLed_Timing:          out      std_logic;
    dly_intr_o:           out      std_logic;
    nLed_Fifo_ne:         out      std_logic;
    ev_fifo_ne_intr_o:    out      std_logic;
    Interlock_Intr_i:     in       std_logic;
    Data_Rdy_Intr_i:      in       std_logic;
    Data_Req_Intr_i:      in       std_logic;
    Interlock_Intr_o:     out      std_logic;
    Data_Rdy_Intr_o:      out      std_logic;
    Data_Req_Intr_o:      out      std_logic;
    nLed_Interl:          out      std_logic;
    nLed_Dry:             out      std_logic;
    nLed_Drq:             out      std_logic;
    every_ms_intr_o:      out      std_logic;
    n_tx_req_led:         out      std_logic;
    n_rx_avail_led:       out      std_logic;
          -- lemo I/F
    lemo_data_o:          out      std_logic_vector(4 downto 1);
    lemo_nled_o:          out      std_logic_vector(4 downto 1);
    lemo_out_en_o:        out      std_logic_vector(4 downto 1);
    lemo_data_i:          in       std_logic_vector(4 downto 1)
    );
end component;


component flash_loader_v01
  PORT
  (
    noe_in:    IN       STD_LOGIC 
  );
END component flash_loader_v01;


component pll_sio
  port(
    inclk0:    in       std_logic;
    c0:        out      std_logic;
    c1:        out      std_logic;
    locked:    out      std_logic
  );
end component;

component mil_pll
  PORT(
    inclk0:    IN       std_logic  := '0';
    c0:        OUT      std_logic ;
    locked:    OUT      std_logic 
  );
end component;


component SysClock
  port(
    inclk0:    in       std_logic := '0';
    c0:        out      std_logic;
    c1:        out      std_logic;
    locked:    out      std_logic 
  );
end component;


component pu_reset
  generic(
    PU_Reset_in_clks :  INTEGER
  );
  port  (
    Clk:      in        std_logic;
    PU_Res:    out      std_logic
  );
end component;


end package scu_sio3_pkg;
