LIBRARY ieee;
use ieee.std_logic_1164.all; 
use ieee.numeric_std.all;

--+-----------------------------------------------------------------------------------------------------------+
--| "hw6408_vhdl" realisiert das Interface zu einem externen Manchester-En-Decoder-Baustein, Type 6408.       |
--| Es ist als Ersatz für das ältere in AHDL geschriebene Interface "ser_para.tdf" vorgesehen.                |
--|                                                                                                           |
--| Autor: W.Panschow; Datum 7.05.14                                                                          |
--+-----------------------------------------------------------------------------------------------------------|
--| K.Kaiser 21-Jul-2017  DSC Denoised, TD SDO TD and vw synchronized by shiftreg due to Harris DSC Problems  |
--+-----------------------------------------------------------------------------------------------------------|



entity hw6408_vhdl is
  generic (
    clk_in_hz:  integer := 125_000_000
    );
  port (
    data_i:       in  std_logic_vector(15 downto 0) := (others => '0');  -- Eingang fuer das zu sendende Datum/Komando
    wr_mil:       in  std_logic := '0';               -- '1' => data_i wird uebernommen und des Senden gestartet.
    mil_send_cmd: in  std_logic := '0';               -- '1' => data_i wird als Kommando, '0' => data_i wird als Datum
                                                      -- gesendet. Vorgabe muss waehrend  wr_mil = '1' stabil sein.
    shift_sd:     in  std_logic := '0';               -- verbinde mit HD6408(sd). '1' der serielle Sende-Datenstrom an sd_o
                                                      -- (mit encoder shift clock getaktet) wird erwartet.
    esc:          in  std_logic := '0';               -- hw6408(esc) liefert den encoder shift clock.
    sdi:          out std_logic;                      -- verbinde mit hw6408(sdi) = serieller Sende-Datenstrom
    ee:           out std_logic;                      -- hw6408(ee) = encoder enable.
    ss:           out std_logic;                      -- hw6408(ss) = syc select. '1' => Kommando Sync, '0' => Data sync.
    boo_n:        in  std_logic := '0';               -- verbinde mit HD6408(nBOO) = encoder bipolar one out.
    bzo_n:        in  std_logic := '0';               -- verbinde mit HD6408(nBZO) = encoder bipolar zero out.
    trm_ena_n:    out std_logic;                      -- '0' => der externe Sendetreiber wird selektiert.
                                                        
    rd_mil:       in  std_logic := '0';               -- liest das empfangene Datum oder Kommando.
    data_o:       out std_logic_vector(15 downto 0);  -- Ausgang fuer das empfangene Datum/Kommando.
    cds:          in  std_logic := '0';               -- verbinde mit HD6408(cds) = command data sync.
                                                      -- '1' => Rcv CMD, '0' =>  Rcv Data.
    vw:           in  std_logic := '0';               -- verbinde mit HD6408(vw) = valid word.
    td:           in  std_logic := '0';               -- verbinde mit HD6408(td) = take data.
    dsc:          in  std_logic := '0';               -- verbinde mit HD6408(dsc) = decoder shift clock.
    sdo:          in  std_logic := '0';               -- verbinde mit HD6408(sdo) = Rcv serial data out.
    rcv_ena_n:    out std_logic;                      -- '0' der externe Empfangspuffer wird selektiert.
    
    rcv_cmd:      out std_logic;                      -- Statusbit: '1 '=> ein Kommando wurde empfangen.
    rcv_err:      out std_logic;                      -- Statusbit: '1' => ein fehlerhaftes Telegramm wurde empfangen. 
    valid_w:      out std_logic;                      -- Statusbit: '1' => ein Telegram, wurde empfangen.
    trm_rdy:      out std_logic;                      -- Statusbit: '1' => Sender ist frei.
    
    nrst_i:       in  std_logic;
    clk_i:        in  std_logic;
    
    res_6408:     out std_logic := '0';               -- verbinde mit HD6408(mr) = master reset.
    sel_6408:     in  std_logic := '0'                -- '1' => hw6408_vhd kann betrieben werden. '0' => hw6408_vhd
                                                      -- ausgeschaltet.
  );
  end hw6408_vhdl;

  
 

architecture arch_hw6408_vhdl of hw6408_vhdl is

type  t_vw_sm is (
        idle_vw,
        wait_vw,
        err_vw
        );

signal  vw_sm: t_vw_sm;

signal  vw_set:         std_logic := '0';   -- achtung mit esc getaktet
signal  rcv_err_set:    std_logic := '0';   -- achtung mit esc getaktet

signal  send_data:      std_logic_vector(15 downto 0) := (others => '0');
signal  send_init:      std_logic := '0';
signal  send_cmd:       std_logic := '0';
signal  par_in_ser_out: std_logic_vector(15 downto 0) := (others => '0');

signal  ser_in_par_out: std_logic_vector(15 downto 0) := (others => '0');
signal  rcv_reg:        std_logic_vector(15 downto 0) := (others => '0');
signal  vw_reg:         std_logic := '0';
signal  rcv_err_reg:    std_logic := '0';
signal  rcv_cmd_reg:    std_logic := '0';
signal  nrcv_ena_shiftr:std_logic_vector(1 downto 0) := (others => '1');


constant  c_vw_tst_cnt: integer := 16;        -- eigentlich muss c_vw_cnt = 17 sein, es geht aber ein count
                                              -- durch das Verlassen des states 'idle_vw' verloren.
constant  c_end_vw_tst: integer := c_vw_tst_cnt + 1;    -- valid word kommt zu spaet, oder garnicht mehr.
signal    vw_cnt:       integer range 0 to c_end_vw_tst + 1;  -- + 1 damit kein Ueberlauf auftreten kann.

signal    s_ee:         std_logic := '0';     -- signal encoder enable
signal    s_trm_ena:    std_logic := '0';

signal dsc_sh1:         std_logic;
signal dsc_sh2:         std_logic;
signal dsc_denoised:    std_logic;

signal td_sh:           std_logic_vector(3 downto 0);
signal cds_sh:          std_logic_vector(3 downto 0);
signal vw_sh:           std_logic_vector(3 downto 0);
signal sdo_sh:          std_logic_vector(3 downto 0);

	 
begin

p_dsc_denoise : PROCESS (clk_i, nrst_i)   --KK
BEGIN
  IF (nrst_i = '0') THEN
    dsc_sh1            <= '0';
    dsc_sh2            <= '0';
    dsc_denoised       <= '0';
	 td_sh              <= (others =>'0');
	 cds_sh             <= (others =>'0');
	 vw_sh              <= (others =>'0');
	 sdo_sh             <= (others =>'0');
  ELSIF rising_edge(clk_i) THEN
	 td_sh (0)          <= td;
	 td_sh (3 downto 1) <= td_sh(2 downto 0 ); 
	 
	 cds_sh (0)         <= cds;
	 cds_sh (3 downto 1)<= cds_sh(2 downto 0 ); 
	 
	 vw_sh (0)          <= vw;
	 vw_sh (3 downto 1) <= vw_sh(2 downto 0 ); 
	 
	 sdo_sh (0)         <= sdo;
	 sdo_sh (3 downto 1)<= sdo_sh(2 downto 0 ); 	
	
    --dsc rise/fall times are max 8ns, should be stable in 16ns
    dsc_sh1            <= dsc;
    dsc_sh2            <= dsc_sh1;
	 	 
    IF    dsc_sh1 = '1' AND dsc_sh2 = '1' THEN
      dsc_denoised <= '1';
    ELSIF dsc_sh1 = '0' AND dsc_sh2 = '0' THEN
      dsc_denoised <= '0';
    ELSE
      NULL; 
    END IF;
	 
  END IF;
END PROCESS p_dsc_denoise;
  
  

p_res_6408:  process(clk_i, nrst_i, sel_6408)

  constant  c_wait_cnt: integer := clk_in_hz / (1_000_000 / 2);   -- cnt fuer 2 us.
  variable  wait_cnt:   integer := 0;
  
  begin
    if (nrst_i = '0') or (sel_6408 = '0') then
      wait_cnt := 0;
      res_6408 <= '1';
    elsif rising_edge(clk_i) then
      if wait_cnt < c_wait_cnt then
        wait_cnt := wait_cnt + 1;
        res_6408 <= '1';
      else
        res_6408 <= '0';
      end if;
    end if;
  end process p_res_6408;

  
p_send_data:  process (clk_i, nrst_i, sel_6408)
  begin
    if (nrst_i = '0') or (sel_6408 = '0') then
      send_data <= (others => '0');
      send_init <= '0';
      send_cmd  <= '0';
    elsif rising_edge(clk_i) then
      if wr_mil = '1' then
        send_data <= data_i;
        send_init <= '1';
        if mil_send_cmd = '1' then
          send_cmd <= '1';
        else
          send_cmd <= '0';
        end if;
      elsif shift_sd = '1' then
        send_init <= '0';
      end if;
    end if;
  end process p_send_data;

ss <= send_cmd;


p_trm_rdy:  process (clk_i, nrst_i, sel_6408)
  variable  shift_sd_edge:  std_logic_vector(2 downto 0) := (others => '0');
  begin
    if (nrst_i = '0') or (sel_6408 = '0') then
      shift_sd_edge := (others => '0');
      trm_rdy <= '1';
    elsif rising_edge(clk_i) then
      shift_sd_edge := shift_sd_edge(1 downto 0) & shift_sd;
      if wr_mil = '1' then
        trm_rdy <= '0';
      elsif shift_sd_edge(2 downto 1) = "10" then
        trm_rdy <= '1';
      end if;
    end if;
  end process p_trm_rdy;
  

p_trm_ena:  process (clk_i, nrst_i, sel_6408)
  begin
    if (nrst_i = '0') or (sel_6408 = '0') then
      s_trm_ena <= '0';
    elsif rising_edge(clk_i) then
      s_trm_ena <= boo_n xor bzo_n;
    end if;
  end process p_trm_ena;
  
trm_ena_n <= not s_trm_ena;


p_par_ser:  process (esc, nrst_i, sel_6408)
  begin
    if (nrst_i = '0') or (sel_6408 = '0') then
      par_in_ser_out <= (others => '0');
    elsif falling_edge(esc) then
      if send_init = '1' then
        par_in_ser_out <= send_data;
      elsif shift_sd = '1' then
        par_in_ser_out <= par_in_ser_out(14 downto 0) & '0';
      end if;
    end if;
  end process p_par_ser;

sdi <= par_in_ser_out(15);

p_ee: process (esc, nrst_i, sel_6408)
  begin
    if (nrst_i = '0') or (sel_6408 = '0') then
      s_ee <= '0';
    elsif falling_edge(esc) then
      s_ee <= send_init;
    end if;
  end process p_ee;
  
ee <= s_ee;
  
  
p_vw_sm:  process (dsc_denoised, nrst_i)    
  begin
    if nrst_i = '0' then
      vw_sm <= idle_vw;
      vw_set <= '0';
      rcv_err_set<= '0';
      vw_cnt <= 0;            -- clear valid word test counter
 
    elsif rising_edge(dsc_denoised) then

      vw_set <= '0';
      rcv_err_set<= '0';
      vw_cnt <= vw_cnt + 1;   -- ausser im 'idle_vw'-state, wird valid word test counter immer inkrementiert.
      
      case vw_sm is

        when idle_vw =>
          vw_cnt <= 0;        -- clear valid word test counter
          if td_sh(3) = '1' then
            vw_sm <= wait_vw;
          end if;
          
        when wait_vw =>       -- wait for valid word
          if vw_cnt = c_end_vw_tst then   -- valid word kommt zu spät, oder garnicht.
            vw_sm <= err_vw;              -- gehe in den Fehlerstate
          elsif vw_sh(3) = '1' then             -- valid word kommt
            if vw_cnt = c_vw_tst_cnt then -- es kommt zum richtigen Zeitpunkt
              vw_set <= '1';              -- valid word set ist fuer einen 'esc'-Takt aktiv.
              vw_sm <= idle_vw;           -- gehe in den 'idle_vw'-state
            else
              vw_sm <= err_vw;            -- valid word kam zu früh, gehe in den Fehlerstate
            end if;
          end if;
          
        when err_vw =>        -- error state
          rcv_err_set <= '1';             -- reveive error set ist fuer einen 'esc'-Takt aktiv.
          vw_sm <= idle_vw;
        
        when others => 
          vw_sm <= idle_vw;
        
      end case;
    end if;
  end process p_vw_sm;


p_ser_par:  process (dsc_denoised, nrst_i, sel_6408)
  begin
    if (nrst_i = '0') or (sel_6408 = '0') then
      ser_in_par_out <= (others => '0');
    elsif rising_edge(dsc_denoised) then
      if td_sh(3) = '1' then
        ser_in_par_out <= ser_in_par_out(14 downto 0) & sdo_sh(3);
      end if;
    end if;
  end process p_ser_par;

    
p_rcv_reg:  process (clk_i, nrst_i, sel_6408)
  begin
    if (nrst_i = '0') or (sel_6408 = '0') then
      rcv_reg <= (others => '0');
    elsif rising_edge(clk_i) then
      rcv_reg <= ser_in_par_out;
    end if;
  end process p_rcv_reg;

data_o <= rcv_reg; 


p_rcv_cmd:  process (dsc_denoised, nrst_i, sel_6408, rd_mil)
  begin
    if (rd_mil = '1') or (nrst_i = '0') or (sel_6408 = '0') then
      rcv_cmd_reg <= '0';
    elsif falling_edge(dsc_denoised) then
      if td_sh(3) = '1' then
        rcv_cmd_reg <= cds_sh(3);
      end if;
    end if;
  end process p_rcv_cmd;

  
p_vw: process (clk_i, nrst_i, sel_6408)  
  begin
    if (nrst_i = '0') or (sel_6408 = '0') then
      vw_reg <= '0';
    elsif rising_edge(clk_i) then
      if vw_set = '1' then
        vw_reg <= '1';
      elsif rd_mil = '1' then
        vw_reg <= '0';
      end if;
    end if;
  end process p_vw;  
  

p_rcv_ena: process (esc, nrst_i, sel_6408, s_ee)  
  begin
    if (nrst_i = '0') or (sel_6408 = '0') or (s_ee = '1') then -- senden (s_ee = '1)' nrcv_ena_shiftr wird inaktive  (='11')
      nrcv_ena_shiftr <= (others => '1');
    elsif falling_edge(esc) then
      nrcv_ena_shiftr <= nrcv_ena_shiftr(0) & (send_init or shift_sd);  -- nach einem senden wird 2 esc-Takte gewartet
                                                                        -- bis rcv_ena_shiftr(1 ) aktiv null wird.
    end if;
  end process p_rcv_ena;
  
rcv_ena_n <= nrcv_ena_shiftr(1);
valid_w   <= vw_reg;
rcv_err   <= rcv_err_set;


end arch_hw6408_vhdl;
