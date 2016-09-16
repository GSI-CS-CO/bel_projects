target = "altera"
action = "synthesis"

fetchto = "../../ip_cores"

syn_device = "ep2agx125ef"
syn_grade = "c5"
syn_package = "25"
syn_top = "scu_diob_v2"
syn_project = "scu_diob_v2"

quartus_preflow = "scu_diob.tcl"

modules = {
  "local" : [ 
    "../../top/scu_diob_v2/", 
  ]
}
syn_tool = "quartus"
