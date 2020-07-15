target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"

syn_device = "5agxma3d4f"
syn_grade = "i3"
syn_package = "27"
syn_top = "pexp_control"
syn_project = "pexp_control"

quartus_preflow = "pexp_control.tcl"

modules = {
  "local" : [ 
    "../../../top/gsi_pexp/control", 
  ]
}

syn_tool = "quartus"
