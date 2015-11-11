LIBRARY ieee;
USE ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.wishbone_pkg.all;
use work.aux_functions_pkg.all;
use work.mil_pkg.all;
use work.wb_mil_scu_pkg.all;
use work.scu_sio3_pkg.all;


entity wb_mil_wrapper_sio IS  
  generic (
    Clk_in_Hz:              integer := 125_000_000;  -- Manchester IP needs 20 Mhz clock for proper detection of short 500ns data pulse
    sio_mil_first_reg_a:    unsigned(15 downto 0)  := x"0400";
    sio_mil_last_reg_a:     unsigned(15 downto 0)  := x"0411";
    evt_filt_first_a:       unsigned(15 downto 0)  := x"1000";
    evt_filt_last_a:        unsigned(15 downto 0)  := x"1FFF"



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
    nSel_Mil_Drv:         out     std_logic;                      --A_MIL1_OUT_En = not nSEl_Mil_Drv
    nSel_Mil_Rcv:         out     std_logic;                      --A_Mil1_nIN_En
    nMil_Out_Pos:         out     std_logic;                      --A_Mil1_nBZO
    nMil_Out_Neg:         out     std_logic;                      --A_Mil1_nBOO

    
    nLed_Mil_Trm:         out     std_logic;
    nLed_Mil_Rcv:         out     std_logic;                      --For Led and TestPort
    nLED_Mil_Rcv_Error:   out     std_logic;                      --For Led and TestPort
    error_limit_reached:  out     std_logic;                      --not used
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
end wb_mil_wrapper_sio;


ARCHITECTURE arch_wb_mil_wrapper_sio OF wb_mil_wrapper_sio IS


constant rd_clr_ev_timer_a_map:         unsigned (15 downto 0)    := sio_mil_first_reg_a + rd_clr_ev_timer_a;
constant rd_ev_timer_LW_a_map:          unsigned (15 downto 0)    := sio_mil_first_reg_a + rd_ev_timer_LW_a;
constant rd_clr_wait_timer_a_map:       unsigned (15 downto 0)    := sio_mil_first_reg_a + rd_clr_wait_timer_a;
constant rd_wait_timer_LW_a_map:        unsigned (15 downto 0)    := sio_mil_first_reg_a + rd_wait_timer_LW_a;
constant rd_wr_dly_timer_a_map:         unsigned (15 downto 0)    := sio_mil_first_reg_a + rd_wr_dly_timer_a;
constant rd_wr_dly_timer_LW_a_map:      unsigned (15 downto 0)    := sio_mil_first_reg_a + rd_wr_dly_timer_LW_a;
constant rd_wr_dly_timer_HW_a_map:      unsigned (15 downto 0)    := sio_mil_first_reg_a + rd_wr_dly_timer_HW_a;


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

signal mil_reg_access:           std_logic;
signal mil_ram_access:           std_logic;
signal slave_o_ack_la:           std_logic;
----------------------------------------------------------------------------------------------
begin

-- Event, Delay, Wait-timer need extra 16 bit LW read latches due to 32 bit data
-- For Read:
-- Timers need to read HW (=0x406|0x407|0x408) first. This latches LW to keep timer value integrity.
-- For Write:
-- Write to Event and Wait timers (=0x406|0x408) clears whole timer
-- Delay Timer needs preloading LW and HW, then write to rd_wr_dly_timer_a(=0x407)for loading.
-- Start of Delay Timer needs set of MSB=0 (this is bit 8 of High Word 15:0)

nReset       <= not Reset_Puls;
nSel_Mil_Drv <= not Sel_Mil_Drv;

-- EPLD_Manchester_Enc <= Sel_EPLD_n6408;   --only used for ModelSim

-- Process for writing to wb_mil_scu directly or via latches

process (Adr_from_SCUB_LA, wr_latch_dly_timer_hw,wr_latch_dly_timer_lw,Ext_wr_active,Data_from_SCUB_LA) 
begin
  case (unsigned(Adr_from_SCUB_LA (15 downto 0))) is
    when rd_wr_dly_timer_LW_a_map  =>
      slave_i.adr   <= x"000" & b"00" & Adr_from_SCUB_LA & b"00";
      slave_i.dat   <= x"0000" & Data_from_SCUB_LA; --dummy write, Data_from_SCUB_LA feeds wr_latch_dly_timer_lw
      slave_i.we    <= '0';
    when rd_wr_dly_timer_HW_a_map =>
      slave_i.adr   <= x"000" & b"00" & Adr_from_SCUB_LA & b"00";
      slave_i.dat   <= Data_from_SCUB_LA & x"0000"; --dummy write, Data_from_SCUB_LA feeds wr_latch_dly_timer_hw
      slave_i.we    <= '0';
    when rd_wr_dly_timer_a_map  =>
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


-- slave_o_ack_la active until end of Local Access

process (clk) 
begin
  if (clk'event and clk='1') then
    if  Reset_Puls='1' or access_LB='0'  then
        cycle_finished <= '0';
        slave_o_ack_la <= '0';
    else
      if access_LB = '0' then
        slave_o_ack_la <= '0';
      elsif slave_o.ack = '1' and slave_o_ack_la='0' then
        slave_o_ack_la <='1'; --set signal until cycle end
      end if;
      
      if  access_LB='1'   and slave_o.ack='1' then
        cycle_finished <= '1'; -- first ack causes finish
      end if;
    end if;
  end if;  
end process;


Dtack_to_SCUB <= ack_stretched or slave_o_ack_la or dly_buf_ack; 
Data_for_SCUB <= ack_stretched or slave_o_ack_la;


process (Adr_from_SCUB_LA)
begin
  if   (unsigned (Adr_from_SCUB_LA) >=  sio_mil_first_reg_a  and  unsigned (Adr_from_SCUB_LA) <=  sio_mil_last_reg_a ) then
     mil_reg_access  <= '1';
     mil_ram_access  <= '0';
  elsif(unsigned(Adr_from_SCUB_LA) >= (evt_filt_first_a)  and  unsigned (Adr_from_SCUB_LA) <=   (evt_filt_last_a)) then
     mil_reg_access <= '0';
      mil_ram_access <= '1';
  else
      mil_reg_access <= '0';
      mil_ram_access <= '0';
  end if;
end process;


process (Adr_from_SCUB_LA,ack_stretched,slave_o, rd_latch_ev_timer, rd_latch_wait_timer,rd_latch_dly_timer,rd_latch, mil_reg_access, mil_ram_access) 
begin

     if slave_o.ack='1'  and (mil_reg_access='1' or mil_ram_access='1' )then  --for first few clocks
            -- get data direct from sources for quick response
            if    (unsigned(Adr_from_SCUB_LA)) = rd_clr_ev_timer_a_map    then
              Data_to_SCUB            <=  slave_o.dat(31 downto 16);
            elsif (unsigned(Adr_from_SCUB_LA)) = rd_ev_timer_LW_a_map     then
              Data_to_SCUB            <=  rd_latch_ev_timer;
            elsif (unsigned(Adr_from_SCUB_LA)) = rd_clr_wait_timer_a_map  then
              Data_to_SCUB            <=  slave_o.dat(31 downto 16);
            elsif (unsigned(Adr_from_SCUB_LA)) = rd_wait_timer_LW_a_map   then
               Data_to_SCUB            <=  rd_latch_wait_timer;
            elsif (unsigned(Adr_from_SCUB_LA)) = rd_wr_dly_timer_a_map    then
              Data_to_SCUB            <=  slave_o.dat(31 downto 16);
            elsif (unsigned(Adr_from_SCUB_LA)) = rd_wr_dly_timer_LW_a_map then
              Data_to_SCUB            <=  rd_latch_dly_timer;
            else
             Data_to_SCUB            <=  slave_o.dat(15 downto 0);
            end if;
      elsif ack_stretched='1' then     
            -- get data from latch for rest of SCU Bus access cycle
            Data_to_SCUB            <=   rd_latch;
      else
            Data_to_SCUB            <=    x"0000";
      end if; 
end process;


process (clk) 
begin
  if (clk'event and clk='1') then
    if Reset_Puls='1' then
      ack_stretched         <= '0';     
      rd_latch              <= (others=>'0');
      rd_latch_ev_timer     <= (others=>'0');
      rd_latch_wait_timer   <= (others=>'0');
      rd_latch_dly_timer    <= (others=>'0');
    else  
      if Ext_Rd_active='1'  and (mil_reg_access='1' or mil_ram_access='1') then 
        if slave_o.ack='1'  or dly_buf_ack='1'   then  
              --read of timer data (hw directly, lw thru latch)    
           if     (unsigned(Adr_from_SCUB_LA)) = rd_clr_ev_timer_a_map   then
              rd_latch            <=  slave_o.dat(31 downto 16);
              rd_latch_ev_timer   <=  slave_o.dat(15 downto  0); 
            elsif (unsigned(Adr_from_SCUB_LA)) = rd_ev_timer_LW_a_map    then
              rd_latch            <=  rd_latch_ev_timer;
            elsif (unsigned(Adr_from_SCUB_LA)) = rd_clr_wait_timer_a_map then
              rd_latch            <=  slave_o.dat(31 downto 16);
              rd_latch_wait_timer <=  slave_o.dat(15 downto  0); 
            elsif (unsigned(Adr_from_SCUB_LA)) = rd_wait_timer_LW_a_map  then
              rd_latch            <=  rd_latch_wait_timer;
            elsif (unsigned(Adr_from_SCUB_LA)) = rd_wr_dly_timer_a_map   then
              rd_latch            <=  slave_o.dat(31 downto 16);
              rd_latch_dly_timer  <=  slave_o.dat(15 downto  0);
            elsif (unsigned(Adr_from_SCUB_LA)) = rd_wr_dly_timer_LW_a_map then
              rd_latch            <=  rd_latch_dly_timer;
            else
               -- read of other wb_mil_scu data including filter ram and fifo
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
    if Reset_Puls='1' then
      wr_latch_dly_timer_hw <= ( others => '0'); 
      wr_latch_dly_timer_lw <= ( others => '0'); 
      dly_buf_ack <='0';
    else
      if  Ext_Wr_Active='1' then
        case unsigned(Adr_from_SCUB_LA)is 
         when rd_wr_dly_timer_LW_a_map =>
              wr_latch_dly_timer_lw <= Data_from_SCUB_LA;
              dly_buf_ack           <='1';
         when rd_wr_dly_timer_HW_a_map =>
              wr_latch_dly_timer_hw <= Data_from_SCUB_LA;
              dly_buf_ack           <='1';              
         when others  =>
              dly_buf_ack <='0';
        end case;
      elsif Ext_Rd_Active ='1' then
        case unsigned(Adr_from_SCUB_LA) is 
          when rd_ev_timer_LW_a_map |rd_wait_timer_LW_a_map | rd_wr_dly_timer_LW_a_map|rd_wr_dly_timer_HW_a_map =>
              dly_buf_ack  <='1';
          when others  =>
              dly_buf_ack  <='0';
        end case;
      else
              dly_buf_ack  <='0';
      end if;
    end if;
  end if;  
end process;


mil : wb_mil_sio
  generic map(
    Clk_in_Hz           => clk_in_hz,
    sio_mil_first_reg_a => sio_mil_first_reg_a,
    sio_mil_last_reg_a  => sio_mil_last_reg_a,
    evt_filt_first_a    => evt_filt_first_a,
    evt_filt_last_a     => evt_filt_last_a
    )
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
    nLed_Mil_Rcv        => nLed_Mil_Rcv,
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

end arch_wb_mil_wrapper_sio;
