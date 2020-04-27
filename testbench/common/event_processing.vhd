LIBRARY ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.math_real.all;

use work.wb_mil_scu_pkg.all;
use work.mil_pkg.all;
use work.genram_pkg.all;
use work.aux_functions_pkg.all;


entity event_processing is 
  generic (
    clk_in_hz:  INTEGER := 125_000_000    -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen
                                          -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
    );
  port (
    ev_filt_12_8b:    in      std_logic;
    ev_filt_on:       in      std_logic;
    ev_reset_on:      in      std_logic;
    puls1_frame:      in      std_logic;
    puls2_frame:      in      std_logic;
    timing_i:         in      std_logic;
    clk_i:            in      std_logic;
    nRst_i:           in      std_logic;
    wr_filt_ram:      in      std_logic;
    rd_filt_ram:      in      std_logic;
    rd_ev_fifo:       in      std_logic;
    clr_ev_fifo:      in      std_logic;
    filt_addr:        in      std_logic_vector(filter_addr_width-1 downto 0);
    filt_data_i:      in      std_logic_vector(filter_data_width-1 downto 0);
    stall_o:          out     std_logic;
    read_port_o:      out     std_logic_vector(15 downto 0);
    ev_fifo_ne:       out     std_logic;
    ev_fifo_full:     out     std_logic;
    ev_timer_res:     out     std_logic;
    ev_puls1:         out     std_logic;
    ev_puls2:         out     std_logic;
    timing_received:  buffer  std_logic
  );
end event_processing;


architecture arch_event_processing of event_processing is


  type  t_filt_cntrl_sm  is (
     fi_idle,
     fi_wr,
     fi_rd,
     ev_rd,
     ev_rd_fin
     );

  signal  filt_cntrl_sm:  t_filt_cntrl_sm;


  signal    timing_cmd:         std_logic;
  signal    timing_rcv:         std_logic;
  signal    event_fin:          std_logic;
  signal    ev_addr_active:     std_logic;
  signal    filt_wr:            std_logic;
  signal    filter_addr:        std_logic_vector(filter_addr_width-1 downto 0);
  signal    event_d:            std_logic_vector(15 downto 0);
  signal    event_o:            std_logic_vector(15 downto 0);
  signal    ev_direct_to_fifo:  std_logic;
  signal    ev_to_fifo:         std_logic;
  signal    ev_fifo_empty:      std_logic;
  signal    filt_data_o:        std_logic_vector(filter_data_width-1 downto 0);
  alias     fi_ev_to_fifo:      std_logic is  filt_data_o(0);             -- if event address filter ram and the memory bit(0) is '1' 
                                                                          -- then this event should be written to the event_fifo.
  alias     fi_ev_timer_res:    std_logic is  filt_data_o(1);             -- if event address filter ram and the memory bit(1) is '1' 
                                                                          -- then this event should reset the event timer.
  alias     fi_ev_puls1_s:      std_logic is  filt_data_o(2);             -- if event address filter ram and the memory bit(2) is '1' 
                                                                          -- then this event starts ev_puls1 or frame_puls1.
  alias     fi_ev_puls1_e:      std_logic is  filt_data_o(3);             -- if event address filter ram and the memory bit(3) is '1' 
                                                                          -- and ev_frame1 is '1' then this event stop frame_puls1.
  alias     fi_ev_puls2_s:      std_logic is  filt_data_o(4);             -- if event address filter ram and the memory bit(4) is '1' 
                                                                          -- then this event starts ev_puls2 or frame_puls2.
  alias     fi_ev_puls2_e:      std_logic is  filt_data_o(5);             -- if event address filter ram and the memory bit(5) is '1' 
                                                                          -- and ev_frame2 is '1' then this event stop frame_puls2.
  signal    fi_stall:           std_logic;
  signal    dly_stall:          std_logic;
  signal    last_acc_event:     std_logic;
  signal    puls1, puls2:       std_logic;
  signal    puls1_o, puls2_o:   std_logic;
  
  signal    ev_timer_res_o:     std_logic;

  signal    n_clr_ev_fifo:      std_logic;

  signal    Rst:                std_logic;

  signal    n_puls1:            std_logic;
  signal    puls1_length_ena:   std_logic;

  signal    n_puls2:            std_logic;
  signal    puls2_length_ena:   std_logic;

begin

Rst <= not nRst_i;

event_fifo: generic_sync_fifo
  generic map (
    g_data_width => 16,
    g_size       => 256,
    g_show_ahead => true
    )
  port map (
    rst_n_i        => n_clr_ev_fifo,
    clk_i          => clk_i,
    d_i            => event_d,
    we_i           => ev_to_fifo,
    q_o            => event_o,
    rd_i           => rd_ev_fifo,
    empty_o        => ev_fifo_empty,
    full_o         => ev_fifo_full,
    almost_empty_o => open,
    almost_full_o  => open,
    count_o        => open);
    
ev_fifo_ne <= not ev_fifo_empty;
n_clr_ev_fifo <= not clr_ev_fifo;


filter_ram: generic_spram
  generic map(
    g_data_width        => filter_data_width,
    g_size              => filter_ram_size
    )
  port map (
    rst_n_i => '1',
    clk_i   => clk_i,
    we_i    => filt_wr,
    a_i     => filter_addr,
    d_i     => filt_data_i,
    q_o     => filt_data_o);


filt_wr <= '1' when filt_cntrl_sm = fi_wr else '0'; -- vielleicht besser getaktet


Serial_Timing:  mil_dec_edge_timed
  generic map (
    CLK_in_Hz         => clk_in_Hz,     -- Um die Flanken des Manchester-Datenstroms von 1Mb/s genau genug ausmessen zu koennen 
                                        -- (kuerzester Flankenabstand 500 ns), muss das Makro mit mindestens 20 Mhz getaktet werden.
                                        -- Die tatsaechlich angelegte Frequenz, muss vor der Synthese in "CLK_in_Hz"
                                        -- in Hertz beschrieben werden.
    Receive_pos_lane  => 0              -- '1' => der positive Signalstrom ist an Manchester_In angeschlossen
                                        -- '0' => der negative Signalstrom ist an Manchester_In angeschlossen.
    )
  port map (
    Manchester_In     => timing_i,      -- Eingangsdatenstrom MIL-1553B
    RD_MIL            => event_fin,     -- setzt Rvc_Cmd, Rcv_Rdy und Rcv_Error zurück. Muss synchron zur Clock 'clk' und 
                                        -- mindesten eine Periode lang aktiv sein!
    Res               => Rst,           -- Muss mindestens einmal für eine Periode von 'clk' aktiv ('1') gewesen sein.
    clk               => clk_i,
    Rcv_Cmd           => timing_cmd,    -- '1' es wurde ein Kommando empfangen.
    Rcv_Error         => open,          -- ist bei einem Fehler für einen Takt aktiv '1'.
    Rcv_Rdy           => timing_rcv,    -- '1' es wurde ein Kommand oder Datum empfangen.
                                        -- Wenn Rcv_Cmd = '0' => Datum. Wenn Rcv_Cmd = '1' => Kommando
    Mil_Rcv_Data      => event_d,       -- Empfangenes Datum oder Komando
    Mil_Decoder_Diag  => open           -- Diagnoseausgänge für Logikanalysator
    );

timing_received <= timing_rcv and timing_cmd;

ev_direct_to_fifo <= '1' when unsigned(event_d(7 downto 0)) >= 200 else '0';

ev_to_fifo <= event_fin and (fi_ev_to_fifo or not ev_filt_on or ev_direct_to_fifo);


p_filt_access:  process (clk_i, nRst_i)
  begin
    if nRst_i = '0' then
      filt_cntrl_sm <= fi_idle;
      last_acc_event <= '0';
      event_fin <= '0';
      fi_stall <= '1';

    elsif rising_edge(clk_i) then

      event_fin <= '0';
      fi_stall <= '1';

      case filt_cntrl_sm is

        when fi_idle =>
          if timing_received = '1' and wr_filt_ram = '0' and rd_filt_ram = '0' then
            filt_cntrl_sm <= ev_rd;
          elsif timing_received = '0' and wr_filt_ram = '1' then
            filt_cntrl_sm <= fi_wr;
          elsif timing_received = '0' and rd_filt_ram = '1' then
            filt_cntrl_sm <= fi_rd;
          elsif timing_received = '1' and wr_filt_ram = '1' then
            if last_acc_event = '1' then
              filt_cntrl_sm <= fi_wr;
            else
              filt_cntrl_sm <= ev_rd;
            end if;
          elsif timing_received = '1' and rd_filt_ram = '1' then
            if last_acc_event = '1' then
              filt_cntrl_sm <= fi_rd;
            else
              filt_cntrl_sm <= ev_rd;
            end if;
          end if;

        when ev_rd =>
          last_acc_event <= '1';
          event_fin <= '1';
          filt_cntrl_sm <= ev_rd_fin;

        when ev_rd_fin =>
          filt_cntrl_sm <= fi_idle;

        when fi_wr =>
          last_acc_event <= '0';
          fi_stall <= '0';
          if wr_filt_ram = '0' then     -- wait for end of write cycle
            filt_cntrl_sm <= fi_idle;
          end if;

        when fi_rd =>
          last_acc_event <= '0';
          fi_stall <= '0';
          if rd_filt_ram = '0' then     -- wait for end of read cycle
            filt_cntrl_sm <= fi_idle;
          end if;

        when others =>
          filt_cntrl_sm <= fi_idle;

      end case;
    end if;

  end process p_filt_access;


puls1_length_ena <= puls1 and not puls1_frame;
n_puls1 <= not puls1;
puls1_length: div_n
  generic map (
    n => clk_in_hz / (1_000_000_000 / 500)  -- 500 ns Pulsbreite
    )
  port map (
    res   => n_puls1,
    clk   => clk_i,
    ena   => puls1_length_ena,
    div_o => puls1_o
    );

p_puls1:  process (clk_i)
  begin
    if rising_edge(clk_i) then
      if (timing_received and ev_filt_on and fi_ev_puls1_s) = '1' then
        puls1 <= '1';
      elsif puls1 = '1' then
        if puls1_frame = '1' then
          if (timing_received and ev_filt_on and fi_ev_puls1_e) = '1' then
            puls1 <= '0';
          end if;
        else
          if puls1_o = '1' then
            puls1 <= '0';
          end if;
        end if;
      end if;
    end if;
  end process p_puls1;
  
ev_puls1 <= puls1;

puls2_length_ena <= puls2 and not puls2_frame;
n_puls2 <= not puls2;
puls2_length: div_n
  generic map (
    n => clk_in_hz / (1_000_000_000 / 500)  -- 500 ns Pulsbreite
    )
  port map (
    res   => n_puls2,
    clk   => clk_i,
    ena   => puls2_length_ena,
    div_o => puls2_o
    );

p_puls2:  process (clk_i)
  begin
    if rising_edge(clk_i) then
      if (timing_received and ev_filt_on and fi_ev_puls2_s) = '1' then
        puls2 <= '1';
      elsif puls2 = '1' then
        if puls2_frame = '1' then
          if (timing_received and ev_filt_on and fi_ev_puls2_e) = '1' then
            puls2 <= '0';
          end if;
        else
          if puls2_o = '1' then
            puls2 <= '0';
          end if;
        end if;
      end if;
    end if;
  end process p_puls2;
  
ev_puls2 <= puls2;


p_ev_timer_res:  process (clk_i)
  begin
    if rising_edge(clk_i) then
      if (timing_received and ev_filt_on and fi_ev_timer_res and ev_reset_on) = '1' then
        ev_timer_res_o <= '1';
      else
        ev_timer_res_o <= '0';
      end if;
    end if;
  end process p_ev_timer_res;
  
ev_timer_res <= ev_timer_res_o;


p_mux_filter_addr:  process (filt_cntrl_sm, filter_addr, event_d, filt_addr)
  begin
    if filt_cntrl_sm = ev_rd or filt_cntrl_sm = ev_rd_fin then
      filter_addr <= event_d(filter_addr_width-1 downto 0);
    else
      filter_addr <= filt_addr;
    end if;
  end process p_mux_filter_addr;


p_mux_read_port:  process (filt_cntrl_sm, filt_data_o, event_o)
  begin
    if filt_cntrl_sm = fi_rd then
      read_port_o <= std_logic_vector(to_unsigned(0 ,(16-filter_data_width))) & filt_data_o;
    else
      read_port_o <= event_o;
    end if;
  end process p_mux_read_port;


stall_o <= fi_stall;

end arch_event_processing;

