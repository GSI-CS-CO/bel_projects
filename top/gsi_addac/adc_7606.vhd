-- Filename: adc_7606.vhd
-- Date :    01.07.2011
-- Author:   Manishkumar Dudhat

-- Inhalt:
-- Implemenation of 16bit ADCs with FPGA Board (ALTERA CYCLON IV)
-- Company: GSI Helmholtzzenrum f�r Schwerionenforchung GmbH

-- ADC Description: AD7606, 16-bits, 6 Channels, power supply : +5V
-- LSB = 152 uV bei Eingangsspannung von +/-5V
-- Mode : Parallel mode used, Wandlung aller Kan�le in 3�s simultant

--=================================================================================================
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;
USE ieee.math_real.ALL;

--=================================================================================================

entity adc_7606 is

GENERIC (
  
    clk_in_hz     : integer  := 100_000_000;     -- 50Mhz system clock
 
    reset_delay   : integer  := 50;             -- RESET high pulse with        (treset)
    conv_wait     : integer  := 25;             -- Minimum delay between Reset low to convst high (t7)
    firstdata_time: integer  := 50;             -- Minimum delay between busy  low to firstdata 'high'(t13+t26)
    
    rd_low        : integer  := 25;             -- RD low pulse width           (t10)
    rd_high       : integer  := 15;             -- RD high pulse width          (t11)
    os_previous             : std_logic_vector(2 downto 0) := "000"; --Digitalfileter "000" inaktiv
    adc_range_previous      : std_logic := '1' ); -- '1' = +/-10 ; '0'= +/-5V Input voltage
   
   
-- Port Declaration 
PORT (
   
   --Interne Anschlu�e f�r Entity 
    clk           : in  std_logic;                        -- Clock Signal f�r FPGA Entity
    rst           : in  std_logic;                        -- reset Signal f�r FPGA Entity
    start_conv    : in  std_logic;                        -- Startet den Konvertierungsablauf
   
   -- Ausg�nge der ausgewerteten Kan�le  
    channel_1     : out std_logic_vector (15 downto 0):= (others => '0');   
    channel_2     : out std_logic_vector (15 downto 0):= (others => '0');
    channel_3     : out std_logic_vector (15 downto 0):= (others => '0');
    channel_4     : out std_logic_vector (15 downto 0):= (others => '0');
    channel_5     : out std_logic_vector (15 downto 0):= (others => '0');
    channel_6     : out std_logic_vector (15 downto 0):= (others => '0');
 
   
    --Anschlu�e an ADC
    busy          : in  std_logic;                        -- falling adge signals shows end of conversion
    firstdata     : in  std_logic;                        -- Anzeige der Konversion f�r den ersten Kanal
    db            : in  std_logic_vector (15 downto 0);   -- 16 bit data bus from the ADC  
    
    convst        : out std_logic;                        -- Startet die Konvertierung
    n_cs          : out std_logic;                        -- chip select enables tri state databus
    n_rd_sclk     : out std_logic;                        -- first falling edge after busy clocks data out
    os            : out std_logic_vector(2 downto 0);     -- Oversampling Configuration (Dig. Filter)
    adc_range     : out std_logic;                        -- '1' = +/-10V; '0' = +/-5V
    adc_reset     : out std_logic;                        -- reset f�r ADC
    Data_Out_Valid: out std_logic := '0');                -- ='1' falls alles fertig 
  
END adc_7606;  

--=================================================================================================

--Architectur Declaration
ARCHITECTURE  behavioral OF adc_7606 IS


TYPE channel_reg_type IS ARRAY (0 to 5) OF STD_LOGIC_VECTOR (15 downto 0); -- Array for 6 Registers
SIGNAL s_channel_regs : channel_reg_type ;

TYPE state_type IS (reset,              -- ADC wird resetet 
                    idle,               -- ADC wird in Ruhezustand geschaltet
                    conv_st,            -- Starte wird gestartet 
                    wait_for_busy,      -- busy = '1' Wandlung l�uft; = '0' Wandlung fertig  
                    wait_for_conv_fin,  -- Konvertierung ist abgeschlossen
                    wait_firstdata,     -- Die Daten von dem Ersten Kanal werden abgewartet
                    channel_data_ready, -- Die Daten sind bereit
                    wait_rd_low,        -- n_RD wird auf 'low' gezogen
                    wait_rd_high,       -- n_RD wird auf 'high' gezogen
                    data_output);       -- alle Ausg�nge werden gleuchzeitig ausgegeben
 
SIGNAL s_conv_state           :  state_type            := reset;  --Statemaschine Signal
SIGNAL s_channel_cnt          :  integer range 0 to 6  := 0;      -- 6 channels counter

--===================================================================================================
-- Hier werden die erforderliche Taktzahlen f�r die Einhaltung der vorgegebenen Zeiten errechnet 

-- [1] Anzahl der aufgerundeten Takte f�r die Zeit treset 
CONSTANT c_reset_delay          : integer := integer
                            (ceil (real (clk_in_hz) / real (1_000_000_000)*real (reset_delay)));  

--Wie vile Bit braucht man um diese Zahl bin�r darzustellen?
CONSTANT c_reset_delay_width    : integer  := integer(ceil (log2 (real(c_reset_delay))));

SIGNAL   s_reset_delay          : unsigned (c_reset_delay_width-1 downto 0)  := (others => '0');


-- [2] Anzahl der aufgerundeten Takte f�r die Zeit t7 - Minimum delay between Reset low to convst high
CONSTANT c_conv_wait            : integer  := integer
                            (ceil (real (clk_in_hz) / real (1_000_000_000) *real (conv_wait))); 
                                                            
CONSTANT c_conv_wait_width      : integer  := integer(ceil (log2 (real(c_conv_wait))));

SIGNAL   s_conv_wait            : unsigned (c_conv_wait_width-1 downto 0)  := (others => '0');

--[3]
-- Das ist die Zeit, die vergeht vom convst high bis die busy Leitung auch auf high wechselt (t1)
-- Diese Zeit betr�gt laut dem Datenblatt 40ns
-- es wird jedoch so lange gewartet bis die busy Leitung auf high geht somit ist die einhaltung 
-- dieser zeit automatisch gegeben

--[4]
-- Diese zeit ist erforderlich f�r die Konvertierung und ist abh�ngig von der Anzahl der 
-- ADC Kan�le und ob die OS eingeschaltet ist


-- [5] Diese Zeit ist erforderlich bei �bergang busy = 'low' bis zu daten des ersten Kanals abrufbereit sind
-- t13 + t26 = 25ns + 25ns

CONSTANT c_wait_first_data       : integer  := integer
                            (ceil (real (clk_in_hz) / real (1_000_000_000) *real (firstdata_time))); 
                                
CONSTANT c_wait_first_data_width : integer  := integer(ceil (log2 (real(c_wait_first_data))));

SIGNAL   s_wait_first_data       : unsigned (c_wait_first_data_width-1 downto 0)  := (others => '0');






--[6.1] Diese Zeit ist erforderlich f�r die Auslesung jedes Kanals (t10) n_RD= 'low'
CONSTANT c_wait_rd_low        : integer  := integer
                            (ceil (real (clk_in_hz) / real (1_000_000_000) *real (rd_low)));
                            
CONSTANT c_wait_rd_low_width  : integer  := integer(ceil (log2 (real(c_wait_rd_low))));

SIGNAL   s_wait_rd_low        : unsigned (c_wait_rd_low_width-1 downto 0)  := (others => '0');


--[6.2] Diese Zeit ist erforderlich f�r die Auslesung jedes Kanals (t10) n_RD= 'high'
CONSTANT c_wait_rd_high       : integer  := integer
                            (ceil (real (clk_in_hz) / real (1_000_000_000) *real (rd_high))); 
                                
CONSTANT c_wait_rd_high_width : integer  := integer(ceil (log2 (real(c_wait_rd_high))));

SIGNAL   s_wait_rd_high       : unsigned (c_wait_rd_high_width-1 downto 0)  := (others => '0');

 --=================================================================================================
 
BEGIN

os              <= os_previous;
adc_range       <= adc_range_previous;

-- Conversion Cycle Process Definition
conv_cycle : process (clk, rst)

BEGIN 
  
    if (rst ='1') then
    
        s_conv_state    <= reset; --der n�chste Zustand reset
        n_cs            <= '1';
        n_rd_sclk       <= '1';
        s_channel_cnt   <= 0; 
        adc_reset       <= '1';
        convst          <= '1';
        Data_Out_Valid  <= '0';
 
    elsif (rising_edge(clk) and clk = '1') then
     
        CASE s_conv_state IS        --State Machine Definition
       
--=================================================================================================     
    when reset =>  --[1. Zustand] 50ns
--================================================================================================= 
        -- Dieser Zustand dauert die vorgegebene Zeit "treset"
        
        if s_reset_delay    = to_unsigned (c_reset_delay, c_reset_delay_width) then
            s_conv_state     <= idle;            
            Data_Out_Valid  <= '0';
            s_reset_delay  <= (others =>'0');     --der Signal reset delay wird auf Null gesetzt
        else
            s_reset_delay  <= s_reset_delay + 1 ; -- bleibt so lange in dem Zustand bis die 
            s_conv_state    <= reset;              -- Reset Zeit (treset) eingehalten wird
        end if;
 
--=================================================================================================
        when idle =>    --[2. Zustand] wird von A��ere Beschaltung bestimmt
--=================================================================================================
            -- Der FPGA wartet bis der Startsignal f�r die Abtastung gegeben wird
            -- Der ADC ist Bereit und wartet
            adc_reset       <= '0';


            if (start_conv = '1') then
                s_conv_state  <= conv_st;
            else 
                s_conv_state  <= idle;
            end if; 
            
--=================================================================================================
    when conv_st =>             --[2. Zustand] 20ns
--=================================================================================================
        -- Dieser Zustand dauert die Zeit t7 = t2 und entspricht dem
        -- Minimum delay between Reset low to convst high
        
        if s_conv_wait = to_unsigned (c_conv_wait, c_conv_wait_width) then
            s_conv_state  <= wait_for_busy;
            convst      <= '1';
            s_conv_wait <= (others => '0');
        else 
            convst      <= '0';
            s_conv_wait <= s_conv_wait +1;
            s_conv_state  <= conv_st;
        end if;
   
--==================================================================================================
    when wait_for_busy =>       --[3.Zustand] 40ns
 --=================================================================================================
        -- Wenn der busy Signal = '1' bedeutet das, dass die Wandlung angefangen hat (t1)
        Data_Out_Valid  <= '0';
        
        if busy = '1' then
            s_conv_state <= wait_for_conv_fin;
        else
            s_conv_state <= wait_for_busy;
        end if;
      
--=================================================================================================
    when wait_for_conv_fin =>   --[4.Zustand] 3�s wenn OS ausgeschaltet ist 
--=================================================================================================
        -- Wird der Digitalfilter aktiviert (OS) so wird die Zeit f�r die Wandlung gr��er
        -- hier sind die Zeiten aus dem Datenblatt f�r 8 kan�ligen ADC angegeben 
        -- OS Ratio 2 -  9,1�s
        -- OS Ratio 4 -  18,8�s
        -- OS Ratio 8 -  39�s
        -- OS Ratio 16 - 78�s
        -- OS Ratio 32 - 158�s
        -- OS Ratio 64 - 315�s
        if (busy = '1') then
           s_conv_state     <= wait_for_conv_fin;
        
        elsif (busy = '0' and s_wait_rd_low  = to_unsigned (c_wait_rd_low-1, c_wait_rd_low_width)) then  -- die Wandlung ist fertig 
            s_wait_rd_low   <= (others => '0');
            s_conv_state    <= wait_firstdata;
        else 
            s_wait_rd_low   <= s_wait_rd_low + 1;
            s_conv_state    <= wait_for_conv_fin;
        end if;

--=================================================================================================
    when wait_firstdata =>            --[5.Zustand] 25ns
--=================================================================================================
        
        
        if s_wait_first_data = to_unsigned (c_wait_first_data, c_wait_first_data_width) then
            s_conv_state          <= channel_data_ready;
            s_wait_first_data   <= (others => '0');
        else 
            s_wait_first_data   <= s_wait_first_data + 1;
            
            n_cs      <= '0';  --Chipselect wird sofort auf '0'gesetzt
            n_rd_sclk <= '0';  --n_RD wird auch sofort auf '0'gesetzt
            s_conv_state        <= wait_firstdata;
        end if;
        
--=================================================================================================
    when channel_data_ready =>          --[6.Zustand]
--=================================================================================================
        -- Um die Daten auszulesen mu�
        -- die n_RD Leitung jedes Mal einmal Low- und High- Zustand annehmen 
       
        if s_channel_cnt < 6 then
            s_channel_cnt   <= s_channel_cnt + 1; --Z�hler f�r die Kan�le 
            s_conv_state      <= wait_rd_low;
        
        elsif s_wait_rd_low  = to_unsigned (c_wait_rd_low-1, c_wait_rd_low_width) then
            s_wait_rd_low   <= (others =>'0');
            n_cs            <= '1';
            n_rd_sclk       <= '1';
            s_channel_cnt   <= 0;
            s_conv_state      <= data_output;
             
        else 
            s_wait_rd_low   <= s_wait_rd_low + 1;
            n_rd_sclk       <= '0'; 
            s_conv_state      <= channel_data_ready;
        end if;
     
--=================================================================================================
    when wait_rd_low =>         --[6.1. Zustand] 25ns
--=================================================================================================
        if s_wait_rd_low     = to_unsigned (c_wait_rd_low-2, c_wait_rd_low_width) then
            s_wait_rd_low   <= (others =>'0');
            n_rd_sclk       <= '1';  --Hold rd_low for c_wait_rd_low (t10)
            s_conv_state      <=  wait_rd_high; 
        else 
            s_wait_rd_low   <= s_wait_rd_low + 1;
            n_rd_sclk <= '0'; 
            s_conv_state      <= wait_rd_low;
        end if;
   
--=================================================================================================
    when wait_rd_high =>        --[6.2 Zustand] 15ns
--=================================================================================================

        if s_wait_rd_high = to_unsigned (c_wait_rd_high-1, c_wait_rd_high_width) then 
            s_wait_rd_high                      <= (others =>'0');
            n_rd_sclk                           <= '0';
            s_channel_regs(s_channel_cnt-1)     <= db;  -- Die Daten des ADC werden in den Register 
            s_conv_state                          <= channel_data_ready;                                               -- geschrieben
        else 
            --Hold rd_low for c_wait_rd_low (t11) 
            n_rd_sclk      <= '1';
            s_wait_rd_high <=  s_wait_rd_high + 1;
            s_conv_state     <=  wait_rd_high;
        end if;

--=================================================================================================
    when data_output =>        --[7 Zustand] 1Takt
--=================================================================================================

        channel_1       <= s_channel_regs(0);
        channel_2       <= s_channel_regs(1);
        channel_3       <= s_channel_regs(2);
        channel_4       <= s_channel_regs(3);
        channel_5       <= s_channel_regs(4);
        channel_6       <= s_channel_regs(5);
        
        Data_Out_Valid  <= '1';
        s_conv_state    <= idle;
--=================================================================================================
    when others =>  
--=================================================================================================
        s_conv_state      <= idle;
    
   END CASE;
 END IF;
 END process;

END ARCHITECTURE behavioral;
