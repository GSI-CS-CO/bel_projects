library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

use work.wishbone_pkg.all;

package interface_lm32 is
    --VL_IN(__SYM__interrupt,31,0);

  function to_integer(logic_value : std_logic) return integer;
  function to_std_logic(integer_value: integer) return std_logic;

  function interface_lm32_init return integer;
  attribute foreign of interface_lm32_init : function is "VHPIDIRECT interface_lm32_init";



  procedure interface_lm32_clock(idx : integer; clk : std_logic);
  procedure interface_lm32_reset(idx : integer; rst : std_logic);

    --VL_IN8(clk_i,0,0);
  procedure interface_lm32_clk(idx : integer; clk : integer);
  attribute foreign of interface_lm32_clk : procedure is "VHPIDIRECT interface_lm32_clk";
    --VL_IN8(rst_i,0,0);
  procedure interface_lm32_rst(idx : integer; rst : integer);
  attribute foreign of interface_lm32_rst : procedure is "VHPIDIRECT interface_lm32_rst";




  procedure interface_lm32_sym_interrupt(idx : integer; interrupt : integer);
  attribute foreign of interface_lm32_sym_interrupt : procedure is "VHPIDIRECT interface_lm32_sym_interrupt";
  procedure interface_lm32_interrupt(idx: integer; irq : std_logic_vector(31 downto 0));



  function  interface_lm32_i_wb_mosi(idx: integer) return t_wishbone_master_out;
  procedure interface_lm32_i_wb_miso(idx: integer; wb_miso : t_wishbone_master_in);
  function  interface_lm32_d_wb_mosi(idx: integer) return t_wishbone_master_out;
  procedure interface_lm32_d_wb_miso(idx: integer; wb_miso : t_wishbone_master_in);

    --VL_OUT(I_ADR_O,31,0);
  function interface_lm32_i_adr_o(idx : integer) return integer;
  attribute foreign of interface_lm32_i_adr_o : function is "VHPIDIRECT interface_lm32_i_adr_o";
    --VL_OUT8(I_CYC_O,0,0);
  function interface_lm32_i_cyc_o(idx : integer) return integer;
  attribute foreign of interface_lm32_i_cyc_o : function is "VHPIDIRECT interface_lm32_i_cyc_o";
    --VL_OUT8(I_STB_O,0,0);
  function interface_lm32_i_stb_o(idx : integer) return integer;
  attribute foreign of interface_lm32_i_stb_o : function is "VHPIDIRECT interface_lm32_i_stb_o";
    --VL_OUT(I_DAT_O,31,0);
  function interface_lm32_i_dat_o(idx : integer) return integer;
  attribute foreign of interface_lm32_i_dat_o : function is "VHPIDIRECT interface_lm32_i_dat_o";
    --VL_OUT8(I_SEL_O,3,0);
  function interface_lm32_i_sel_o(idx : integer) return integer;
  attribute foreign of interface_lm32_i_sel_o : function is "VHPIDIRECT interface_lm32_i_sel_o";
    --VL_OUT8(I_WE_O,0,0);
  function interface_lm32_i_we_o(idx : integer) return integer;
  attribute foreign of interface_lm32_i_we_o : function is "VHPIDIRECT interface_lm32_i_we_o";
    --VL_OUT8(I_CTI_O,2,0);
  function interface_lm32_i_cti_o(idx : integer) return integer;
  attribute foreign of interface_lm32_i_cti_o : function is "VHPIDIRECT interface_lm32_i_cti_o";
    --VL_OUT8(I_LOCK_O,0,0);
  function interface_lm32_i_lock_o(idx : integer) return integer;
  attribute foreign of interface_lm32_i_lock_o : function is "VHPIDIRECT interface_lm32_i_lock_o";
    --VL_OUT8(I_BTE_O,1,0);
  function interface_lm32_i_bte_o(idx : integer) return integer;
  attribute foreign of interface_lm32_i_bte_o : function is "VHPIDIRECT interface_lm32_i_bte_o";
    --VL_IN8(I_ACK_I,0,0);
  procedure interface_lm32_i_ack_i(idx : integer; ack : integer);
  attribute foreign of interface_lm32_i_ack_i : procedure is "VHPIDIRECT interface_lm32_i_ack_i";
    --VL_IN(I_DAT_I,31,0);
  procedure interface_lm32_i_dat_i(idx : integer; dat : integer);
  attribute foreign of interface_lm32_i_dat_i : procedure is "VHPIDIRECT interface_lm32_i_dat_i";
    --VL_IN8(I_ERR_I,0,0);
  procedure interface_lm32_i_err_i(idx : integer; err : integer);
  attribute foreign of interface_lm32_i_err_i : procedure is "VHPIDIRECT interface_lm32_i_err_i";
    --VL_IN8(I_RTY_I,0,0);
  procedure interface_lm32_i_rty_i(idx : integer; rty : integer);
  attribute foreign of interface_lm32_i_rty_i : procedure is "VHPIDIRECT interface_lm32_i_rty_i";


    --VL_OUT(D_ADR_O,31,0);
  function interface_lm32_d_adr_o(idx : integer) return integer;
  attribute foreign of interface_lm32_d_adr_o : function is "VHPIDIRECT interface_lm32_d_adr_o";
    --VL_OUT8(D_CYC_O,0,0);
  function interface_lm32_d_cyc_o(idx : integer) return integer;
  attribute foreign of interface_lm32_d_cyc_o : function is "VHPIDIRECT interface_lm32_d_cyc_o";
    --VL_OUT8(D_STB_O,0,0);
  function interface_lm32_d_stb_o(idx : integer) return integer;
  attribute foreign of interface_lm32_d_stb_o : function is "VHPIDIRECT interface_lm32_d_stb_o";
    --VL_OUT(D_DAT_O,31,0);
  function interface_lm32_d_dat_o(idx : integer) return integer;
  attribute foreign of interface_lm32_d_dat_o : function is "VHPIDIRECT interface_lm32_d_dat_o";
    --VL_OUT8(D_SEL_O,3,0);
  function interface_lm32_d_sel_o(idx : integer) return integer;
  attribute foreign of interface_lm32_d_sel_o : function is "VHPIDIRECT interface_lm32_d_sel_o";
    --VL_OUT8(D_WE_O,0,0);
  function interface_lm32_d_we_o(idx : integer) return integer;
  attribute foreign of interface_lm32_d_we_o : function is "VHPIDIRECT interface_lm32_d_we_o";
    --VL_OUT8(D_CTI_O,2,0);
  function interface_lm32_d_cti_o(idx : integer) return integer;
  attribute foreign of interface_lm32_d_cti_o : function is "VHPIDIRECT interface_lm32_d_cti_o";
    --VL_OUT8(D_LOCK_O,0,0);
  function interface_lm32_d_lock_o(idx : integer) return integer;
  attribute foreign of interface_lm32_d_lock_o : function is "VHPIDIRECT interface_lm32_d_lock_o";
    --VL_OUT8(D_BTE_O,1,0);
  function interface_lm32_d_bte_o(idx : integer) return integer;
  attribute foreign of interface_lm32_d_bte_o : function is "VHPIDIRECT interface_lm32_d_bte_o";
    --VL_IN8(D_ACK_I,0,0);
  procedure interface_lm32_d_ack_i(idx : integer; ack : integer);
  attribute foreign of interface_lm32_d_ack_i : procedure is "VHPIDIRECT interface_lm32_d_ack_i";
    --VL_IN(D_DAT_I,31,0);
  procedure interface_lm32_d_dat_i(idx : integer; dat : integer);
  attribute foreign of interface_lm32_d_dat_i : procedure is "VHPIDIRECT interface_lm32_d_dat_i";
    --VL_IN8(D_ERR_I,0,0);
  procedure interface_lm32_d_err_i(idx : integer; err : integer);
  attribute foreign of interface_lm32_d_err_i : procedure is "VHPIDIRECT interface_lm32_d_err_i";
    --VL_IN8(D_RTY_I,0,0);
  procedure interface_lm32_d_rty_i(idx : integer; rty : integer);
  attribute foreign of interface_lm32_d_rty_i : procedure is "VHPIDIRECT interface_lm32_d_rty_i";


  shared variable my_var : integer := 43;
end package;

package body interface_lm32 is
  function to_integer(logic_value: std_logic) return integer is
  begin
    if logic_value = '1' then 
      return 1;
    else 
      return 0;
    end if;
  end function;

  function to_std_logic(integer_value: integer) return std_logic is
  begin
    if integer_value = 0 then 
      return '0';
    else 
      return '1';
    end if;
  end function;


  function interface_lm32_init return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;


  procedure interface_lm32_clock(idx : integer; clk : std_logic) is
  begin
    interface_lm32_clk(idx, to_integer(clk));
  end procedure;
  procedure interface_lm32_reset(idx : integer; rst : std_logic) is
  begin
    interface_lm32_rst(idx, to_integer(rst));
  end procedure;

  procedure interface_lm32_clk(idx : integer; clk : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;
  procedure interface_lm32_rst(idx : integer; rst : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;



  procedure interface_lm32_sym_interrupt(idx : integer; interrupt : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;


  -- instruction access wishbone wrapper
  function interface_lm32_i_wb_mosi(idx: integer) return t_wishbone_master_out is
    variable wb_mosi : t_wishbone_master_out;
  begin
    wb_mosi.adr := std_logic_vector(to_signed(interface_lm32_i_adr_o(idx),32));
    wb_mosi.dat := std_logic_vector(to_signed(interface_lm32_i_dat_o(idx),32));
    wb_mosi.sel := std_logic_vector(to_signed(interface_lm32_i_sel_o(idx),4));
    wb_mosi.cyc :=                 to_std_logic(interface_lm32_i_stb_o(idx));
    wb_mosi.stb :=                 to_std_logic(interface_lm32_i_stb_o(idx));
    wb_mosi.we  :=                 to_std_logic(interface_lm32_i_we_o (idx));
    return wb_mosi;
  end function;
  procedure interface_lm32_i_wb_miso(idx: integer; wb_miso : t_wishbone_master_in) is
  begin
    interface_lm32_i_dat_i(idx, to_integer(signed(wb_miso.dat)));
    interface_lm32_i_ack_i(idx, to_integer(wb_miso.ack));
    interface_lm32_i_err_i(idx, to_integer(wb_miso.err));
    interface_lm32_i_rty_i(idx, to_integer(wb_miso.rty));
  end procedure;
  -- data access wishbone wrapper
  function interface_lm32_d_wb_mosi(idx: integer) return t_wishbone_master_out is
    variable wb_mosi : t_wishbone_master_out;
  begin
    wb_mosi.adr := std_logic_vector(to_signed(interface_lm32_d_adr_o(idx),32));
    wb_mosi.dat := std_logic_vector(to_signed(interface_lm32_d_dat_o(idx),32));
    wb_mosi.sel := std_logic_vector(to_signed(interface_lm32_d_sel_o(idx),4));
    wb_mosi.cyc :=                 to_std_logic(interface_lm32_d_stb_o(idx));
    wb_mosi.stb :=                 to_std_logic(interface_lm32_d_stb_o(idx));
    wb_mosi.we  :=                 to_std_logic(interface_lm32_d_we_o (idx));
    return wb_mosi;
  end function;
  procedure interface_lm32_d_wb_miso(idx: integer; wb_miso : t_wishbone_master_in) is
  begin
    interface_lm32_d_dat_i(idx, to_integer(signed(wb_miso.dat)));
    interface_lm32_d_ack_i(idx, to_integer(wb_miso.ack));
    interface_lm32_d_err_i(idx, to_integer(wb_miso.err));
    interface_lm32_d_rty_i(idx, to_integer(wb_miso.rty));
  end procedure;
  procedure interface_lm32_interrupt(idx: integer; irq : std_logic_vector(31 downto 0)) is 
  begin
    interface_lm32_sym_interrupt(idx, to_integer(signed(irq)));
  end procedure;



  function interface_lm32_i_adr_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_i_cyc_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_i_stb_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_i_dat_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_i_sel_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_i_we_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_i_cti_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_i_lock_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_i_bte_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  procedure interface_lm32_i_ack_i(idx : integer; ack : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

  procedure interface_lm32_i_dat_i(idx : integer; dat : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

  procedure interface_lm32_i_err_i(idx : integer; err : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

  procedure interface_lm32_i_rty_i(idx : integer; rty : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;





  function interface_lm32_d_adr_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_d_cyc_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_d_stb_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_d_dat_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_d_sel_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_d_we_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_d_cti_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_d_lock_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  function interface_lm32_d_bte_o(idx : integer) return integer is
  begin
    assert false report "VHPI" severity failure;
  end function;

  procedure interface_lm32_d_ack_i(idx : integer; ack : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

  procedure interface_lm32_d_dat_i(idx : integer; dat : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

  procedure interface_lm32_d_err_i(idx : integer; err : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

  procedure interface_lm32_d_rty_i(idx : integer; rty : integer) is
  begin
    assert false report "VHPI" severity failure;
  end procedure;

end package body;




library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;  

use work.wishbone_pkg.all;
use work.interface_lm32.all;

entity lm32_top is
port(
  clk_i     : in std_logic;
  rst_i     : in std_logic;
  interrupt : in std_logic_vector(31 downto 0);
  I_DAT_I   : in std_logic_vector(31 downto 0);
  I_ACK_I   : in std_logic;
  I_ERR_I   : in std_logic;
  I_RTY_I   : in std_logic;
  D_DAT_I   : in std_logic_vector(31 downto 0);
  D_ACK_I : in std_logic;
  D_ERR_I : in std_logic;
  D_RTY_I : in std_logic;
  I_DAT_O : out std_logic_vector(31 downto 0);
  I_ADR_O : out std_logic_vector(31 downto 0);
  I_CYC_O : out std_logic;
  I_SEL_O : out std_logic_vector(3 downto 0);
  I_STB_O : out std_logic;
  I_WE_O  : out std_logic;
  I_CTI_O : out std_logic_vector(2 downto 0);
  I_LOCK_O : out std_logic;
  I_BTE_O : out std_logic_vector(1 downto 0);
  D_DAT_O : out std_logic_vector(31 downto 0);
  D_ADR_O : out std_logic_vector(31 downto 0);
  D_CYC_O : out std_logic;
  D_SEL_O : out std_logic_vector(3 downto 0);
  D_STB_O : out std_logic;
  D_WE_O  : out std_logic;
  D_CTI_O : out std_logic_vector(2 downto 0);
  D_LOCK_O : out std_logic;
  D_BTE_O : out std_logic_vector(1 downto 0)
);
end entity;

architecture simulation of lm32_top is
  signal lm32_idx         : integer   := interface_lm32_init;

  signal iwb_o : t_wishbone_master_out;
  signal iwb_i : t_wishbone_master_in;
  signal dwb_o : t_wishbone_master_out;
  signal dwb_i : t_wishbone_master_in;
  signal irq_i : std_logic_vector(31 downto 0);

begin

  irq_i <= interrupt;

  iwb_i.dat <= I_DAT_I;
  iwb_i.ack <= I_ACK_I;
  iwb_i.err <= I_ERR_I;
  iwb_i.rty <= I_RTY_I;

  dwb_i.dat <= D_DAT_I;
  dwb_i.ack <= D_ACK_I;
  dwb_i.err <= D_ERR_I;
  dwb_i.rty <= D_RTY_I;

  I_DAT_O  <= iwb_o.dat;
  I_ADR_O  <= iwb_o.adr;
  I_CYC_O  <= iwb_o.cyc;
  I_SEL_O  <= iwb_o.sel;
  I_STB_O  <= iwb_o.stb;
  I_WE_O   <= iwb_o.we;
  
  D_DAT_O  <= dwb_o.dat;
  D_ADR_O  <= dwb_o.adr;
  D_CYC_O  <= dwb_o.cyc;
  D_SEL_O  <= dwb_o.sel;
  D_STB_O  <= dwb_o.stb;
  D_WE_O   <= dwb_o.we;

  main: process
    variable adr : integer := 131072;
  begin
    --if rst_i = '1' then 
    --  wait until falling_edge(rst_i);
    --end if;
    --wait until rising_edge(clk_i);
    --dwb_o.cyc <= '1';
    --dwb_o.stb <= '1';
    --dwb_o.adr <= std_logic_vector(to_unsigned(adr,32));
    --dwb_o.we  <= '0';
    --dwb_o.sel <= "1111";
    --wait until rising_edge(dwb_i.ack) or rising_edge(dwb_i.err) or rising_edge(dwb_i.rty);
    --dwb_o.cyc <= '0';
    --dwb_o.stb <= '0';
    --adr := adr+4;


    wait until rising_edge(clk_i);
    interface_lm32_reset(lm32_idx, rst_i);
    interface_lm32_clock(lm32_idx, clk_i);

    -- get the output from verilator simulation
    iwb_o    <= interface_lm32_i_wb_mosi(lm32_idx); -- instruction bus
    dwb_o    <= interface_lm32_d_wb_mosi(lm32_idx); -- data bus

    -- for completeness also handle cti,lock,bte 
    --  but it is not used in the current configuration
    I_CTI_O  <= std_logic_vector(to_signed(interface_lm32_i_cti_o  (lm32_idx),3)); 
    I_LOCK_O <= to_std_logic(interface_lm32_i_lock_o (lm32_idx));                 
    I_BTE_O  <= std_logic_vector(to_signed(interface_lm32_i_bte_o  (lm32_idx),2));
    D_CTI_O  <= std_logic_vector(to_signed(interface_lm32_d_cti_o  (lm32_idx),3)); 
    D_LOCK_O <= to_std_logic(interface_lm32_d_lock_o (lm32_idx));
    D_BTE_O  <= std_logic_vector(to_signed(interface_lm32_d_bte_o  (lm32_idx),2));

    wait until falling_edge(clk_i);
    interface_lm32_i_wb_miso(lm32_idx, iwb_i); -- instruction bus
    interface_lm32_d_wb_miso(lm32_idx, dwb_i); -- data bus

    interface_lm32_interrupt(lm32_idx, irq_i); -- interrupts

    interface_lm32_clock(lm32_idx, clk_i);
  end process;

end architecture;