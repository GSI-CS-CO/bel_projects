-- Copyright (C) 1991-2004 Altera Corporation
-- Any  megafunction  design,  and related netlist (encrypted  or  decrypted),
-- support information,  device programming or simulation file,  and any other
-- associated  documentation or information  provided by  Altera  or a partner
-- under  Altera's   Megafunction   Partnership   Program  may  be  used  only
-- to program  PLD  devices (but not masked  PLD  devices) from  Altera.   Any
-- other  use  of such  megafunction  design,  netlist,  support  information,
-- device programming or simulation file,  or any other  related documentation
-- or information  is prohibited  for  any  other purpose,  including, but not
-- limited to  modification,  reverse engineering,  de-compiling, or use  with
-- any other  silicon devices,  unless such use is  explicitly  licensed under
-- a separate agreement with  Altera  or a megafunction partner.  Title to the
-- intellectual property,  including patents,  copyrights,  trademarks,  trade
-- secrets,  or maskworks,  embodied in any such megafunction design, netlist,
-- support  information,  device programming or simulation file,  or any other
-- related documentation or information provided by  Altera  or a megafunction
-- partner, remains with Altera, the megafunction partner, or their respective
-- licensors. No other licenses, including any licenses needed under any third
-- party's intellectual property, are provided herein.

-- PROGRAM "Quartus II"
-- VERSION "Version 4.1 Build 208 09/10/2004 Service Pack 2 SJ Full Version"


-- Vers 1.1	19.02.2008 W.P. Generic "Loader_CLK_in_Hz" eingefügt, wird nach
--							"b2v_imodulif : modul2spi" weitergereicht, damit
--							dort die SPI-Clk immer richtig generiert wird. 
LIBRARY ieee;
USE ieee.std_logic_1164.all; 

LIBRARY work;

ENTITY K_EPCS_IF IS
	generic(Loader_CLK_in_Hz: INTEGER);
	port
	(
		clk :  IN  STD_LOGIC;
		clrn :  IN  STD_LOGIC;
		we :  IN  STD_LOGIC;
		Adr :  IN  STD_LOGIC;
		rd :  IN  STD_LOGIC;
		CTRL_LOAD :  IN  STD_LOGIC;
		CTRL_RES :  IN  STD_LOGIC;
		D :  IN  STD_LOGIC_VECTOR(15 downto 0);
		LOADER_DB :  INOUT  STD_LOGIC_VECTOR(3 downto 0);
		LOADER_WRnRD :  OUT  STD_LOGIC;
		RELOAD :  OUT  STD_LOGIC;
		LOAD_OK :  OUT  STD_LOGIC;
		LOAD_ERROR :  OUT  STD_LOGIC;
		INIT_DONE :  OUT  STD_LOGIC;
		Q :  OUT  STD_LOGIC_VECTOR(15 downto 0)
	);
END K_EPCS_IF;

ARCHITECTURE bdf_type OF K_EPCS_IF IS 

component modul2spi
	GENERIC(Loader_CLK_in_Hz: INTEGER);
	PORT(clk : IN STD_LOGIC;
		 clrn : IN STD_LOGIC;
		 we : IN STD_LOGIC;
		 rd : IN STD_LOGIC;
		 Adr : IN STD_LOGIC;
		 inkr : IN STD_LOGIC;
		 store : IN STD_LOGIC;
		 fertig : IN STD_LOGIC;
		 CTRL_LOAD : IN STD_LOGIC;
		 CTRL_RES : IN STD_LOGIC;
		 D : IN STD_LOGIC_VECTOR(15 downto 0);
		 SQ : IN STD_LOGIC_VECTOR(7 downto 0);
		 req : OUT STD_LOGIC;
		 ena : OUT STD_LOGIC;
		 cont : OUT STD_LOGIC;
		 RELOAD : OUT STD_LOGIC;
		 INIT_DONE : OUT STD_LOGIC;
		 LOAD_OK : OUT STD_LOGIC;
		 LOAD_ERROR : OUT STD_LOGIC;
		 cmd : OUT STD_LOGIC_VECTOR(7 downto 0);
		 Q : OUT STD_LOGIC_VECTOR(15 downto 0);
		 SA : OUT STD_LOGIC_VECTOR(23 downto 0);
		 SD : OUT STD_LOGIC_VECTOR(7 downto 0)
	);
end component;

component epcs_spi
	PORT(clk : IN STD_LOGIC;
		 clrn : IN STD_LOGIC;
		 ena : IN STD_LOGIC;
		 req : IN STD_LOGIC;
		 cont : IN STD_LOGIC;
		 cmd : IN STD_LOGIC_VECTOR(7 downto 0);
		 LOADER_DB : INOUT STD_LOGIC_VECTOR(3 downto 0);
		 SA : IN STD_LOGIC_VECTOR(23 downto 0);
		 SD : IN STD_LOGIC_VECTOR(7 downto 0);
		 LOADER_WRnRD : OUT STD_LOGIC;
		 store : OUT STD_LOGIC;
		 inkr : OUT STD_LOGIC;
		 fertig : OUT STD_LOGIC;
		 SQ : OUT STD_LOGIC_VECTOR(7 downto 0)
	);
end component;

signal	inkr :  STD_LOGIC;
signal	store :  STD_LOGIC;
signal	fertig :  STD_LOGIC;
signal	sq :  STD_LOGIC_VECTOR(7 downto 0);
signal	ena :  STD_LOGIC;
signal	req :  STD_LOGIC;
signal	cont :  STD_LOGIC;
signal	cmd :  STD_LOGIC_VECTOR(7 downto 0);
signal	sa :  STD_LOGIC_VECTOR(23 downto 0);
signal	sd :  STD_LOGIC_VECTOR(7 downto 0);


BEGIN 



b2v_imodulif : modul2spi
GENERIC MAP(Loader_CLK_in_Hz => Loader_CLK_in_Hz)
PORT MAP(clk => clk,
		 clrn => clrn,
		 we => we,
		 rd => rd,
		 Adr => Adr,
		 inkr => inkr,
		 store => store,
		 fertig => fertig,
		 CTRL_LOAD => CTRL_LOAD,
		 CTRL_RES => CTRL_RES,
		 D => D,
		 SQ => sq,
		 req => req,
		 ena => ena,
		 cont => cont,
		 RELOAD => RELOAD,
		 INIT_DONE => INIT_DONE,
		 LOAD_OK => LOAD_OK,
		 LOAD_ERROR => LOAD_ERROR,
		 cmd => cmd,
		 Q => Q,
		 SA => sa,
		 SD => sd);

b2v_ispi : epcs_spi
PORT MAP(clk => clk,
		 clrn => clrn,
		 ena => ena,
		 req => req,
		 cont => cont,
		 cmd => cmd,
		 LOADER_DB => LOADER_DB,
		 SA => sa,
		 SD => sd,
		 LOADER_WRnRD => LOADER_WRnRD,
		 store => store,
		 inkr => inkr,
		 fertig => fertig,
		 SQ => sq);

END; 