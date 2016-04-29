target = "altera"
action = "synthesis"

fetchto = "../../ip_cores"

syn_device = "EP1C20F400"
syn_grade = "C7"
syn_package = "0"
syn_top = "ifa8"
syn_project = "ifa8"


modules = {
  "local" : [ 
    "../../top/gsi_ifa8/", 
  ]
}
syn_tool = "quartus"
