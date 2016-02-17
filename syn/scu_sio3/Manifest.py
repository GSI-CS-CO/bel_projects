target = "altera"
action = "synthesis"

fetchto = "../../ip_cores"

syn_device = "ep2agx125ef"
syn_grade = "c5"
syn_package = "29"
syn_top = "scu_sio3"
syn_project = "scu_sio3"

quartus_preflow = "scu_sio3.tcl"

modules = {
  "local" : [ 
    "../../top/scu_sio3/" 
  ]
}
