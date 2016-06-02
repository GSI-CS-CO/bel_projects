--! Register Map ************************
--! 0x00, wr, enable time counter start
--! 0x04, wr, start tai low 32 bit
--! 0x08, wr, start tai high 8 bit
--! 0x0c, wr, start cycle 
--! 0x10, wr, stop tai low 32 bit
--! 0x14, wr, stop tai high 8 bit
--! 0x18, wr, stop cycle

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.time_counter_pkg.all;

entity wb_slave_time_counter is
   port (
      clk_i          : in  std_logic;
      rst_n_i        : in  std_logic;
      wb_slave_i     : in  t_wishbone_slave_in;
      wb_slave_o     : out t_wishbone_slave_out;
      stat_reg_i     : in  t_counter_stat_reg;
      ctrl_reg_o     : out t_counter_ctrl_reg);

end wb_slave_time_counter;

architecture rtl of wb_slave_time_counter is	

  signal s_stat: t_counter_stat_reg := c_counter_stat_default;
  signal s_ctrl: t_counter_ctrl_reg := c_counter_ctrl_default;
	
begin

   -- this wb slave doesn't supoort them
  wb_slave_o.int <= '0';
  wb_slave_o.rty <= '0';
  wb_slave_o.err <= '0';

  wb_process : process(clk_i)
    begin

      if rising_edge(clk_i) then
        if rst_n_i = '0' then
          s_stat            <= c_counter_stat_default;
          s_ctrl            <= c_counter_ctrl_default;
          wb_slave_o.ack    <= '0';
          wb_slave_o.dat    <= (others => '0');
       else
         wb_slave_o.ack     <= wb_slave_i.cyc and wb_slave_i.stb;

         if wb_slave_i.cyc = '1' and wb_slave_i.stb = '1' then
           case wb_slave_i.adr(5 downto 2) is
             when "0000"   => -- enable time counter start 0x0
               if wb_slave_i.we = '1' then
                 s_ctrl.s_counter_start        			<= wb_slave_i.dat(0);
               end if;
               wb_slave_o.dat(0) 			        <= s_ctrl.s_counter_start;
               wb_slave_o.dat(31 downto 1) 		<= (others => '0');
            when "0001"   => -- read start tai 0x4
              if wb_slave_i.we = '0' then
                wb_slave_o.dat(31 downto 0)  <= s_stat.s_counter_ts_start_tai (31 downto 0);
              end if;    
            when "0010"   => -- read start tai 0x8
              if wb_slave_i.we = '0' then
                wb_slave_o.dat(7 downto 0)  <= s_stat.s_counter_ts_start_tai (39 downto 32);
                wb_slave_o.dat(31 downto 8) <= (others => '0');
              end if;
            when "0011"   => -- read start cyc 0xC
              if wb_slave_i.we = '0' then
                wb_slave_o.dat(27 downto 0)  <= s_stat.s_counter_ts_start_cyc (27 downto 0);
                wb_slave_o.dat(31 downto 28) <= (others => '0');
              end if;

            when "0100"   => -- read stop tai 0x10
              if wb_slave_i.we = '0' then
                wb_slave_o.dat(31 downto 0)  <= s_stat.s_counter_ts_stop_tai (31 downto 0);
              end if;    
            when "0101"   => -- read stop tai 0x14
              if wb_slave_i.we = '0' then
                wb_slave_o.dat(7 downto 0)  <= s_stat.s_counter_ts_stop_tai (39 downto 32);
                wb_slave_o.dat(31 downto 8) <= (others => '0');
              end if;

             when "0110"   => -- read stop cyc 0x18
               if wb_slave_i.we = '0' then
                 wb_slave_o.dat(27 downto 0)  <= s_stat.s_counter_ts_stop_cyc;
                 wb_slave_o.dat(31 downto 28) <= (others => '0');
               end if;                  
             when others =>
           end case;
         end if;      
         s_stat     <= stat_reg_i; 
         ctrl_reg_o <= s_ctrl;
       end if;
     end if;
   end process;

end rtl;
