library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.wishbone_pkg.all;
use work.genram_pkg.all;

entity scu_to_wb is
  generic (
    Base_addr:      unsigned(15 downto 0);
    size:           integer;
    g_init_file:    string
  );
  port (
    clk_sys_i : in std_logic;
    rst_n_i   : in std_logic;

    -- Wishbone
    master_i : in  t_wishbone_master_in;
    master_o : buffer t_wishbone_master_out;
    

    -- SCU bus
    Adr_from_SCUB_LA:   in    std_logic_vector(15 downto 0);  -- latched address from SCU_Bus
    Data_from_SCUB_LA:  in    std_logic_vector(15 downto 0);  -- latched data from SCU_Bus
    Ext_Adr_Val:        in    std_logic;                      -- '1' => "ADR_from_SCUB_LA" is valid
    Ext_Rd_active:      in    std_logic;                      -- '1' => Rd-Cycle is active
    Ext_Wr_active:      in    std_logic;                      -- '1' => Wr-Cycle is active
    user_rd_active:     out   std_logic;                      -- '1' = read data available at 'Data_to_SCUB'-output
    Data_to_SCUB:       out   std_logic_vector(15 downto 0);  -- connect read sources to SCUB-Macro
    Dtack_to_SCUB:      out   std_logic                       -- connect Dtack to SCUB-Macro
  );
end entity;

architecture arch of scu_to_wb is

  signal rd_active:         std_logic;
  signal dtack:             std_logic;
  signal rd_active_dly:     std_logic;
  signal dtack_dly:         std_logic;
  constant scubus_width :   integer := 16;
  constant wishbone_width : integer := 32;
  constant c_wb_adr_h:        integer := 0;
  constant c_wb_adr_l:        integer := 1;
  constant c_wb_dat_h:        integer := 2;
  constant c_wb_dat_l:        integer := 3;
  constant c_wb_rdwrsel:      integer := 4;
  
  signal wrpulse:           std_logic;
  signal pulse1:            std_logic;
  signal pulse2:            std_logic;
  signal wb_dat:            std_logic_vector(31 downto 0);
  signal wb_adr:            std_logic_vector(31 downto 0);
  signal wb_rdwrsel:        std_logic_vector(15 downto 0);
  type    t_wb_cyc  is (idle, write, read, write_stalled, read_stalled, ack_err);
  signal  wb_state      : t_wb_cyc;
  
  
  
begin
  
  write_pulse: process(clk_sys_i, Ext_Wr_active, Ext_Adr_Val)
  begin
    if rising_edge(clk_sys_i) then
      if rst_n_i = '0' then
        pulse1 <= '0';
        pulse2 <= '0';
      else
        pulse1 <= Ext_Wr_active;
        pulse2 <= pulse1;
      end if;
    end if;
  end process;

  wrpulse <= pulse1 and not pulse2;

  

  wb_regs: process (clk_sys_i, rst_n_i)
  begin
    if rising_edge(clk_sys_i) then
    end if;  
  end process;

  
  wb_fsm: process (clk_sys_i)
  begin
    if rising_edge(clk_sys_i) then
      dtack          <= '0';
      rd_active      <= '0';
      master_o.stb   <= '0';
      user_rd_active <= '0';
      
      case wb_state is
        when idle =>
          if Ext_Adr_Val = '1' then
            if unsigned(Adr_from_SCUB_LA) = ( Base_addr + c_wb_adr_h ) then
              if wrpulse = '1' then
                wb_adr(31 downto 16) <= Data_from_SCUB_LA;
                dtack        <= '1';
              end if;

            elsif unsigned(Adr_from_SCUB_LA) = ( Base_addr + c_wb_adr_l ) then
              if wrpulse = '1' then
                wb_adr(15 downto 0) <= Data_from_SCUB_LA;
                dtack  <= '1';
              end if;

            elsif unsigned(Adr_from_SCUB_LA) = ( Base_addr + c_wb_dat_h ) then
              if wrpulse = '1' then
                wb_dat(31 downto 16) <= Data_from_SCUB_LA;
                dtack  <= '1';
              end if;
              if Ext_Rd_active = '1' then
                Data_to_SCUB   <= wb_dat(31 downto 16);
                user_rd_active <= '1';
                dtack          <= '1';
              end if;
              
            elsif unsigned(Adr_from_SCUB_LA) = ( Base_addr + c_wb_dat_l ) then
              if wrpulse = '1' then
                wb_dat(15 downto 0) <= Data_from_SCUB_LA;
                dtack  <= '1';
              end if;
              if Ext_Rd_active = '1' then
                Data_to_SCUB   <= wb_dat(15 downto 0);
                user_rd_active <= '1';
                dtack          <= '1';
              end if;

            elsif unsigned(Adr_from_SCUB_LA) = ( Base_addr + c_wb_rdwrsel ) then
              if wrpulse = '1' then
                wb_rdwrsel(15 downto 0) <= Data_from_SCUB_LA;
                dtack  <= '1';
              end if;
            end if;
          end if;

          if wb_rdwrsel(1) = '1' then
            wb_state     <= write;
            master_o.cyc <= '1';
            wb_rdwrsel(1) <= '0';
          elsif wb_rdwrsel(0) = '1' then
            wb_state <= read;
            master_o.cyc <= '1';
            wb_rdwrsel(0) <= '0';
          end if;

        when write =>
          master_o.dat <= wb_dat;
          master_o.sel <= wb_rdwrsel(5 downto 2);
          master_o.we  <= '1';
          master_o.stb <= '1';
          if master_i.stall = '0' then
            wb_state <= idle;
          else
            wb_state <= write_stalled;
          end if;

        when write_stalled =>
          master_o.stb <= '1';
          if master_i.stall = '0' then
            master_o.stb <= '0';
            wb_state <= ack_err;
          end if;

        when read =>
          master_o.sel <= wb_rdwrsel(5 downto 2);
          master_o.we <= '0';
          master_o.stb <= '1';
          if master_i.stall = '0' then
            wb_state <= idle;
          else
            wb_state <= read_stalled;
          end if;

        when read_stalled =>
          master_o.stb <= '1';
          if master_i.stall = '0' then
            wb_state <= ack_err;
          end if;

        when ack_err =>
          if master_i.ack = '1' then
            master_o.cyc <= '0';
            if master_o.we = '0' then
              wb_dat <= master_i.dat;
            end if;
            wb_state <= idle;
          elsif master_i.err = '1' then
            wb_state <= idle;
            master_o.cyc <= '0';
          end if;

        when others =>
        end case;
      
    end if;
  end process wb_fsm;

  master_o.adr <= wb_adr;
  
  rd_delay: process(clk_sys_i, rd_active, dtack)
  begin
    if rising_edge(clk_sys_i) then
      rd_active_dly <= rd_active;
      dtack_dly <= dtack;
    end if;
  end process;
  
  
  --user_rd_active <= rd_active_dly;
  Dtack_to_SCUB <= dtack_dly;

end architecture;
