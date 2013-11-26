set project  [lindex $quartus(args) 1]
set revision [lindex $quartus(args) 2]

project_open $project -revision $revision

proc qmegawiz {files} {
  set dir [file dirname [info script]]
  post_message "Testing for megawizard regeneration in $dir:$files"

  set device  [ get_global_assignment -name DEVICE ]
  set family  [ get_global_assignment -name FAMILY ]

  foreach i $files {
    if {![file exists "$dir/$i.qip"] || [file mtime "$dir/$i.txt"] > [file mtime "$dir/$i.qip"]} {
      post_message -type info "Regenerating $i using qmegawiz"
      file delete "$dir/$i.qip"
      file copy -force "$dir/$i.txt" "$dir/$i.vhd"
      
      set sf [open "| qmegawiz -silent \"-defaultfamily:$family\" \"-defaultdevice:$device\" \"$dir/$i.vhd\" 2>@file1" "r"]
      while {[gets $sf line] >= 0} { post_message -type info "$line" }
      if {[catch {close $sf} err]} {
        post_message -type error "Executing qmegawiz: $err"
        exit 1
      }
      if {![file exists "$dir/$i.qip"]} {
        post_message -type error "Executing qmegawiz: did not create $dir/$i.qip!"
        exit 1
      }
      
      file mtime "$dir/$i.qip" [file mtime "$dir/$i.vhd"]
    }
  }
}
