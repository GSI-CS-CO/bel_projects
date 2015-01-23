target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"

syn_device = "5agxma3d4f"
syn_grade = "c5"
syn_package = "27"
syn_top = "microtca_control"
syn_project = "microtca_control"

quartus_preflow = "microtca_control.tcl"

modules = {
  "local" : [ 
    "../../../top/gsi_microtca/control", 
  ]
}
