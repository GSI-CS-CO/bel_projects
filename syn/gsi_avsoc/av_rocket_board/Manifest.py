target = "altera"
action = "synthesis"

fetchto = "../../../ip_cores"

syn_device = "5ASTFD5K3F40I3"
syn_grade = "c3"
syn_package = "31"
syn_top = "av_rocket_board"
syn_project = "av_rocket_board"

quartus_preflow = "av_rocket_board.tcl"

modules = {
  "local" : [ 
    "../../../top/gsi_avsoc/av_rocket_board", 
  ]
}
syn_tool = "quartus"
