target = "altera"
action = "synthesis"

fetchto = "../../ip_cores"

syn_device = "ep2agx125df"
syn_grade = "c5"
syn_package = "25"
syn_top = "scu_diob"
syn_project = "scu_diob2"

quartus_preflow = "scu_diob2.tcl"

modules = {
  "local" : [ 
    "../../top/scu_diob2/", 
  ]
}
syn_tool = "quartus"
