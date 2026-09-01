set project  [lindex $quartus(args) 1]
set revision [lindex $quartus(args) 2]

project_open $project -revision $revision

proc qmegawiz {files} {
  set dir [file dirname [info script]]
  post_message "Testing for megawizard regeneration in $dir:$files"

  set device  [ get_global_assignment -name DEVICE ]
  set family  [ get_global_assignment -name FAMILY ]

  # Opt-in for container/headless runs (Jenkins uses Xvnc and must keep the default path).
  # Example: make vetar2a QMEGAWIZ_HEADLESS=1
  set headless 0
  if {[info exists ::env(QMEGAWIZ_HEADLESS)] && $::env(QMEGAWIZ_HEADLESS) ne "" && $::env(QMEGAWIZ_HEADLESS) ne "0"} {
    set headless 1
  }

  foreach i $files {
    set need_regen [expr {![file exists "$dir/$i.qip"] || [file mtime "$dir/$i.txt"] > [file mtime "$dir/$i.qip"]}]
    if {$headless && [file exists "$dir/$i.qip"] && [file size "$dir/$i.qip"] == 0} {
      set need_regen 1
    }

    if {$need_regen} {
      post_message -type info "Regenerating $i using qmegawiz"
      file delete "$dir/$i.qip"
      file copy -force "$dir/$i.txt" "$dir/$i.vhd"

      set qmegawiz_bin "qmegawiz"
      if {$headless && [auto_execok xvfb-run] ne ""} {
        set qmegawiz_bin "xvfb-run -a qmegawiz"
      }

      set sf [open "| $qmegawiz_bin -silent \"-defaultfamily:$family\" \"-defaultdevice:$device\" \"$dir/$i.vhd\" 2>@file1" "r"]
      while {[gets $sf line] >= 0} { post_message -type info "$line" }
      if {[catch {close $sf} err]} {
        post_message -type error "Executing qmegawiz: $err"
        exit 1
      }
      if {![file exists "$dir/$i.qip"] || ($headless && [file size "$dir/$i.qip"] == 0)} {
        post_message -type error "Executing qmegawiz: did not create $dir/$i.qip!"
        exit 1
      }

      file mtime "$dir/$i.qip" [file mtime "$dir/$i.vhd"]
    }
  set_global_assignment -name QIP_FILE "$dir/$i.qip"
  }
}

proc qsys-generate {files} {
  set dir [file dirname [info script]]
  foreach i $files {
    set sf [open "| qsys-generate -syn \"$dir/$i/$i.qsys\" 2>@file1" "r"]
    while {[gets $sf line] >= 0} { post_message -type info "$line" }
    if {[catch {close $sf} err]} {
      post_message -type error "Executing qsys-generate: $err"
      exit 1
    }
    set_global_assignment -name QSYS_FILE "$dir/$i/$i.qsys"
  }
}
