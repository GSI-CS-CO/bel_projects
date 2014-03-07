files = [
   "scu_diob.vhd",
   "scu_diob.sdc",
   "flash_loader_v01.vhd",
   "Zeitbasis.vhd",
   "AW_IO_Reg.vhd",
   "diob_sys_clk_local_clk_switch_pkg.vhd",
   "diob_sys_clk_local_clk_switch.vhd",
   "diob_local_clk_to_12p5_mhz.vhd",  
]

modules = {
  "local" : [
    "../../modules",
    "../../ip_cores",
  ]
}
