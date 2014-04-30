--! @file packet_capture.vhd
--! @brief Captures network packages on a 16b WB interface
--!
--! Copyright (C) 2011-2012 GSI Helmholtz Centre for Heavy Ion Research GmbH 
--!
--! Important details about its implementation
--! should go in these comments.
--!
--! @author Mathias Kreider <m.kreider@gsi.de>
--!
--! @bug No know bugs.
--!
--------------------------------------------------------------------------------
--! This library is free software; you can redistribute it and/or
--! modify it under the terms of the GNU Lesser General Public
--! License as published by the Free Software Foundation; either
--! version 3 of the License, or (at your option) any later version.
--!
--! This library is distributed in the hope that it will be useful,
--! but WITHOUT ANY WARRANTY; without even the implied warranty of
--! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
--! Lesser General Public License for more details.
--!  
--! You should have received a copy of the GNU Lesser General Public
--! License along with this library. If not, see <http://www.gnu.org/licenses/>.
---------------------------------------------------------------------------------

library IEEE;
--! Standard packages    
use IEEE.std_logic_1164.all;

use IEEE.numeric_std.all;

library work;
use work.wr_fabric_pkg.all;

entity packet_capture is
generic(g_filename : string := "123.pcap"; g_buffersize : natural := 1600);
port(
   clk_i          : in   std_logic;                                        --clock
   rst_n_i        : in   std_logic;
   
   rec_i          : in  std_logic;
   snk_i          : in  t_wrf_sink_in;
   snk_o          : out t_wrf_sink_out
   
);   
end entity packet_capture;

architecture behavioral of packet_capture is

type t_pcap_hdr is array (0 to 5) of std_logic_vector (31 downto 0);
constant pcap_hdr : t_pcap_hdr :=(  x"D4C3B2A1",   -- magic number pcap
                                    x"02000400",   -- major version
                                    x"00000000",   -- timezone offset
                                    x"00000000",   -- time accuracy
                                    x"ffff0000",   -- snapshot length def 65535
                                    x"01000000");  -- format is ethernet
                                             
                        
                                                  
type t_char_buff is array (g_buffersize-1 downto 0) of character; -- one byte each                              
type t_char_file is file of character; -- one byte each
file char_file : t_char_file;

type st is (IDLE, FILE_INIT, LISTEN, WAIT4START, FILE_WRITE, FILE_CLOSE);

--signal len : integer := 0;
signal r_stall       : std_logic;
signal s_ts_s,
       s_ts_us       : integer;
signal wordsize      : natural := 16;       
signal r_snk_i       : t_wrf_sink_in;
signal s_snk_o       : t_wrf_sink_out;
   
signal s_pkt_start,
       s_pkt_end     : std_logic;
signal r_state : st :=  IDLE;       
signal cbuf : t_char_buff;       
signal s_len : integer;



begin -- architecture sim

   s_ts_s  <= (now / 1000 ms);
   s_ts_us <= (now - (s_ts_s * 1000 ms)) / 1 us;
 
   s_pkt_start <=     snk_i.cyc and not r_snk_i.cyc;
   s_pkt_end   <= not snk_i.cyc and     r_snk_i.cyc;
   
   
   snk_o <= s_snk_o;
   
   s_snk_o.err <= '0';
   s_snk_o.rty <= '0';
   
   main : process(clk_i) is 
      variable this : std_logic_vector(31 downto 0);
      variable i, j : integer := 0;
      variable v_len, v_len_sum, v_n_pkt : integer;
      variable state : st :=  IDLE;
   begin
      if rising_edge(clk_i) then
         if(rst_n_i = '0') then
            s_snk_o.stall  <= '0';
            s_snk_o.ack    <= '0';
         else
            r_snk_i           <= snk_i;
            s_snk_o.ack       <= snk_i.cyc and snk_i.stb and not s_snk_o.stall;
            r_stall           <= s_snk_o.stall;
         
            state := r_state;
            s_len <= v_len;
            case state is
               when IDLE         => if(rec_i = '1') then
                                       state := FILE_INIT;
                                    end if;
                                    v_len_sum   := 0;
                                    v_n_pkt     := 1;   
                           
               when FILE_INIT  =>   file_open(char_file, g_filename, write_mode);
                                    for i in 0 to 5 loop
                                       this := pcap_hdr(i);
                                       for j in 3 downto 0 loop
                                           write(char_file, character'val(to_integer(unsigned(this((j+1)*8-1 downto j*8)))));
                                       end loop;
                                    end loop;
                                    state := WAIT4START;
                                      
               when WAIT4START   => v_len    := 0;
                                    if(s_pkt_start = '1') then
                                       state := LISTEN;
                                    end if;
                                    if(rec_i = '0') then
                                       state := FILE_CLOSE;
                                    end if; 
                           
               when LISTEN       => -- Listen to snk, write to buffer
                                    if( r_snk_i.cyc = '1' and r_snk_i.stb = '1' and r_stall = '0') then 
                                       for i in 0 to 1 loop
                                          cbuf(v_len + i) <= character'val(to_integer(unsigned(r_snk_i.dat((2-i)*8-1 downto (1-i)*8))));
                                       end loop;   
                                       v_len     := v_len +2;         
                                    end if;
                                    if(s_pkt_end = '1') then
                                       state := FILE_WRITE;
                                    end if;
                           
               when FILE_WRITE   => this := std_logic_vector(to_unsigned(s_ts_s, this'length));
                                    for j in 0 to 3 loop
                                       write(char_file, character'val(to_integer(unsigned(this((j+1)*8-1 downto j*8)))));
                                    end loop;
                                    this := std_logic_vector(to_unsigned(s_ts_us, this'length));
                                    for j in 0 to 3 loop
                                       write(char_file, character'val(to_integer(unsigned(this((j+1)*8-1 downto j*8)))));
                                    end loop;
                                    this := std_logic_vector(to_unsigned(v_len, this'length));
                                    for j in 0 to 3 loop
                                       write(char_file, character'val(to_integer(unsigned(this((j+1)*8-1 downto j*8)))));
                                    end loop;
                                    this := std_logic_vector(to_unsigned(v_len, this'length));
                                    for j in 0 to 3 loop
                                       write(char_file, character'val(to_integer(unsigned(this((j+1)*8-1 downto j*8)))));
                                    end loop;
                                    --this := std_logic_vector(to_unsigned(v_n_pkt, this'length));
                                    --for j in 0 to 3 loop
                                    --   write(char_file, character'val(to_integer(unsigned(this((j+1)*8-1 downto j*8)))));
                                    --end loop;
                                    
                                    for j in 0 to v_len-1 loop
                                       write(char_file, cbuf(j));
                                    end loop;
                                    report "Pkt " & integer'image(v_n_pkt) & " (" & integer'image(v_len) & " bytes) captured";
                                    
                                    v_len_sum   := v_len_sum   + v_len;
                                    v_n_pkt     := v_n_pkt     + 1;
                                    if(rec_i = '1') then
                                       state := WAIT4START;
                                    else
                                       state := FILE_CLOSE;
                                    end if;
                                    
                when FILE_CLOSE   => file_close(char_file);
                                    report "File " & g_filename & "closed, " & integer'image(v_n_pkt) & " (" & integer'image(v_len_sum) & " bytes total)" severity warning;                      state := IDLE;     
               
               when others       => state := IDLE;
            end case;   


            if(state = LISTEN) then
               s_snk_o.stall <= '0';
            else
               s_snk_o.stall <= '1';
            end if;
            
            r_state <= state;   
         end if; -- rst
      end if; -- rising edge      
      
end process;

end architecture;

