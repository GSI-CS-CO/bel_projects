target = "altera"
action = "synthesis"

fetchto = "../../ip_cores"

syn_device = "ep2agx125df"
syn_grade = "c5"
syn_package = "25"
syn_top = "scu_addac"
syn_project = "scu_addac2"

quartus_preflow = "scu_addac2.tcl"

modules = {
  "local" : [ 
    "../../top/gsi_addac2/", 
  ]
}
syn_tool = "quartus"
