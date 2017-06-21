fetchto = "ip_cores"

modules = {
  "local" : [
    "modules"
  ],
  "git" : [
    "git://ohwr.org/hdl-core-lib/general-cores.git",
    "git://ohwr.org/hdl-core-lib/etherbone-core.git",
    "git://ohwr.org/hdl-core-lib/pci-core.git",
    "git://ohwr.org/hdl-core-lib/wr-cores.git"
  ],
  "gitsm" : [
    "git://ohwr.org/hdl-core-lib/fpga-config-space.git"
  ]
}
