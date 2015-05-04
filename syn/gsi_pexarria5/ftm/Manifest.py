target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"
syn_tool = "quartus"
syn_device = "5agxma3d4f"
syn_grade = "c5"
syn_package = "27"
syn_top = "ftm"
syn_project = "ftm"

quartus_preflow = "ftm.tcl"

modules = {
  "local" : [ 
    "../../../top/gsi_pexarria5/ftm", 
  ]
}
syn_tool = "quartus"
