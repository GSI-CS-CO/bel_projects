target = "altera"
action = "synthesis"

fetchto = "../../ip_cores"

syn_device = "ep2agx125df"
syn_grade = "c5"
syn_package = "25"
syn_top = "blm_aco"
syn_project = "blm_aco"

quartus_preflow = "blm_aco.tcl"

modules = {
  "local" : [ 
    "../../top/blm_aco/", 
  ]
}
syn_tool = "quartus"
