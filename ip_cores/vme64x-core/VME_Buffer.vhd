library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.VME_Buffer_pack.all;
use work.xVME64x_pack.all;

-- Buffer direction
-- VME to FPGA -> _v2f_ in schematics ab, negative logic
-- FPGA to VME -> _f2v_ in schematics ba, negative logic

entity VME_Buffer_ctrl  is
   generic(
      g_bus_mode        : bus_mode  := LATCHED); -- CLOCK         
      
   port( 
      clk_i             : in  std_logic;
      rst_i             : in  std_logic;
      buffer_stat_i     : in  t_VME_BUFFER;
      
      buffer_clk_o      : out std_logic;

      data_buff_v2f_o   : out std_logic;
      data_buff_f2v_o   : out std_logic;

      addr_buff_v2f_o   : out std_logic;
      addr_buff_f2v_o   : out std_logic;

      latch_buff_o      : out std_logic
   );
end entity;

architecture behavioural of VME_Buffer_ctrl is

   signal   pre_clk  :  std_logic := '0';
begin
	
   clock_buff_gen :  process(clk_i, rst_i)
   begin
   
      if rising_edge(clk_i)   then
         if rst_i =  '0'   then
            buffer_clk_o      <= '0';
				pre_clk				<= '0';
        else
			  pre_clk <= buffer_stat_i.s_clk;
			  
           if (pre_clk = buffer_stat_i.s_clk) or  
              (buffer_stat_i.s_clk = '0') then

              buffer_clk_o    <= '0';

           else   -- There is a transition 

              buffer_clk_o    <= '1';
           end if;
         end if;
      end if;
   end process;
	
-- VME -> FPGA, WRITE from VME_bus s_DataDir = 0
-- FPGA -> VME, READ  from FPGA    s_DataDir = 1
	with buffer_stat_i.s_buffer_eo select
		addr_buff_v2f_o	<= buffer_stat_i.s_addrDir 		when	ADDR_BUFF,	
									'1'									when	DATA_BUFF,
									buffer_stat_i.s_addrDir			when  DATA_ADDR_BUFF;

	with buffer_stat_i.s_buffer_eo select
		addr_buff_f2v_o	<= not buffer_stat_i.s_addrDir	when	ADDR_BUFF,	
									'1'									when	DATA_BUFF,
									not buffer_stat_i.s_addrDir	when  DATA_ADDR_BUFF;
	
	with buffer_stat_i.s_buffer_eo select
		data_buff_v2f_o	<= '1'									when	ADDR_BUFF,	
									buffer_stat_i.s_dataDir			when	DATA_BUFF,
									buffer_stat_i.s_dataDir			when  DATA_ADDR_BUFF;	
								
	with buffer_stat_i.s_buffer_eo select
		data_buff_f2v_o	<= '1'									when	ADDR_BUFF,	
									not buffer_stat_i.s_dataDir	when	DATA_BUFF,
									not buffer_stat_i.s_dataDir	when  DATA_ADDR_BUFF;

   with g_bus_mode   select
      latch_buff_o   <= '1'   when  LATCHED,
                        '0'   when  CLOCKED;

--   dir_eo_buff_ctrl   :  process(clk_i, rst_i)
--   begin
--      if rising_edge(clk_i)   then
--
--         if rst_i=   '0'   then                  
--            addr_buff_v2f_o    <= buffer_stat_i.s_addrDir;
--            addr_buff_f2v_o    <= not buffer_stat_i.s_addrDir;
--
--            data_buff_v2f_o    <= '1';
--            data_buff_f2v_o    <= '1';
--
--         else
--
--
--            case buffer_stat_i.s_buffer_eo is
--
--               when ADDR_BUFF       =>
--                  addr_buff_v2f_o    <= buffer_stat_i.s_addrDir;
--                  addr_buff_f2v_o    <= not buffer_stat_i.s_addrDir;
--
--                  data_buff_v2f_o    <= '1';
--                  data_buff_f2v_o    <= '1';
--
--               when DATA_BUFF       =>
--                  addr_buff_v2f_o    <= '1'; 
--                  addr_buff_f2v_o    <= '1';
--
--                  data_buff_v2f_o    <= buffer_stat_i.s_dataDir;
--                  data_buff_f2v_o    <= not buffer_stat_i.s_dataDir;
--               when DATA_ADDR_BUFF  =>
--                  addr_buff_v2f_o    <= buffer_stat_i.s_addrDir;
--                  addr_buff_f2v_o    <= not buffer_stat_i.s_addrDir;
--
--                  data_buff_v2f_o    <= buffer_stat_i.s_dataDir;
--                  data_buff_f2v_o    <= not buffer_stat_i.s_dataDir;
--            end case;
--            
--         end if;
--      end if;
--
--   end process;
   
end architecture;
