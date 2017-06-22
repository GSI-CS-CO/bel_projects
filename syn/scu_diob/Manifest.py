target = "altera"
action = "synthesis"

fetchto = "../../ip_cores"

syn_device = "ep2agx125df"
syn_grade = "c5"
syn_package = "25"
syn_top = "scu_diob"
syn_project = "scu_diob"

quartus_preflow = "scu_diob.tcl"

modules = {
  "local" : [ 
    "../../top/scu_diob/", 
  ]
}
syn_tool = "quartus"
