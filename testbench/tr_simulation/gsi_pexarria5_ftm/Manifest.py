target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"
syn_tool = "quartus"
syn_device = "5agxma3d4f"
syn_grade = "i3"
syn_package = "27"
syn_top = "ftm"
syn_project = "ftm"

quartus_preflow = "ftm.tcl"

files = [
   "testbench.vhd",
]

modules = {
  "local" : [
    "../../../top/gsi_pexarria5/ftm",
  ]
}

