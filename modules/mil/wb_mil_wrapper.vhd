LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.std_logic_arith.all;
USE work.wb_mil_scu_pkg.all;
USE work.wishbone_pkg.all;

entity wb_mil_wrapper IS  
  generic (
    Clk_in_Hz:    INTEGER := 125_000_000;    -- Manchester IP needs 20 Mhz clock for proper detection of short 500ns data pulses
                                            -- Generic "Mil_clk_in_Hz"  "Baudrate" des Manchester-Ein-/Ausgangsdatenstroms umgepolt.
    Base_Addr:    INTEGER := 16#400#
  );
port  (
    -- SCU Bus Slave I/F
    Adr_from_SCUB_LA:     in      std_logic_vector(15 downto 0);
    Data_from_SCUB_LA:    in      std_logic_vector(15 downto 0);
    Ext_Adr_Val:          in      std_logic;                  
    Ext_Rd_active:        in      std_logic;                  
    Ext_Rd_fin:           in      std_logic;                      -- marks end of read cycle, active one for one clock period of sys_clk
    Ext_Wr_active:        in      std_logic;                    
    Ext_Wr_fin:           in      std_logic;                      -- marks end of write cycle, active one for one clock period of sys_clk
    clk:                  in      std_logic;                      -- same clk as used by SCU_Bus_Slave
    Data_to_SCUB:         out     std_logic_vector(15 downto 0);
    Data_for_SCUB:        out     std_logic;
    Dtack_to_SCUB:        out     std_logic;
    -- MIL I/F
    nME_BZO:              in      std_logic;
    nME_BOO:              in      std_logic;
    Reset_Puls:           in      std_logic;
    ME_SD:                in      std_logic;
    ME_ESC:               in      std_logic;
    ME_CDS:               in      std_logic;
    ME_SDO:               in      std_logic;
    ME_DSC:               in      std_logic;
    ME_VW:                in      std_logic;
    ME_TD:                in      std_logic;
    ME_SDI:               out     std_logic;
    ME_SS:                out     std_logic;
    ME_EE:                out     std_logic;
    Mil_In_Pos:           in      std_logic;                      --A_Mil1_BOI
    Mil_In_Neg:           in      std_logic;                      --A_Mil1_BZI
    ME_BOI:               out     std_logic;  
    ME_BZI:               out     std_logic;  
    Mil_Trm_Rdy:          buffer  std_logic;                      --For Led and TestPort
    nSel_Mil_Drv:         out     std_logic;                      --A_MIL1_OUT_En = not nSEl_Mil_Drv
    nSel_Mil_Rcv:         out     std_logic;                      --A_Mil1_nIN_En
    nMil_Out_Pos:         out     std_logic;                      --A_Mil1_nBZO
    nMil_Out_Neg:         out     std_logic;                      --A_Mil1_nBOO
    Mil_Rcv_Rdy:          buffer  std_logic;                      --For Led and TestPort
    
    nLed_Mil_Trm:         out     std_logic;
    nLED_Mil_Rcv_Error:   buffer  std_logic;                      --For Led and TestPort
    error_limit_reached:  out     std_logic;                      --not used
    No_VW_Cnt:            buffer  std_logic_vector(15 downto 0);  --EPLD-Manchester Bit[15..8] Fehlerzähler No Valid Word des positiven Decoders "No_VW_p", 
                                                                  --                Bit[7..0]  Fehlerzähler No Valid Word des negativen Decoders "No_VM_n"
    Not_Equal_Cnt:        buffer  std_logic_vector(15 downto 0);  --EPLD-Manchester Bit[15..8] Fehlerzähler Data_not_equal, 
                                                                  --                Bit[7..0]  Fehlerzähler unterschiedliche CMD-Data-Kennung (CMD_not_equal).
    Mil_Decoder_Diag_p:   out     std_logic_vector(15 downto 0);  --EPLD-Manchester-Decoders Diagnose: des Positiven Signalpfades
    Mil_Decoder_Diag_n:   out     std_logic_vector(15 downto 0);  --EPLD-Manchester-Decoders Diagnose: des Negativen Signalpfades
    -- LEDs and Interrupts
    timing:               in      std_logic;
    nLed_Timing:          out     std_logic;
    dly_intr_o:           out     std_logic;
    nLed_Fifo_ne:         out     std_logic;
    ev_fifo_ne_intr_o:    out     std_logic;
    Interlock_Intr_i:     in      std_logic;
    Data_Rdy_Intr_i:      in      std_logic;
    Data_Req_Intr_i:      in      std_logic;
    Interlock_Intr_o:     out     std_logic;
    Data_Rdy_Intr_o:      out     std_logic;
    Data_Req_Intr_o:      out     std_logic;
    nLed_Interl:          out     std_logic;
    nLed_Dry:             out     std_logic;
    nLed_Drq:             out     std_logic;
    every_ms_intr_o:      out     std_logic;
    -- lemo I/F
    lemo_data_o:          out     std_logic_vector(4 downto 1);
    lemo_nled_o:          out     std_logic_vector(4 downto 1);
    lemo_out_en_o:        out     std_logic_vector(4 downto 1);
    lemo_data_i:          in      std_logic_vector(4 downto 1)
    );
end wb_mil_wrapper;


ARCHITECTURE arch_wb_mil_wrapper OF wb_mil_wrapper IS


signal slave_i:                  t_wishbone_slave_in;
signal slave_o:                  t_wishbone_slave_out;

signal nReset:                   std_logic;
signal Sel_Mil_Drv:              std_logic;

signal rd_latch:                 std_logic_vector(15 downto 0);
signal rd_latch_ev_timer:        std_logic_vector(15 downto 0);
signal rd_latch_wait_timer:      std_logic_vector(15 downto 0);
signal rd_latch_dly_timer:       std_logic_vector(15 downto 0);

signal wr_latch_dly_timer_lw:    std_logic_vector(15 downto 0);
signal wr_latch_dly_timer_hw:    std_logic_vector(15 downto 0);

signal ack_stretched:            std_logic;
signal dly_buf_ack:              std_logic;

signal cycle_finished:           std_logic;
signal access_LB:                std_logic;
----------------------------------------------------------------------------------------------
begin

-- Event, Delay, Wait-timer need extra handling due to 32 bit access
-- SCU  with 32 bit access uses one hit, SIO with 16bit needs 2 hits (HW,LW)
-- For Read:
-- For SIO read the HW first! This causes latching the LW to keep timer value integrity.
-- For Write:
-- Write to Event and Wait timers, clears this timers no matter if written on HW or LW.
-- Delay Timer needs preloading LW and HW, then write to rd_wr_dly_timer_a  for loading.
-- Start of Delay Timer needs two accesses to Delay Register. 
--      First set DelayTimer  with MSB=0 (this is bit 8 of HW15:0). 
--      Then start Delay timer by doing a second write with same data on same address.

nReset       <= not Reset_Puls;
nSel_Mil_Drv <= not Sel_Mil_Drv;

-- EPLD_Manchester_Enc <= Sel_EPLD_n6408;   --only used for ModelSim

-- Process for writing to wb_mil_scu directly or via latches

process (Adr_from_SCUB_LA, wr_latch_dly_timer_hw,wr_latch_dly_timer_lw,Ext_wr_active,Data_from_SCUB_LA) 
begin
  case conv_integer(unsigned(Adr_from_SCUB_LA)) is
   -- these other regs have no address equivalent in wb_mil_scu 
   -- when rd_clr_ev_timer_LW_a  | rd_clr_ev_timer_HW_a | rd_clr_wait_timer_LW_a | rd_clr_wait_timer_HW_a =>
   --    slave_i.adr   <= x"000" & b"00" & Adr_from_SCUB_LA & b"00";
   --    slave_i.dat   <= (others =>'0'); 
   --    slave_i.we    <= Ext_wr_active;      
   -- Delay timer regs need preloaded latches
    when rd_wr_dly_timer_LW_a   =>
      slave_i.adr   <= x"000" & b"00" & Adr_from_SCUB_LA & b"00";
      slave_i.dat   <= x"0000" & Data_from_SCUB_LA; --dummy write, Data_from_SCUB_LA feeds wr_latch_dly_timer_lw
      slave_i.we    <= '0';
    when rd_wr_dly_timer_HW_a  =>
      slave_i.adr   <= x"000" & b"00" & Adr_from_SCUB_LA & b"00";
      slave_i.dat   <= Data_from_SCUB_LA & x"0000"; --dummy write, Data_from_SCUB_LA feeds wr_latch_dly_timer_hw
      slave_i.we    <= '0';
    when rd_wr_dly_timer_a  =>
      slave_i.adr   <= x"000" & b"00" & Adr_from_SCUB_LA & b"00";
      slave_i.dat   <= wr_latch_dly_timer_hw & wr_latch_dly_timer_lw;
      slave_i.we    <= Ext_wr_active;
    -- this regs and filter ram is loaded directly, using bits 15:0
    when others =>
      slave_i.adr   <= x"000" & b"00" & Adr_from_SCUB_LA & b"00";
      slave_i.dat   <= x"0000" & Data_from_SCUB_LA; 
      slave_i.we    <= Ext_wr_active;
    end case;
end process;





access_LB <= Ext_Adr_Val and (Ext_Wr_Active or Ext_Rd_active);
slave_i.sel <= x"F";
slave_i.stb <= access_LB and not cycle_finished;
slave_i.cyc <= access_LB and not cycle_finished;



process (clk) 
begin
  if (clk'event and clk='1') then
    if Reset_Puls or not access_LB  then
        cycle_finished <= '0';
    else
      if  access_LB  and slave_o.ack then
        cycle_finished <= '1'; -- first ack causes finish
      end if;
    end if;
  end if;  
end process;




Dtack_to_SCUB <= ack_stretched or slave_o.ack;
Data_for_SCUB <= ack_stretched or slave_o.ack;
--Data_to_SCUB  <= rd_latch;

--this mux handles read sources to have data for 1 additional clock present

process (Adr_from_SCUB_LA,ack_stretched,slave_o, rd_latch_ev_timer, rd_latch_wait_timer,rd_latch_dly_timer,rd_latch) 
begin

     if slave_o.ack  then  --for first few clocks
            -- get data direct from sources for quick response
            if    (conv_integer(unsigned(Adr_from_SCUB_LA)) = rd_clr_ev_timer_a)    then
              Data_to_SCUB            <=  slave_o.dat(31 downto 16);
            elsif(conv_integer(unsigned(Adr_from_SCUB_LA))  = rd_ev_timer_LW_a )    then
              Data_to_SCUB            <=  rd_latch_ev_timer;
            elsif(conv_integer(unsigned(Adr_from_SCUB_LA))  = rd_clr_wait_timer_a ) then
              Data_to_SCUB            <=  slave_o.dat(31 downto 16);
            elsif(conv_integer(unsigned(Adr_from_SCUB_LA))  = rd_wait_timer_LW_a )  then
               Data_to_SCUB            <=  rd_latch_wait_timer;
            elsif(conv_integer(unsigned(Adr_from_SCUB_LA))  = rd_wr_dly_timer_a )   then
              Data_to_SCUB            <=  slave_o.dat(31 downto 16);
            elsif(conv_integer(unsigned(Adr_from_SCUB_LA))  = rd_wr_dly_timer_LW_a )then
              Data_to_SCUB            <=  rd_latch_dly_timer;
            else
             Data_to_SCUB            <=  slave_o.dat(15 downto 0);
            end if;
      elsif ack_stretched then     
            -- get data from latch for rest of SCU Bus access cycle
            Data_to_SCUB            <=   rd_latch;
      else
            Data_to_SCUB            <=    x"0000";
      end if; 
end process;


process (clk) 
begin
  if (clk'event and clk='1') then
    if Reset_Puls ='1' then
      ack_stretched         <= '0';     
      rd_latch              <= (others=>'0');
      rd_latch_ev_timer     <= (others=>'0');
      rd_latch_wait_timer   <= (others=>'0');
      rd_latch_dly_timer    <= (others=>'0');
    else  
      if Ext_Rd_active = '1' then 
        if slave_o.ack= '1' or dly_buf_ack='1'  then
              --stores timer value in one hit to keep timer value consistency
              --rd_latch is  transport to SCU_Bus within access
              --LW latch has to be read in an second SCU Bus access
              --therefore a LW latch for every timer to avoid data mixup       
           if    (conv_integer(unsigned(Adr_from_SCUB_LA)) = rd_clr_ev_timer_a    ) then  -- rd captures both
              rd_latch            <=  slave_o.dat(31 downto 16);
              rd_latch_ev_timer   <=  slave_o.dat(15 downto  0); 
            elsif(conv_integer(unsigned(Adr_from_SCUB_LA)) = rd_ev_timer_LW_a ) then
              rd_latch            <=  rd_latch_ev_timer;
            elsif(conv_integer(unsigned(Adr_from_SCUB_LA)) = rd_clr_wait_timer_a  ) then  -- rd captures both
              rd_latch            <=  slave_o.dat(31 downto 16);
              rd_latch_wait_timer <=  slave_o.dat(15 downto  0); 
            elsif(conv_integer(unsigned(Adr_from_SCUB_LA)) = rd_wait_timer_LW_a )then
              rd_latch            <=  rd_latch_wait_timer;
            elsif(conv_integer(unsigned(Adr_from_SCUB_LA)) = rd_wr_dly_timer_a )     then -- rd captures both
              rd_latch            <=  slave_o.dat(31 downto 16);
              rd_latch_dly_timer  <=  slave_o.dat(15 downto  0);
            elsif(conv_integer(unsigned(Adr_from_SCUB_LA)) = rd_wr_dly_timer_LW_a )   then
              rd_latch            <=  rd_latch_dly_timer;
            else
               -- all other data is 16 bit only 
              rd_latch            <=  slave_o.dat(15 downto  0);
           end if;           
        end if;
        ack_stretched <= '1';
      else
        rd_latch      <=  ( others => '0');
        ack_stretched <='0';      
      end if; 
    end if;
  end if;  
end process;



-- Latch for Delay Timer, which needs to be feed 32 bit in one hit
process (clk) 
begin
  if (clk'event and clk='1') then
    if Reset_Puls ='1' then
      wr_latch_dly_timer_hw <= ( others => '0'); 
      wr_latch_dly_timer_lw <= ( others => '0'); 
      dly_buf_ack <='0';
    else
      if  Ext_Wr_Active ='1' then
        case conv_integer(unsigned(Adr_from_SCUB_LA)) is 
         when rd_wr_dly_timer_LW_a =>
              wr_latch_dly_timer_lw <= Data_from_SCUB_LA;
              dly_buf_ack           <='1';
         when rd_wr_dly_timer_HW_a =>
              wr_latch_dly_timer_hw <= Data_from_SCUB_LA;
              dly_buf_ack           <='1';              
         when others  =>
              dly_buf_ack <='0';
        end case;
      elsif Ext_Rd_Active ='1' then
        case conv_integer(unsigned(Adr_from_SCUB_LA)) is 
          when rd_ev_timer_LW_a |rd_wait_timer_LW_a | rd_wr_dly_timer_LW_a|rd_wr_dly_timer_HW_a =>
            dly_buf_ack           <='1';
          when others  =>
            dly_buf_ack <='0';
        end case;
      else
          dly_buf_ack <='0';
      end if;
    end if;
  end if;  
end process;


mil : wb_mil_scu_v2
  generic map(
    Clk_in_Hz           => clk_in_hz)
  port map(
    clk_i               => clk,
    nRst_i              => nReset,
    slave_i             => slave_i,             -- cyc, stb, adr31..0, sel3..0, we, dat 31..0 
    slave_o             => slave_o,             -- ack,err,stall,dat 31..0 (int not used, rty=static 0) 
  -- encoder (transmitter) signals of HD6408 --------------------------------------------------------------------------------
    nME_BOO             => nme_boo,
    nME_BZO             => nme_bzo,
    ME_SD               => me_sd,
    ME_ESC              => me_esc,
    ME_SDI              => me_sdi,
    ME_EE               => me_ee,
    ME_SS               => me_ss,
  -- decoder (receiver) signals of HD6408 ---------------------------------------------------------------------------------
    ME_BOI              => ME_BOI,
    ME_BZI              => ME_BZI,
    ME_UDI              => open,                --No use of Harris UDI, set to 0 on sio.vhd
    ME_CDS              => me_cds,
    ME_SDO              => me_sdo,
    ME_DSC              => me_dsc,
    ME_VW               => me_vw,
    ME_TD               => me_td,
  -- decoder/encoder signals of HD6408 ------------------------------------------------------------------------------------
    Mil_BOI             => Mil_In_Pos,
    Mil_BZI             => Mil_In_Neg,
    Sel_Mil_Drv         => Sel_Mil_Drv,
    nSel_Mil_Rcv        => nsel_Mil_Rcv,
    Mil_nBOO            => nMil_Out_Neg,
    Mil_nBZO            => nMil_Out_Pos,
  -- others ------------------------------------------------------------------------------------
    nLed_Mil_Rcv        => open,
    nLed_Mil_Trm        => nLed_Mil_Trm,
    nLed_Mil_Err        => nLED_Mil_Rcv_Error,
    error_limit_reached => error_limit_reached,
    Mil_Decoder_Diag_p  => Mil_Decoder_Diag_p,
    Mil_Decoder_Diag_n  => Mil_Decoder_Diag_n,
    timing              => timing,              --A_Timing
    dly_intr_o          => dly_intr_o,          --Interrupt from Delay Timer
    nLed_Timing         => nLed_Timing,         --A_nLED_Timing
    nLed_Fifo_ne        => nLed_Fifo_ne,        --A_nLED7 Functionality
  -- interrupts (in=to be debounced, out=debounced interrupts)
    ev_fifo_ne_intr_o   => ev_fifo_ne_intr_o, 
    Interlock_Intr_i    => Interlock_Intr_i,    -- not A_nOPK_INL
    Data_Rdy_Intr_i     => Data_Rdy_Intr_i,     -- not A_nOPK_DRDY
    Data_Req_Intr_i     => Data_Req_Intr_i,     -- not A_nOPK_DRQ
    Interlock_Intr_o    => Interlock_Intr_o, 
    Data_Rdy_Intr_o     => Data_Rdy_Intr_o,
    Data_Req_Intr_o     => Data_Req_Intr_o,
  -- leds
    nLed_Interl         => nLed_Interl,
    nLed_drq            => nLed_Drq,
    nLed_dry            => nLed_dry,
    every_ms_intr_o     => every_ms_intr_o,
  -- lemo I/F
    lemo_data_o         => lemo_data_o,
    lemo_nled_o         => lemo_nled_o,
    lemo_out_en_o       => lemo_out_en_o,
    lemo_data_i         => lemo_data_i,
    nsig_wb_err         => open
  );

end arch_wb_mil_wrapper;
