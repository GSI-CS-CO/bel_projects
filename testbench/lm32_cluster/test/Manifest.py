target = "altera"
action = "synthesis"

fetchto = "../../ip_cores"
syn_tool = "quartus"
syn_device = "5agxma3d4f"
syn_grade = "i3"
syn_package = "27"
syn_top = "testbench"
syn_project = "testbench"

quartus_preflow = "pci_control.tcl"

files = [
   "testbench.vhd",
]

modules = {
  "local" : [
    "../../../modules/monster",
    "../../../ip_cores/etherbone-core/hdl/eb_usb_core",
    "../../../ip_cores/general-cores/modules/common",
    "../../../modules/prioq2",
    "../../../ip_cores/etherbone-core/hdl/eb_slave_core",
    "../../../ip_cores/wr-cores/modules/fabric",
    "../../../modules/mbox",
    "../../../modules/aux_functions",
    "../../../ip_cores/general-cores/modules/genrams/generic",
    "../../../ip_cores/general-cores/modules/wishbone",
    "../../../ip_cores/general-cores/modules/wishbone/wb_crossbar",
    "../../../ip_cores/general-cores/modules/genrams",
    "../../../modules/wb_timer",
    "../../../modules/wb_arria_reset/",
    "../../../ip_cores/general-cores/modules/wishbone/wb_irq",
    "../../../ip_cores/general-cores/modules/wishbone/wb_register",
    "../../../modules/dm_diag",
    "../../../modules/ftm",
    "./eb_sim_core"
  ]
}

