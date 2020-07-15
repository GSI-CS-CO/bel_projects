#!/bin/bash
# EB SSH Wrapper Functions
# 2018 by Mathias Kreider, GSI
#
# Automatically translates eb-tool actions over TCP via socat into SSH commands to the host platform
# To override this and use original eb-tools behaviour for 'tcp/' devices,
# prefix the call with a backslash, eg '\eb-ls tcp/...'
#
# CAVEAT: While other syntaxes with preceding 'command' or absolute paths also work for tcp/SSH override,
# they will cause trouble when using autocompletion for dm-cmd. The reason is that autocomplete only reads
# the command word (dm-cmd) and onward, all preceding words are lost. We can thus only make the background call
# to dm-sched also use the override if we got told as part of the dm-cmd word, i.e. '\dm-cmd'
# Best to only ever use '\<eb-tool> tcp/<eb-device>' syntax when overriding.
#
#
# Install:
# this file belongs in your home directory and should be sourced by .bashrc
#
# cat append_eb_ssh_to_bashrc >> ~/.bashrc
# source ~/.bashrc
#
# You can check if the automation is installed by typing:
# alias
#
# As a real world test, deactivate socat on frontend, try: '\eb-ls tcp/<front-end>'. This should fail.
# Now call the aliased version (the usual call):            'eb-ls tcp/<front-end>'. This should work.

_add_eb_aliases()
{
  alias eb-ls='_eb_ssh_eb_ls'
  alias eb-info='_eb_ssh_eb_info'
  alias eb-read='_eb_ssh_eb_read'
  alias eb-write='_eb_ssh_eb_write'
  alias eb-get='_eb_ssh_eb_get'
  alias eb-put='_eb_ssh_eb_put'
  alias eb-mon='_eb_ssh_eb_mon'
  alias eb-fwload='_eb_ssh_eb_fwload'
  alias eb-find='_eb_ssh_eb_find'
  alias eb-reset='_eb_ssh_eb_reset'
  alias eb-console='_eb_ssh_eb_console'
  alias eb-flash='_eb_ssh_eb_flash'
  alias eb-test='_eb_ssh_test_me'
}

_add_dm_aliases()
{
  alias dm-cmd='_eb_ssh_dm_cmd'
  alias dm-sched='_eb_ssh_dm_sched'
}

_rm_eb_aliases()
{
  unalias eb-ls
  unalias eb-info
  unalias eb-read
  unalias eb-write
  unalias eb-get
  unalias eb-put
  unalias eb-mon
  unalias eb-fwload
  unalias eb-find
  unalias eb-reset
  unalias eb-console
  unalias eb-flash
  unalias eb-test
}

_rm_dm_aliases()
{
  unalias dm-cmd
  unalias dm-sched
}

#use _eb_ssh_ prefix as a sort of namespace to clashes with possible autocomplete functions

_eb_ssh_eb_func() {
  local args cmd pciDev dev sshDev sshArgs sshCmd directCmd sshOpt
  # input from caller func
  dev=$1
  cmd=$2
  args=$3
  sshOpt=$4

  if [[ $dev = *"tcp/"* ]]; then # wrap in ssh
    pciDev="dev/wbm0"                                                   # constant for pci device string
    sshDev=`echo $dev | cut -d\/ -f2`                                   # remove tcp/ udp/ dev/ from device string for ssh hostname
    sshArgs="${args/$dev/$pciDev}"                                      # replace device string with pci device string for ssh access
    sshCmd="ssh $sshOpt root@$sshDev \"cd /tmp; $cmd $sshArgs\""        # the full ssh command
    eval $sshCmd
  else
    directCmd="command $cmd $args" # the full direct command, bypass alias to avoid loop
    eval $directCmd
  fi
}

_get_eb_device() {
  local dev
  dev=`echo $@ | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]*"`
  echo "$dev"
}

_get_file_name_no_path() {
  local rawCmd ending fileNoPath
  ending="$1"
  shift 1
  rawCmd=$@
  fileNoPath=`echo $rawCmd | grep -o -E "[a-zA-Z0-9\_\-]*\.("$ending")"`  # isolate file string from input
  echo "$fileNoPath"
}

_get_file_name_with_path() {
  local rawCmd ending fileWithPath
  ending="$1"
  shift 1
  rawCmd=$@
  fileWithPath=`echo $rawCmd | grep -o -E "[a-zA-Z0-9\_\-\/\.]*\.("$ending")"`  # isolate file string from input
  echo "$fileWithPath"
}

_eb_ssh_eb_cpy_file_to_fe() {
  local dev fileWithPath fileNoPath sshDev scpCmd
  dev=$1
  fileWithPath=$2
  fileNoPath=$3

  # secure copy the file from caller pc to frontend
  if [[ $dev = *"tcp/"* ]] && [[ -n $fileWithPath ]]; then
    sshDev=`echo $dev | cut -d\/ -f2`
    scpCmd="scp $fileWithPath root@$sshDev:/tmp/"
    eval $scpCmd
  fi

}

_eb_ssh_eb_cpy_file_from_fe() {
  local dev fileWithPath fileNoPath sshDev scpCmd
  dev=$1
  fileWithPath=$2
  fileNoPath=$3

  # secure copy the file from frontend to caller pc
  if [[ $dev = *"tcp/"* ]] && [[ -n $fileWithPath ]]; then
    sshDev=`echo $dev | cut -d\/ -f2`
    scpCmd="scp root@$sshDev:/tmp/$fileNoPath $fileWithPath"
    eval $scpCmd
  fi
}

_eb_ssh_eb_cpy_download_dot_from_fe() {
  local dev sshDev scpCmd
  dev=$1

  if [[ $dev = *"tcp/"* ]]; then
    sshDev=`echo $dev | cut -d\/ -f2`                                             # remove tcp/ udp/ dev/ from device string for ssh hostname
    scpCmd="scp root@$sshDev:/tmp/download.dot ."                                 # secure copy the file from frontend to caller pc
    eval $scpCmd
  fi
}

_my_arg_test() {
  local dev rawCmd fileWithPath fileNoPath
  dev=$1
  fileWithPath=$2
  fileNoPath=$3
  rawCmd=$4

  echo "args 1! $dev 2! $fileWithPath 3! $fileNoPath @! $rawCmd"
}


_eb_ssh_eb_ls() {
  local cmd raw dev
  cmd="eb-ls"

  raw=$@
  dev=$(_get_eb_device "$raw")
  _eb_ssh_eb_func "$dev" "$cmd" "$raw"
}

_eb_ssh_eb_info() {
  local cmd raw dev
  cmd="eb-info"

  raw=$@
  dev=$(_get_eb_device "$raw")
  _eb_ssh_eb_func "$dev" "$cmd" "$raw"
}

_eb_ssh_eb_read() {
  local cmd raw dev
  cmd="eb-read"

  raw=$@
  dev=$(_get_eb_device "$raw")
  _eb_ssh_eb_func "$dev" "$cmd" "$raw"
}

_eb_ssh_eb_write() {
  local cmd raw dev
  cmd="eb-write"

  raw=$@
  dev=$(_get_eb_device "$raw")
  _eb_ssh_eb_func "$dev" "$cmd" "$raw"
}

_eb_ssh_eb_get() {
  cmd="eb-get"
  raw=$@
  fnp=$(_get_file_name_no_path "bin" "$raw")
  fwp=$(_get_file_name_with_path "bin" "$raw")
  dev=$(_get_eb_device "$raw")

  if [ -n $fnp ] && [[ $dev = *"tcp/"* ]]; then
    raw="${raw/$fwp/$fnp}"                    # remove path from file names
  fi

  _eb_ssh_eb_func "$dev" "$cmd" "$raw" ""
  _eb_ssh_eb_cpy_file_from_fe "$dev" "$fwp" "$fnp"
}

_eb_ssh_eb_put() {
  cmd="eb-put"
  raw=$@
  fnp=$(_get_file_name_no_path "bin" "$raw")
  fwp=$(_get_file_name_with_path "bin" "$raw")
  dev=$(_get_eb_device "$raw")

  if [ -n $fnp ] && [[ $dev = *"tcp/"* ]]; then
    raw="${raw/$fwp/$fnp}"                    # remove path from file names
  fi

  _eb_ssh_eb_cpy_file_to_fe "$dev" "$fwp" "$fnp"
  _eb_ssh_eb_func "$dev" "$cmd" "$raw" ""
}

_eb_ssh_eb_flash() {
  cmd="eb-flash"
  raw=$@
  fnp=$(_get_file_name_no_path "rpd" "$raw")
  fwp=$(_get_file_name_with_path "rpd" "$raw")
  dev=$(_get_eb_device "$raw")

  if [ -n $fnp ] && [[ $dev = *"tcp/"* ]]; then
    raw="${raw/$fwp/$fnp}"                    # remove path from file names
  fi

  _eb_ssh_eb_cpy_file_to_fe "$dev" "$fwp" "$fnp"
  _eb_ssh_eb_func "$dev" "$cmd" "$raw" ""
}


_eb_ssh_eb_mon() {
  local cmd raw dev
  cmd="eb-mon"

  raw=$@
  dev=$(_get_eb_device "$raw")
  _eb_ssh_eb_func "$dev" "$cmd" "$raw"
}

_eb_ssh_eb_fwload() {
  local cmd raw fnp fwp dev
  cmd="eb-fwload"
  raw=$@
  fnp=$(_get_file_name_no_path "bin" "$raw")
  fwp=$(_get_file_name_with_path "bin" "$raw")
  dev=$(_get_eb_device "$raw")

  if [ -n $fnp ] && [[ $dev = *"tcp/"* ]]; then
    raw="${raw/$fwp/$fnp}"                    # remove path from file names
  fi

  _eb_ssh_eb_cpy_file_to_fe "$dev" "$fwp" "$fnp"
  _eb_ssh_eb_func "$dev" "$cmd" "$raw" ""
}

_eb_ssh_eb_find() {
  local cmd raw dev
  cmd="eb-find"

  raw=$@
  dev=$(_get_eb_device "$raw")
  _eb_ssh_eb_func "$dev" "$cmd" "$raw"
}

_eb_ssh_eb_reset() {
  local cmd raw dev
  cmd="eb-reset"

  raw=$@
  dev=$(_get_eb_device "$raw")
  _eb_ssh_eb_func "$dev" "$cmd" "$raw"
}

_eb_ssh_eb_console() {
  local cmd raw dev opt
  cmd="eb-console"
  opt="-t"

  raw=$@
  dev=$(_get_eb_device "$raw")
  _eb_ssh_eb_func "$dev" "$cmd" "$raw" "$opt"


}


# DM tools
_eb_ssh_dm_sched() {
  local cmd raw fnp fwp dev
  cmd="dm-sched"
  raw=$@
  fnp=$(_get_file_name_no_path "dot" "$raw")
  fwp=$(_get_file_name_with_path "dot" "$raw")
  dev=$(_get_eb_device "$raw")

  if [ -n $fnp ] && [[ $dev = *"tcp/"* ]]; then
    raw="${raw/$fwp/$fnp}"                    # remove path from file names
  fi

  _eb_ssh_eb_cpy_file_to_fe "$dev" "$fwp" "$fnp"
  _eb_ssh_eb_func "$dev" "$cmd" "$raw" ""
  _eb_ssh_eb_cpy_download_dot_from_fe "$dev" "$fwp" "$fnp"
}

_eb_ssh_dm_cmd() {
  local cmd raw fnp fwp dev
  cmd="dm-cmd"
  raw=$@
  fnp=$(_get_file_name_no_path "dot" "$raw")
  fwp=$(_get_file_name_with_path "dot" "$raw")
  dev=$(_get_eb_device "$raw")

  if [ -n $fnp ] && [[ $dev = *"tcp/"* ]]; then
    raw="${raw/$fwp/$fnp}"                    # remove path from file names
  fi

  _eb_ssh_eb_cpy_file_to_fe "$dev" "$fwp" "$fnp"
  _eb_ssh_eb_func "$dev" "$cmd" "$raw" ""

}

_eb_ssh_test_me() {
  local fnp fwp dev raw
  raw=$@
  fnp=$(_get_file_name_no_path "bin" "$raw")
  fwp=$(_get_file_name_with_path "bin" "$raw")
  dev=$(_get_eb_device "$raw")

  echo "mydev $dev"

  if [ -n $fnp ]; then
    raw="${raw/$fwp/$fnp}"                    # remove path from file names
  fi
  _my_arg_test "$dev" "$fwp" "$fnp" "$raw"
}