files = [
   "scu_addac.vhd",
   "scu_addac.sdc",
   "IO_4x8.vhd",
   "flash_loader_v01.vhd",
   "adda_pll.vhd",
   "addac_local_clk_to_12p5_mhz.vhd",
   "addac_sys_clk_local_clk_switch_pkg.vhd",
   "addac_sys_clk_local_clk_switch.vhd",
]

modules = {
  "local" : [
    "../../modules",
    "../../ip_cores",
  ]
}
