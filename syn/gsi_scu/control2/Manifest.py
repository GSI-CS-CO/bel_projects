target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"

syn_device = "ep2agx125ef"
syn_grade = "c5"
syn_package = "29"
syn_top = "scu_control"
syn_project = "scu_control"

quartus_preflow = "scu_control.tcl"

modules = {
  "local" : [ 
    "../../../top/gsi_scu/control2", 
  ]
}
syn_tool = "quartus"
