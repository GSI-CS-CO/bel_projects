/* Quartus II 32-bit Version 12.1 Build 177 11/07/2012 SJ Full Version */
JedecChain;
	FileRevision(JESD32A);
	DefaultMfr(6E);

	P ActionCode(Cfg)
		Device PartName(EP2AGX125DF25) Path("G:/bel_projects/syn/gsi_addac/") File("scu_addac.sof") MfrSpec(OpMask(1) SEC_Device(EPCS128) Child_OpMask(1 1) SFLPath("G:/bel_projects/syn/gsi_addac/scu_addac.jic"));
	P ActionCode(Ign)
		Device PartName(ISP_CLOCK_5410) MfrSpec(OpMask(0));

ChainEnd;

AlteraBegin;
	ChainType(JTAG);
AlteraEnd;
