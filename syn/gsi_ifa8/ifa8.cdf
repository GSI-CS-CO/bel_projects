/* Quartus II 64-Bit Version 13.0.0 Build 156 04/24/2013 SJ Full Version */
JedecChain;
	FileRevision(JESD32A);
	DefaultMfr(6E);

	P ActionCode(Cfg)
		Device PartName(EP1C20F400) Path("/home/stefan/bel_projects/syn/gsi_ifa8/") File("ifa8.sof") MfrSpec(OpMask(1));
	P ActionCode(Ign)
		Device PartName(EPM3128A) MfrSpec(OpMask(0));

ChainEnd;

AlteraBegin;
	ChainType(JTAG);
AlteraEnd;
