library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.wishbone_pkg.all;

entity WBDebugMaster is
  generic(
    g_data_width  : integer := 32;
    g_burst       : integer := 1;
    g_addr_start  : unsigned(31 downto 0) := x"00000000";
    g_addr_end    : unsigned(31 downto 0) := x"00000010"
    );
  port(
    -- Common wishbone signals
    clk_i       : in  std_logic;
    rst_n_i     : in  std_logic;
    -- Slave control port
    master_i     : in  t_wishbone_master_in;
    master_o     : out t_wishbone_master_out;
    -- Current timestamp
    received_o  : out std_logic_vector(35 downto 0)
    );
end WBDebugMaster;

architecture rtl of WBDebugMaster is

type sSTATE is (sIDLE,sWRITE, sREAD, sDONE);

signal master    : t_wishbone_master_out;
signal ACKDONE : std_logic;
signal STBDONE : std_logic;
signal STARTOP : std_logic;
signal addr_cnt : unsigned(31 downto 0);
signal data_gen : unsigned(31 downto 0);
signal STBcnt : integer;
signal ACKERRcnt : integer;
  signal state : sState;
        


begin


master_o.CYC <= master.CYC;
master_o.STB <= master.CYC AND NOT STBDONE;
master_o.WE <= master.WE;
master_o.SEL <= master.SEL;
master_o.ADR <= std_logic_vector(addr_cnt);
master_o.DAT <= std_logic_vector(data_gen);


 
 ReplyCnt : process(clk_i)
  begin
    if rising_edge(clk_i) then
      if rst_n_i = '0' then
        STBDONE   <= '0';
          ACKDONE   <= '0';
          STBcnt    <= g_burst-1;
          ACKERRcnt <= g_burst;
      else
        if (STARTOP = '1') then
          STBDONE   <= '0';
          ACKDONE   <= '0';
          STBcnt    <= g_burst-1;
          ACKERRcnt <= g_burst;
       else
        received_o <= (others => '1');
        
        if(STBcnt <= 0) then
          STBDONE <= '1';
        else
          if(master.CYC = '1' and master.STB = '1' and master_i.STALL = '0') then
            STBcnt <= STBcnt -1; 
          end if;
        end if;
        
        if(ACKERRcnt <= 0) then
          ACKDONE <= '1';
        else
          if(master_i.ACK = '1' or master_i.ERR = '1') then
            ACKERRcnt <= ACKERRcnt -1;
          end if; 
        end if;
        
        if(master_i.ACK = '1' or master_i.ERR = '1') then
      
            if(state = sREAD) then
              received_o <= x"0" & master_i.DAT;
            end if;
          end if; 
        
      end if;
    end if;
  end if;  
end process;     
  
 main : process(clk_i)

 
 begin
    if rising_edge(clk_i) then
      if rst_n_i = '0' then
        addr_cnt      <=  g_addr_start;
        data_gen      <=  (others => '0');
        state <= sIDLE;       
        
      else
       STARTOP   <= '0';
       master.CYC  <= '0';
       master.STB  <= '0';
       master.WE   <= '0';
       master.SEL  <=  (others => '1');
                         
        
        case state is
          when sIDLE =>   
                          addr_cnt      <=  g_addr_start;
                            data_gen      <=  (others => '0');
                            STARTOP <= '1';
                            
                            state <= sWRITE;
                          
                         
          when sWRITE =>  if(ACKDONE = '1' AND STARTOP = '0') then
                            state <=  sREAD;
                            STARTOP <= '1';
                            --addr_cnt      <=  addr_cnt + 1;
                          else
                            master.CYC  <= '1';
                            master.WE   <= '1';
                            master.STB  <= '1';
                            if(master_i.STALL = '0') then
                                --addr_cnt      <=  addr_cnt + 1;
                                data_gen      <=  data_gen + 1;
                            end if;
                          end if;
                          
          when sREAD =>   if(ACKDONE = '1' AND STARTOP = '0') then
                            if((addr_cnt + g_data_width/4) < g_addr_end) then
                              state <= sWRITE;
                              STARTOP <= '1';
                              --addr_cnt      <=  addr_cnt + 1;
                            else
                              state <= sDONE;
                            end if;
                          else
                            master.CYC  <= '1';
                            master.WE   <= '0';
                            master.STB  <= '1';
                            if(master_i.STALL = '0') then
                                
                            end if;
                          end if;
                          
          when sDONE =>   state <=  sIDLE;
                        
          when others =>   state <=  sIDLE; 
        end case;
   end if;
 end if;         
end process;

end architecture;    
  
  
  
  